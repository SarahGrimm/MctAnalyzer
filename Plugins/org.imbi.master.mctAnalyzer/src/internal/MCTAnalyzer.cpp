/*===================================================================

Calculate pore distribution and scaffold quality key figures for given mCT scaffold stacks

@author Sarah Richter

===================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <regex>
#include "mitkSurfaceToImageFilter.h"
#include "vtkStringArray.h"
#include <berryIWorkbenchWindow.h>
#include <mitkExtractSliceFilter.h>
#include <vtkMassProperties.h>
#include <vtkCleanPolyData.h>
#include "mitkPointSet.h"
#include <numeric>
#include <QFileDialog>
#include <vtkTIFFReader.h>
#include "itkLabelStatisticsImageFilter.h"
#include <vtkCenterOfMass.h>
#include <mitkITKImageImport.h>
#include <QmitkBoneSegmentationWidget.h>
#include "ConnectedSegmentationFilter.h"
#include "itkSimpleContourExtractorImageFilter.h"
#include "itkTileImageFilter.h"
#include "itkAddImageFilter.h"
// Qmitk
#include "MCTAnalyzer.h"
#include "mctAnalyzerCropFilter.h"
#include "NodeHelper.h"
#include <vtkLinearExtrusionFilter.h>
#include "itkJoinImageFilter.h"
//#include "mctAnalyzerBinarySegmentationFilter.h"
#include "mctAnalyzerSegmentationFilter.h"
#include "mctAnalyzerDistanceMapFilter.h"
#include "mctAnalyzerRegionalMaximaFilter.h"
#include "mitkImageCast.h"
#include <mitkImageToSurfaceFilter.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkGeometryFilter.h>
#include <vtkDelaunay3D.h>
#include <vtkDelaunay2D.h>
#include <vtkHull.h>
#include <vtkCellIterator.h>
#include <vector>
#include <algorithm>
#include "mitkSurface.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include <vtkSphereSource.h>
#include <vtkSmoothPolyDataFilter.h>
#include <mitkImagePixelReadAccessor.h>
#include <vtkPolyDataToImageStencil.h>
#include "itkBinaryContourImageFilter.h"

#include "HoughTransformation2DFilter.h"
#include "HoughTransformation3DFilter.h"


//itk


// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>

const std::string MCTAnalyzer::VIEW_ID = "org.mitk.views.mctanalyzer";

void MCTAnalyzer::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

/*Create QT UI binding*/
void MCTAnalyzer::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file

	verticalLayout = new QVBoxLayout;
	path = "/";
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
 
  m_Controls.SegmentationWidget->hide();
  m_Controls.widget_11->hide();

  m_Controls.showPoreVisualisation->hide();

  connect(m_Controls.pushButton, SIGNAL(clicked()), this, SLOT(AddSegment()));
  connect(m_Controls.outputDirectoryButton, SIGNAL(clicked()), this, SLOT(SetOutputDirectory()));
  connect(m_Controls.all, SIGNAL(clicked(bool)), this, SLOT(toggled(bool)));
  connect(m_Controls.c_showImages, SIGNAL(clicked(bool)), this, SLOT(ShowImages(bool)));
  connect(m_Controls.addExaminationButton, SIGNAL(clicked()), this, SLOT(AddExamination()));
  connect(m_Controls.SegmentationWidget, SIGNAL(SubmitThresholds(int, int, QString)), this, SLOT(SaveThresholds(int, int, QString)));
  connect(m_Controls.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
  connect(m_Controls.showPoreVisualisation, SIGNAL(clicked(bool)), this, SLOT(ShowPoreVis(bool)));
}




/* Find Pores via euclidean distance map and regional maxima
*/
std::vector<double> MCTAnalyzer::FindPores(const mitk::Image::Pointer segmentedBinaryImage, double pixelSize, int& lowerPercentile, int& upperPercentile, const mitk::Image::Pointer  maski, const mitk::Surface::Pointer convexHull){


	mitk::BaseGeometry::Pointer boGeometry = convexHull->GetGeometry();
	mitk::BaseGeometry::Pointer inputImageGeometry = segmentedBinaryImage->GetSlicedGeometry();

	mitk::BoundingBox::Pointer boBoxRelativeToImage =
		boGeometry->CalculateBoundingBoxRelativeToTransform(inputImageGeometry->GetIndexToWorldTransform());



	mitk::Image::Pointer contour = CreateContourFromMask(maski, segmentedBinaryImage);


	//DoHoughTransform(test);

	MctAnalyzerDistanceMapFilter::Pointer dF = MctAnalyzerDistanceMapFilter::New();
	dF->SetInput(contour); // don't forget this
	dF->SetBoundingObject(boBoxRelativeToImage);
	dF->Update();
	mitk::Image::Pointer distanceMap = dF->GetOutput();

	if (showImages){
		mitk::DataNode::Pointer dMNode2 = mitk::DataNode::New();
		dMNode2->SetData(distanceMap);
		dMNode2->SetProperty("name", mitk::StringProperty::New("distance_Map_1"));
		this->GetDataStorage()->Add(dMNode2);
	}

	MctAnalyzerCropFilter::Pointer cF = MctAnalyzerCropFilter::New();
	cF->SetInput(distanceMap);
	cF->SetMask(maski);
	cF->SetBoundingObject(boBoxRelativeToImage);
	cF->Update();

	mitk::Image::Pointer test = cF->GetOutput();


	if (showImages){
		mitk::DataNode::Pointer dMNode = mitk::DataNode::New();
		dMNode->SetData(test);
		dMNode->SetProperty("name", mitk::StringProperty::New("distance_Map"));
		this->GetDataStorage()->Add(dMNode);
	}

	MctAnalyzerRegionalMaximaFilter::Pointer mF = MctAnalyzerRegionalMaximaFilter::New();
	mF->SetInput(test); // don't forget this
	mF->Update();

	mitk::Image::Pointer regionalMaxima = mF->GetOutput();


	if (showImages){
		mitk::DataNode::Pointer rNode = mitk::DataNode::New();
		rNode->SetData(regionalMaxima);
		rNode->SetProperty("name", mitk::StringProperty::New("pore_origin_Map"));
		this->GetDataStorage()->Add(rNode);
	}

	std::vector<double> radii;

	mitk::PointSet::Pointer pointSet = mF->GetPointSet();

	int numberOfPoints = pointSet->GetSize();
	double radius;
	int durchmesser;
	int  range;
	int remainder;
	for (int i = 0; i < numberOfPoints; i++)
	{
		mitk::Point3D point = pointSet->GetPoint(i);
		itk::Index<3> currentIndex;
		currentIndex[0] = point[0];
		currentIndex[1] = point[1];
		currentIndex[2] = point[2];

		radius = distanceMap->GetPixelValueByIndex(currentIndex);


		durchmesser = radius * 2 * pixelSize;

		remainder = durchmesser % 10;

		range = durchmesser + 10 - remainder;

		mitk::Point3D pOrigin;
		mitk::BaseGeometry::Pointer geometry = test->GetGeometry();
		geometry->IndexToWorld(point, pOrigin);

		if (poreSizeRange.find(range) != poreSizeRange.end()) {

			Pore pore;
			pore.origin = pOrigin;
			pore.radius = radius;
			poreSizeRange[range].push_back(pore);
		}
		else {

			std::vector<Pore> pores;
			Pore pore;
			pore.origin = pOrigin;
			pore.radius = radius;
			pores.push_back(pore);
			poreSizeRange[range] = pores;
		}

		radii.push_back(durchmesser);
		
	}


	std::sort(radii.begin(), radii.end());
	int vectorSize = radii.size();


	int cLower, cUpper;

	cLower = *std::min_element(radii.begin(), radii.end());
	cUpper = *std::max_element(radii.begin(), radii.end());

	MITK_WARN << "Biggest" << cUpper << " Lowest" << cLower;


	lowerPercentile = ceil(vectorSize*0.05);

	upperPercentile = ceil(vectorSize*0.95);

	MITK_WARN << "UpperPercentile" << upperPercentile << " lowerPercentile" << lowerPercentile;

	//std::vector<double> filteredRadii(radii.begin() + lowerPercentile, radii.begin() + upperPercentile);
	std::vector<double> filteredRadii(radii.begin(), radii.end());
	MITK_WARN << "Count" << filteredRadii.size();


	remainder = cLower % 10;
	range = cLower + 10 - remainder;

	m_Controls.horizontalSlider->setTickPosition(QSlider::TicksBothSides);
	m_Controls.horizontalSlider->setTickInterval(10);
	m_Controls.horizontalSlider->setSingleStep(10);
	m_Controls.horizontalSlider->setMinimum(range / 10);

	remainder = cUpper % 10;
	range = cUpper + 10 - remainder;

	m_Controls.horizontalSlider->setMaximum(range / 10);

	MITK_WARN << "PoreSizeRange" << poreSizeRange.size();

	return filteredRadii;


}

/*Calculate results*/
void MCTAnalyzer::DoImageProcessing()
{
	//PreprocessingWriter();

	m_StartTime = std::chrono::system_clock::now();
	QList<QHBoxLayout *> allPButtons = m_Controls.widget_7->findChildren<QHBoxLayout *>();
	MITK_WARN << "LineEdits" << allPButtons.count();
	typedef int PixelType;
	typedef itk::Image< PixelType, 3 > RealImageType;

	typedef itk::LabelStatisticsImageFilter< RealImageType, RealImageType > LabelStatisticsImageFilterType;
	typedef LabelStatisticsImageFilterType::LabelPixelType                LabelPixelType;

	mitk::Image::Pointer mImage;
	RealImageType::Pointer originalImageITK;
	mitk::Image::Pointer segmentedBinaryImage;
	RealImageType::Pointer segmentedBinaryImageITK;
	mitk::Image::Pointer maskConvexHullImage;

	LabelStatisticsImageFilterType::Pointer labelStatisticsImageFilter;
	LabelStatisticsImageFilterType::Pointer labelStatisticsTVImageFilter;
	mitk::Surface::Pointer segmentedSurface;
	mitk::Surface::Pointer convexHullSurface;
	RealImageType::Pointer maskConvexHullImageITK;
	mitk::Image::Pointer maskCon;

	double SV = 0;
	double SS = 0;
	double TV = 0;
	double P = 0;
	double SR = 0;

	int lowerPercentile = 0;
	int upperPercentile = 0;
	double TMD_TV_Mean = 0;
	double TMD_TV_Sigma = 0;

	double TMD_SV_Mean = 0;
	double TMD_SV_Sigma = 0;
	int PN = 0;
	double PV = 0;
	double PS_Mean = 0;
	double PS_Sigma = 0;
	double pixelSize = 11.78;
	QString sampleText;

	for (int i = 0; i < counter; ++i) {
		QString pixelQstr;
		pixelQstr.append("pixel");
		pixelQstr.append(QString::number(i));
		QLineEdit* lineEdit = m_Controls.widget_7->findChild<QLineEdit*>(pixelQstr);

		pixelSize = lineEdit->text().toDouble();

		QString sampleQstr;
		sampleQstr.append("Sample");
		sampleQstr.append(QString::number(i));

		sampleText = m_Controls.widget_7->findChild<QLineEdit*>(sampleQstr)->text();

		for (int m = 0; m <= examinationCounter; ++m)
		{
			MITK_WARN << i << "_" << m;
			QString examStr;
			examStr.append("T");
			examStr.append(QString::number(m));

			QString lineEditQstr;
			lineEditQstr.append("T_");
			lineEditQstr.append(QString::number(i));
			lineEditQstr.append("_");
			lineEditQstr.append(QString::number(m));

			QString objectName;
			objectName.append(QString::number(i));
			objectName.append("_");
			objectName.append(QString::number(m));

			QString path = m_Controls.widget_7->findChild<QLineEdit*>(lineEditQstr)->text();

			SV = 0;
			SS = 0;
			TV = 0;
			P = 0;
			SR = 0;

			lowerPercentile = 0;
			upperPercentile = 0;
			TMD_TV_Mean = 0;
			TMD_TV_Sigma = 0;

			TMD_SV_Mean = 0;
			TMD_SV_Sigma = 0;
			PN = 0;
			PV = 0;
			PS_Mean = 0;
			PS_Sigma = 0;

			std::vector<double> filteredRadii;

			//MITK_WARN << "Path" << path;
			if (!path.isEmpty()){

				QDir directory(path);
				QStringList images = directory.entryList(QStringList() << "*.tif" << "*.TIF", QDir::Files);

				vtkSmartPointer<vtkStringArray> sA = vtkSmartPointer<vtkStringArray>::New();

				foreach(QString filename, images) {
					sA->InsertNextValue(path.toStdString() + "/" + filename.toStdString());
					//MITK_WARN << filename.toStdString();

				}

				//std::string filepath = paths.at(i).toStdString() + "/" + sA->GetValue(0);

				//MITK_WARN << sA->GetSize() << " " << sA->GetValue(0);

				vtkSmartPointer<vtkTIFFReader> reader =
					vtkSmartPointer<vtkTIFFReader>::New();
				reader->SetFileNames(sA);
				reader->Update();
				vtkSmartPointer<vtkImageData> imageData = reader->GetOutput();
				imageData->SetSpacing(1.0f, 1.0f, 1.0f);
				//MITK_WARN << imageData->GetExtent()[0] << " " << imageData->GetExtent()[1] << " " << imageData->GetExtent()[2] << " " << imageData->GetExtent()[3] << " " << imageData->GetExtent()[4] << " " << imageData->GetExtent()[5];

				//MITK_WARN << "Range" << imageData->GetScalarTypeMin();
				//MITK_WARN << "Range" << imageData->GetScalarTypeMax();


				mImage = mitk::Image::New();
				mImage->Initialize(imageData);
				mImage->SetVolume(imageData->GetScalarPointer());


				mitk::ScalarType spacing[3];
				spacing[0] = 1.0f;
				spacing[1] = 1.0f;
				spacing[2] = 1.0f;
				mitk::Point3D origin;
				origin.Fill(0);
				mImage->SetSpacing(spacing);
				mImage->SetOrigin(origin);

				if (showImages){
					mitk::DataNode::Pointer newNode = mitk::DataNode::New();
					newNode->SetData(mImage);
					// set some properties
					//newNode->SetProperty("binary", mitk::BoolProperty::New(true));
					newNode->SetProperty("name", mitk::StringProperty::New("dumb segmentation34"));
					this->GetDataStorage()->Add(newNode);
				}

				if (mImage)
				{

					mitk::CastToItkImage(mImage, originalImageITK);

					segmentedBinaryImage = ScaffoldSegmentation(mImage, objectName);


					mitk::CastToItkImage(segmentedBinaryImage, segmentedBinaryImageITK);

					if (segmentedBinaryImageITK != nullptr && (m_Controls.cB_SV->isChecked() || m_Controls.cB_SR->isChecked() || m_Controls.cB_P->isChecked() || m_Controls.cB_TMD_SV->isChecked())){

						//Stats for segmented binary image
						labelStatisticsImageFilter = LabelStatisticsImageFilterType::New();
						labelStatisticsImageFilter->SetLabelInput(segmentedBinaryImageITK);
						labelStatisticsImageFilter->SetInput(originalImageITK);
						labelStatisticsImageFilter->ReleaseDataFlagOn();
						labelStatisticsImageFilter->Update();

						if (labelStatisticsImageFilter->HasLabel(1))
						{
							LabelPixelType labelValue = 1;
							std::cout << "min: " << labelStatisticsImageFilter->GetMinimum(labelValue) << std::endl;
							/*std::cout << labelValue << std::endl;

							std::cout << "max: " << labelStatisticsImageFilter->GetMaximum(labelValue) << std::endl;
							std::cout << "median: " << labelStatisticsImageFilter->GetMedian(labelValue) << std::endl;
							std::cout << "variance: " << labelStatisticsImageFilter->GetVariance(labelValue) << std::endl;
							std::cout << "sum: " << labelStatisticsImageFilter->GetSum(labelValue) << std::endl;
							std::cout << "region: " << labelStatisticsImageFilter->GetRegion(labelValue) << std::endl;
							std::cout << std::endl << std::endl;*/
							//std::cout << "box: " << labelStatisticsImageFilter->GetBoundingBox( labelValue ) << std::endl; // can't output a box

							if (m_Controls.cB_SV->isChecked() || m_Controls.cB_SR->isChecked() || m_Controls.cB_P->isChecked()){

								SV = labelStatisticsImageFilter->GetCount(labelValue) * std::pow(pixelSize, 3.0);
								MITK_WARN << "Scaffold Volume: " << SV;
								resultMap["SV"] = SV;
							}

							if (m_Controls.cB_TMD_SV->isChecked()) {

								TMD_SV_Mean = labelStatisticsImageFilter->GetMean(labelValue);
								TMD_SV_Sigma = labelStatisticsImageFilter->GetSigma(labelValue);
								resultMap["mean(TMD_SV)"] = TMD_SV_Mean;
								resultMap["sd(TMD_SV)"] = TMD_SV_Sigma;
								std::cout << "TMD_SV_Mean: " << TMD_SV_Mean << std::endl;
								std::cout << "TMD_SV_Sigma: " << TMD_SV_Sigma << std::endl;
							}

						}

					}


					if (m_Controls.cB_TV->isChecked() || m_Controls.cB_SS->isChecked() || m_Controls.cB_SR->isChecked() || m_Controls.cB_P->isChecked()
						|| m_Controls.cB_PN->isChecked() || m_Controls.cB_PSR->isChecked() || m_Controls.cB_PV->isChecked() || m_Controls.cB_PS->isChecked()
						|| m_Controls.cB_TMD_TV->isChecked()) {

						segmentedSurface = CalculateScaffoldSurface(segmentedBinaryImage, SS, pixelSize, mImage);
						std::cout << "Scaffold Surface: " << SS << std::endl;
						resultMap["SS"] = SS;

					}

					if (m_Controls.cB_SR->isChecked()) {
						SR = SS / SV;
						std::cout << "Scaffold Ratio: " << SR << std::endl;
						resultMap["SR"] = SR;
					}


					if (m_Controls.cB_TV->isChecked() || m_Controls.cB_P->isChecked()
						|| m_Controls.cB_PN->isChecked() || m_Controls.cB_PSR->isChecked() || m_Controls.cB_PV->isChecked() || m_Controls.cB_PS->isChecked()
						|| m_Controls.cB_TMD_TV->isChecked()) {

						convexHullSurface = CalculateConvexSurface(segmentedSurface, TV, pixelSize);
						std::cout << "Total Volume: " << TV << std::endl;
						resultMap["TV"] = TV;
					}

					if (m_Controls.cB_P->isChecked()) {
						P = (TV - SV) / TV;
						std::cout << "Porosity: " << P << std::endl;
						resultMap["P"] = P;
					}

					//End Stats for segmented binary image


					//Calculate Total Volume

					if (m_Controls.cB_PN->isChecked() || m_Controls.cB_PSR->isChecked() || m_Controls.cB_PV->isChecked() || m_Controls.cB_PS->isChecked()
						|| m_Controls.cB_TMD_TV->isChecked()) {

						//RealImageType::Pointer image2;
						maskCon = CreateMaskOfConvexSurface(convexHullSurface, mImage);
						mitk::CastToItkImage(maskCon, maskConvexHullImageITK);
					}


					if (m_Controls.cB_TMD_TV->isChecked()) {

						labelStatisticsTVImageFilter = LabelStatisticsImageFilterType::New();
						labelStatisticsTVImageFilter->SetLabelInput(maskConvexHullImageITK);
						labelStatisticsTVImageFilter->ReleaseDataFlagOn();
						labelStatisticsTVImageFilter->SetInput(originalImageITK);
						labelStatisticsTVImageFilter->Update();


						if (labelStatisticsTVImageFilter->HasLabel(1))
						{
							LabelPixelType labelValue = 1;
							/*std::cout << "min: " << labelStatisticsImageFilter->GetMinimum(labelValue) << std::endl;
							std::cout << "max: " << labelStatisticsImageFilter->GetMaximum(labelValue) << std::endl;
							std::cout << "median: " << labelStatisticsImageFilter->GetMedian(labelValue) << std::endl;
							std::cout << "variance: " << labelStatisticsImageFilter->GetVariance(labelValue) << std::endl;
							std::cout << "sum: " << labelStatisticsImageFilter->GetSum(labelValue) << std::endl;
							std::cout << "count: " << labelStatisticsImageFilter->GetCount(labelValue) << std::endl;
							//std::cout << "box: " << labelStatisticsImageFilter->GetBoundingBox( labelValue ) << std::endl; // can't output a box
							std::cout << "region: " << labelStatisticsImageFilter->GetRegion(labelValue) << std::endl;
							std::cout << std::endl << std::endl;*/

							TMD_TV_Mean = labelStatisticsTVImageFilter->GetMean(labelValue);
							TMD_TV_Sigma = labelStatisticsTVImageFilter->GetSigma(labelValue);
							std::cout << "TMD_TV_Mean: " << TMD_TV_Mean << std::endl;
							std::cout << "TMD_TV_Sigma: " << TMD_TV_Sigma << std::endl;
							resultMap["mean(TMD_TV)"] = TMD_TV_Mean;
							resultMap["sd(TMD_TV)"] = TMD_TV_Sigma;

						}

					}
					//End Calculate Total Volume


					//Calculate Pore Stats

					if (m_Controls.cB_PN->isChecked() || m_Controls.cB_PSR->isChecked() || m_Controls.cB_PV->isChecked() || m_Controls.cB_PS->isChecked()) {

						filteredRadii = FindPores(segmentedBinaryImage, pixelSize, lowerPercentile, upperPercentile, maskCon, convexHullSurface);

						PN = filteredRadii.size();
						resultMap["PN"] = PN;
						MITK_WARN << "Circle Count" << PN;

					}

					if (m_Controls.cB_PS->isChecked()) {

						double sum = std::accumulate(filteredRadii.begin(), filteredRadii.end(), 0.0);
						PS_Mean = sum / filteredRadii.size();

						double sq_sum = std::inner_product(filteredRadii.begin(), filteredRadii.end(), filteredRadii.begin(), 0.0);
						PS_Sigma = std::sqrt(sq_sum / filteredRadii.size() - PS_Mean * PS_Mean);
						MITK_WARN << "Mean" << PS_Mean << " STD " << PS_Sigma;
						resultMap["mean(PS)"] = PS_Mean;
						resultMap["sd(PS)"] = PS_Sigma;

					}

					if (m_Controls.cB_PV->isChecked()) {

						std::vector<double> volume;
						volume.resize(filteredRadii.size());

						std::transform(filteredRadii.begin(), filteredRadii.end(), volume.begin(), [=](double i) {  return (0.166666666667 * 3.14159265359 * std::pow(i, 3.0)); });



						PV = std::accumulate(volume.begin(), volume.end(), 0.0);


						MITK_WARN << volume.size() << "," << filteredRadii[500] << "_" << volume[500];
						/*for (int i = 0; i <= filteredRadii.size(); i++) {
						PV += (4 / 3) * 3.14159265359 * std::pow(filteredRadii[i] / 2, 3.0);
						}*/

						MITK_WARN << "Total Pore Volume " << PV;
						resultMap["PV"] = PV;
					}

					mitk::RenderingManager::GetInstance()->RequestUpdateAll();

				}

				WriteResults(examStr, sampleText);
				WritePoreSizeRange(examStr, sampleText);

			}
		}
	}

	if (counter == 1 && examinationCounter == 0){
		m_Controls.showPoreVisualisation->show();

	}

}

/*Visualize pores*/
void MCTAnalyzer::ShowPoreVis(bool checked){
	if (!checked) {
		m_Controls.widget_11->hide();
		mitk::NodePredicateProperty::Pointer HelperObject = mitk::NodePredicateProperty::New("helper object");
		mitk::DataStorage::SetOfObjects::ConstPointer allNodes = this->GetDataStorage()->GetSubset(HelperObject);
		this->GetDataStorage()->Remove(allNodes);
	}
	else{
		m_Controls.widget_11->show();
	}

}

/*Set value viaSlider*/
void MCTAnalyzer::setValue(int val) {

	if (m_Controls.showPoreVisualisation->isChecked()) {

		int range = val * 10;

		m_Controls.poreSize->setText(QString::number(range));

		if (!this->GetDataStorage()->Exists(this->GetDataStorage()->GetNamedNode("Sphere" + std::to_string(range)))){
			if (poreSizeRange.find(range) != poreSizeRange.end()) {


				if (!m_Controls.showAllPores->isChecked()) {
					mitk::NodePredicateProperty::Pointer HelperObject = mitk::NodePredicateProperty::New("helper object");
					mitk::DataStorage::SetOfObjects::ConstPointer allNodes = this->GetDataStorage()->GetSubset(HelperObject);
					this->GetDataStorage()->Remove(allNodes);
				}

				for (auto const& x : poreSizeRange[range]){
					mitk::Point3D pOrigin = x.origin;
					double radius = x.radius;

					// Init Sphere
					vtkSmartPointer<vtkSphereSource> sphereSource =
						vtkSmartPointer<vtkSphereSource>::New();
					sphereSource->SetCenter(pOrigin[0], pOrigin[1], pOrigin[2]);
					sphereSource->SetRadius(radius);
					sphereSource->Update();

					// Add Sphere to datastorage
					mitk::Surface::Pointer sphereSurface = mitk::Surface::New();
					sphereSurface->SetVtkPolyData(sphereSource->GetOutput());
					mitk::DataNode::Pointer sphereNode = mitk::DataNode::New();
					sphereNode->SetData(sphereSurface);
					sphereNode->SetName("Sphere" + std::to_string(range));
					sphereNode->SetProperty("helper object", mitk::BoolProperty::New(true));
					this->GetDataStorage()->Add(sphereNode);
				}
			}
		}
	}
}

/*Segmentation Thresholds*/
void MCTAnalyzer::SaveThresholds(int min, int max, QString name) {

	Threshold newThreshold;
	newThreshold.min = min;
	newThreshold.max = max;

	thresholds[name] = newThreshold;

	

}

void MCTAnalyzer::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  /*foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls.labelWarning->setVisible( false );
      m_Controls.buttonPerformImageProcessing->setEnabled( true );
      return;
    }
  }

  m_Controls.labelWarning->setVisible( true );
  m_Controls.buttonPerformImageProcessing->setEnabled( false );*/
}


void MCTAnalyzer::toggled(bool checked){
	QList<QCheckBox *> allPButtons = m_Controls.widget_5->findChildren<QCheckBox *>();
	for (int i = 0; i < allPButtons.count(); i++){
		allPButtons.at(i)->setChecked(checked);
	}
}

void MCTAnalyzer::ShowImages(bool checked){
	showImages = checked;

}


/*add examination to gui*/
void MCTAnalyzer::AddExamination()
{ 
	examinationCounter++;
	
	QHBoxLayout* examinationLabels = m_Controls.widget_9->findChild<QHBoxLayout*>();
	//QHBoxLayout* examinationLabels = m_Controls.widget_9->layout();
	

	QLabel *label = new QLabel();
	QString labelString;
	labelString.append("T");
	labelString.append(QString::number(examinationCounter));
	label->setText(labelString);
	label->setFixedWidth(80);

	examinationLabels->insertWidget(examinationCounter, label);

	examinationLabels->addStretch();

	for (int i = 0; i < counter; ++i)
	{
		MITK_WARN << i << "_" << examinationCounter;
		QString sampleString;
		sampleString.append("examinationLayout");
		sampleString.append(QString::number(i));
	
		QHBoxLayout* lineEdit = m_Controls.widget_7->findChild<QHBoxLayout*>(sampleString);

		QLineEdit *le = new QLineEdit();
		QString lineEditQstr;
		lineEditQstr.append("T_");
		lineEditQstr.append(QString::number(i));
		lineEditQstr.append("_");
		lineEditQstr.append(QString::number(examinationCounter));
		le->setObjectName(lineEditQstr);
		le->setFixedWidth(50);
		le->setStyleSheet("margin-right: 0px;margin-left: 3px;");

		lineEdit->addWidget(le);

		QPushButton *choose_button = new QPushButton();
		choose_button->setText(tr("..."));
		choose_button->setFixedWidth(10);
		QString chooseQstr;
		//chooseQstr.append("DirButton_");
		chooseQstr.append(QString::number(i));
		chooseQstr.append("_");
		chooseQstr.append(QString::number(examinationCounter));
		choose_button->setObjectName(chooseQstr);
		choose_button->setStyleSheet("margin-right: 0px; margin-left: 0px;");
		QObject::connect(choose_button, SIGNAL(clicked()), this, SLOT(ChooseDirectories()));


		lineEdit->addWidget(choose_button);

		QPushButton *preview_button = new QPushButton();
		preview_button->setText(tr("P"));
		preview_button->setFixedWidth(10);
		QString previewQstr;
		//previewQstr.append("PreviewButton_");
		previewQstr.append(QString::number(i));
		previewQstr.append("_");
		previewQstr.append(QString::number(examinationCounter));
		preview_button->setObjectName(previewQstr);
		preview_button->setStyleSheet("margin-right: 0px; margin-left: 0px;");
		QObject::connect(preview_button, SIGNAL(clicked()), this, SLOT(ShowPreview()));

		lineEdit->addWidget(preview_button);

		QPushButton *manuel_button = new QPushButton();
		manuel_button->setText(tr("M"));
		manuel_button->setFixedWidth(10);
		QString manuelQstr;
		//manuelQstr.append("ManuelButton_");
		manuelQstr.append(QString::number(i));
		manuelQstr.append("_");
		manuelQstr.append(QString::number(examinationCounter));
		manuel_button->setObjectName(manuelQstr);
		manuel_button->setStyleSheet("margin-right: 0px; margin-left: 0px;");
		QObject::connect(manuel_button, SIGNAL(clicked()), this, SLOT(ManualSegmentation()));

		lineEdit->addWidget(manuel_button);

		for (int i = 0; i < lineEdit->count(); ++i) {
			QLayoutItem *layoutItem = lineEdit->itemAt(i);
			if (layoutItem->spacerItem()) {
				lineEdit->removeItem(layoutItem);
				// You could also use: layout->takeAt(i);
				delete layoutItem;
				--i;
			}
		}

		

		lineEdit->addStretch();
	}
	
}

/*remove node by name*/
void MCTAnalyzer::RemoveNode(const std::string& name)
{
	mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode(name);
	if (node.IsNotNull())
	{
		GetDataStorage()->Remove(node);
		
	}
}

/*return node by name*/
bool MCTAnalyzer::ExistsNodeWithName(const std::string& name)
{
	mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode(name);
	if (node.IsNotNull())
	{
		return true;

	}
	return false;
}


/*add segment in gui*/
void MCTAnalyzer::AddSegment() {

	QHBoxLayout *horizontalLayout = new QHBoxLayout;


	QString qstr;
	qstr.append("Sample");
	qstr.append(QString::number(counter));
	

	QLineEdit *l = new QLineEdit();
	l->setFixedWidth(50);
	l->setText(qstr);
	l->setObjectName(qstr);


	QHBoxLayout *internHorizontalLayout = new QHBoxLayout;


	QString internHorizontalLayoutName;
	internHorizontalLayoutName.append("examinationLayout");
	internHorizontalLayoutName.append(QString::number(counter));
	internHorizontalLayout->setObjectName(internHorizontalLayoutName);
	internHorizontalLayout->setAlignment(Qt::AlignRight);
	internHorizontalLayout->setSpacing(0);

	for (int i = 0; i < (examinationCounter + 1); ++i)
	{
		QLineEdit *le = new QLineEdit();
		QString lineEditQstr;
		lineEditQstr.append("T_");
		lineEditQstr.append(QString::number(counter));
		lineEditQstr.append("_");
		lineEditQstr.append(QString::number(i));
		le->setObjectName(lineEditQstr);
		le->setFixedWidth(50);
		le->setStyleSheet("margin-right: 0px;");

		internHorizontalLayout->addWidget(le);

		QPushButton *choose_button = new QPushButton();
		choose_button->setText(tr("..."));
		choose_button->setFixedWidth(10);
		QString chooseQstr;
		//chooseQstr.append("DirButton_");
		chooseQstr.append(QString::number(counter));
		chooseQstr.append("_");
		chooseQstr.append(QString::number(i));
		choose_button->setObjectName(chooseQstr);
		choose_button->setStyleSheet("margin-right: 0px; margin-left: 0px;");
		QObject::connect(choose_button, SIGNAL(clicked()), this, SLOT(ChooseDirectories()));


		internHorizontalLayout->addWidget(choose_button);

		QPushButton *preview_button = new QPushButton();
		preview_button->setText(tr("P"));
		preview_button->setFixedWidth(10);
		QString previewQstr;
		//previewQstr.append("PreviewButton_");
		previewQstr.append(QString::number(counter));
		previewQstr.append("_");
		previewQstr.append(QString::number(i));
		preview_button->setObjectName(previewQstr);
		preview_button->setStyleSheet("margin-right: 0px; margin-left: 0px;");
		QObject::connect(preview_button, SIGNAL(clicked()), this, SLOT(ShowPreview()));

		internHorizontalLayout->addWidget(preview_button);

		QPushButton *manuel_button = new QPushButton();
		manuel_button->setText(tr("M"));
		manuel_button->setFixedWidth(10);
		QString manuelQstr;
		//manuelQstr.append("ManuelButton_");
		manuelQstr.append(QString::number(counter));
		manuelQstr.append("_");
		manuelQstr.append(QString::number(i));
		manuel_button->setObjectName(manuelQstr);
		manuel_button->setStyleSheet("margin-right: 0px; margin-left: 0px;");
		QObject::connect(manuel_button, SIGNAL(clicked()), this, SLOT(ManualSegmentation()));

		internHorizontalLayout->addWidget(manuel_button);
	}

	internHorizontalLayout->addStretch();

	QLineEdit *ple = new QLineEdit();
	QString plineEditQstr;
	plineEditQstr.append("pixel");
	plineEditQstr.append(QString::number(counter));
	ple->setFixedWidth(50);
	ple->setText("11.78");
	ple->setObjectName(plineEditQstr);

	horizontalLayout->addWidget(l);
	horizontalLayout->addLayout(internHorizontalLayout);
	horizontalLayout->addWidget(ple);
	verticalLayout->addLayout(horizontalLayout);


	m_Controls.widget_7->setLayout(verticalLayout);
	MITK_WARN << counter << "_" << examinationCounter;
	counter++;


}

/*Show Segmentation Preview*/
void MCTAnalyzer::ShowPreview() {

	//DeleteAllNodes();

	QString objectName = ((QPushButton*)sender())->objectName();
	MITK_WARN << objectName;
	QString lineEditQstr;
	lineEditQstr.append("T_");
	lineEditQstr.append(objectName);

	QLineEdit* lineEdit = m_Controls.widget_7->findChild<QLineEdit*>(lineEditQstr);

	QString path = lineEdit->text();

	mitk::Image::Pointer mImage;

	//if (images.count(lineEditQstr) == 0){
	mImage = Generate3DImageFromImageStack(path);
	/*	images[lineEditQstr] = mImage;
	}
	else {
		mImage = images[lineEditQstr];
	}*/
		
	if (mImage) {

		//std::string imgName = "image_" + lineEditQstr.toStdString();
		//std::string segName = "segmentation_" + lineEditQstr.toStdString();

		std::string imgName = "image";
		std::string segName = "segmentation";

		mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode(imgName);
		if(!node) {
			mitk::DataNode::Pointer newNode = mitk::DataNode::New();
			newNode->SetData(mImage);
			// set some properties
			//newNode->SetProperty("binary", mitk::BoolProperty::New(true));
		
			newNode->SetProperty("name", mitk::StringProperty::New(imgName));
			//RemoveNode(imgName);

			this->GetDataStorage()->Add(newNode);
		}
		else {
			node->SetData(mImage);
		}

		mitk::Image::Pointer segmentedBinaryImage;
		if (thresholds.count(objectName) == 0) {
			MctSegmentationFilter::Pointer segmentedBinaryFilter = MctSegmentationFilter::New();
			segmentedBinaryFilter->SetInput(mImage);
			segmentedBinaryFilter->Update();

			segmentedBinaryImage = segmentedBinaryFilter->GetOutput();

		}
		else {
			Threshold t = thresholds[objectName];
			ConnectedSegmentationFilter::Pointer segmentationFilter = ConnectedSegmentationFilter::New();
			segmentationFilter->SetLowerThreshold(t.min);
			segmentationFilter->SetUpperThreshold(t.max);
			segmentationFilter->SetInput(mImage);
			segmentationFilter->SetUseMedian(false);
			segmentationFilter->Update();

			MITK_WARN << "... done";
			segmentedBinaryImage = segmentationFilter->GetOutput();
		}

		// End Segmentation

		// Show Segmentation

		mitk::DataNode::Pointer segNode = this->GetDataStorage()->GetNamedNode(segName);
		if (!segNode) {

			mitk::DataNode::Pointer segmentedBinaryImageNode = NodeHelper::BuildBaseDataNode(NULL, true, true, segName.c_str(), 0.3, mitk::ColorProperty::New(1.0, 0.74, 0.0));
			segmentedBinaryImageNode->SetData(segmentedBinaryImage);
			//RemoveNode(segName);
			this->GetDataStorage()->Add(segmentedBinaryImageNode);
		}
		else {
			segNode->SetData(segmentedBinaryImage);
		}

		this->GlobalReinit();		
	}

}

/*delete all nodes*/
void MCTAnalyzer::DeleteAllNodes() {
	mitk::DataStorage::SetOfObjects::ConstPointer newNodes = GetDataStorage()->GetAll();

	for (mitk::DataStorage::SetOfObjects::ConstIterator iter = newNodes->Begin(), iterEnd = newNodes->End(); iter != iterEnd;
		++iter)
	{
		mitk::DataNode::Pointer node = iter->Value();

		if (node) { 
			GetDataStorage()->Remove(node);
		}
	}
}

void MCTAnalyzer::GlobalReinit() {
	mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();

	if (renderWindow == NULL) return;

	mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());
}

/*Execute manual  Segmentation*/
void MCTAnalyzer::ManualSegmentation() {

	//DeleteAllNodes();

	m_Controls.SegmentationWidget->show();

	QString objectName = ((QPushButton*)sender())->objectName();
	QString lineEditQstr;
	lineEditQstr.append("T_");
	lineEditQstr.append(objectName);

	QLineEdit* lineEdit = m_Controls.widget_7->findChild<QLineEdit*>(lineEditQstr);

	QString path = lineEdit->text();

	mitk::Image::Pointer mImage;

	//if (images.count(lineEditQstr) == 0){
	mImage = Generate3DImageFromImageStack(path);
	/*	images[lineEditQstr] = mImage;
	}
	else {
		mImage = images[lineEditQstr];
	}*/
	
	if (mImage) {

		//std::string imgName = "image_" + lineEditQstr.toStdString();
		std::string imgName = "image" ;
		
		mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode(imgName);

		if (!node) {
			mitk::DataNode::Pointer newNode = mitk::DataNode::New();
			newNode->SetData(mImage);
			// set some properties
			//newNode->SetProperty("binary", mitk::BoolProperty::New(true));

			newNode->SetProperty("name", mitk::StringProperty::New(imgName));
			this->GetDataStorage()->Add(newNode);
		}
		else {
			node->SetData(mImage);
		}

		std::string segName = "segmentation";

		RemoveNode(segName);
		
		m_Controls.SegmentationWidget->SetDataStorage(this->GetDataStorage());
		
		if (thresholds.count(objectName) != 0) {
			Threshold t = thresholds[objectName];
			m_Controls.SegmentationWidget->SetImageNode(mImage, objectName, t.min, t.max);
		}
		else{
			m_Controls.SegmentationWidget->SetImageNode(mImage, objectName, -1, -1);
		}

		this->GlobalReinit();
	}
	
}



void MCTAnalyzer::ChooseDirectories(){

	QString objectName = ((QPushButton*)sender())->objectName();
	MITK_WARN << objectName;
	QString lineEditQstr;
	lineEditQstr.append("T_");
	lineEditQstr.append(objectName);

	QLineEdit* lineEdit = m_Controls.widget_7->findChild<QLineEdit*>(lineEditQstr);

	QString lineText = lineEdit->text();

	QString s;
	
	if (!lineText.isEmpty()){
		s.append(lineText);
		//s.append(";");
	}
	
	

	QString dir = QFileDialog::getExistingDirectory(NULL, tr("Open Directory"),
		path,
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	path = dir;

	s.append(dir);
	//pathsForSample.insert(counter, paths);

	lineEdit->setText(s);
	
}

void MCTAnalyzer::SetOutputDirectory(){

	QString dir = QFileDialog::getExistingDirectory(NULL, tr("Open Directory"),
		"/",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	m_Controls.outputDirectoryInput->setText(dir);

}

/*Get segmentation of scaffold via threshold*/
mitk::Image::Pointer MCTAnalyzer::ScaffoldSegmentation(const mitk::Image::Pointer originalImage, const QString& objectName) {

	mitk::Image::Pointer segmentedBinaryImage;

	if (thresholds.count(objectName) == 0) {
		MctSegmentationFilter::Pointer segmentedBinaryFilter = MctSegmentationFilter::New();
		segmentedBinaryFilter->SetInput(originalImage);
		segmentedBinaryFilter->Update();

		segmentedBinaryImage = segmentedBinaryFilter->GetOutput();

	}
	else {
		Threshold t = thresholds[objectName];
		ConnectedSegmentationFilter::Pointer segmentationFilter = ConnectedSegmentationFilter::New();
		segmentationFilter->SetLowerThreshold(t.min);
		segmentationFilter->SetUpperThreshold(t.max);
		segmentationFilter->SetInput(originalImage);
		segmentationFilter->SetUseMedian(false);
		segmentationFilter->Update();

		MITK_WARN << "... done";
		segmentedBinaryImage = segmentationFilter->GetOutput();
	}

	// End Segmentation

	// Show Segmentation
	if (showImages){
		mitk::DataNode::Pointer segmentedBinaryImageNode = mitk::DataNode::New();
		segmentedBinaryImageNode->SetData(segmentedBinaryImage);
		segmentedBinaryImageNode->SetProperty("name", mitk::StringProperty::New("binary segmented Image"));
		this->GetDataStorage()->Add(segmentedBinaryImageNode);
	}
	return segmentedBinaryImage;

}

/*Calculate scaffold surface*/
mitk::Surface::Pointer MCTAnalyzer::CalculateScaffoldSurface(const mitk::Image::Pointer segmentedImage, double& surface, double pixelSize, const mitk::Image::Pointer originalImage) {
	mitk::ImageToSurfaceFilter::Pointer surfaceFilter = mitk::ImageToSurfaceFilter::New();
	surfaceFilter->SetInput(segmentedImage);
	surfaceFilter->SetThreshold(0.5);
	surfaceFilter->ReleaseDataFlagOn();
	surfaceFilter->Update();

	mitk::Surface::Pointer segmentedSurface = surfaceFilter->GetOutput();

	vtkSmartPointer< vtkMassProperties > massProp =
		vtkSmartPointer< vtkMassProperties >::New();
	massProp->SetInputData(segmentedSurface->GetVtkPolyData());

	surface = massProp->GetSurfaceArea() * std::pow(pixelSize, 2.0);
	MITK_WARN << massProp->GetSurfaceArea() << "_" << massProp->GetVolume() << "_";

	vtkSmartPointer<vtkCenterOfMass> centerOfMassFilter = vtkSmartPointer<vtkCenterOfMass>::New();

	centerOfMassFilter->SetInputData(segmentedSurface->GetVtkPolyData());

	centerOfMassFilter->SetUseScalarsAsWeights(false);
	centerOfMassFilter->Update();

	
	centerOfMassFilter->GetCenter(center);

	std::cout << "Center of mass is " << center[0] << " " << center[1] << " " << center[2] << std::endl;

	if (showImages){
		mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
		surfaceNode->SetData(segmentedSurface);
		surfaceNode->SetProperty("color", mitk::ColorProperty::New(0.0781, 0.19531, 1.0));
		surfaceNode->SetOpacity(0.5);
		surfaceNode->SetName("segmented_Surface");
		this->GetDataStorage()->Add(surfaceNode);
	}
	return segmentedSurface;
}

/*Approximate convex hull*/
mitk::Surface::Pointer MCTAnalyzer::CalculateConvexSurface(const mitk::Surface::Pointer segmentedSurface, double& volume, double pixelSize) {
	vtkSmartPointer<vtkSphereSource> sphereSource =
	vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->SetCenter(center[0], center[1], center[2]);
	sphereSource->SetRadius(1000);
	sphereSource->SetPhiResolution(50);
	sphereSource->SetThetaResolution(50);
	sphereSource->ReleaseDataFlagOn();
	sphereSource->Update();

	vtkSmartPointer<vtkSmoothPolyDataFilter> smoothFilter =
		vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
	smoothFilter->SetInputConnection(0, sphereSource->GetOutputPort());
	smoothFilter->SetInputData(1, segmentedSurface->GetVtkPolyData());
	smoothFilter->FeatureEdgeSmoothingOn();
	smoothFilter->ReleaseDataFlagOn();
	smoothFilter->BoundarySmoothingOn();
	smoothFilter->Update();


	MITK_WARN << "numVerts: " << segmentedSurface->GetVtkPolyData()->GetNumberOfPoints();


	vtkSmartPointer<vtkPolyData> pd2 = vtkSmartPointer<vtkPolyData>::New();
	pd2->SetPoints(segmentedSurface->GetVtkPolyData()->GetPoints());

	vtkSmartPointer<vtkHull> hullFilter =
		vtkSmartPointer<vtkHull>::New();
	hullFilter->SetInputData(pd2);
	hullFilter->AddCubeFacePlanes();
	hullFilter->Update();

	vtkSmartPointer<vtkCleanPolyData> cleaner =
		vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner->SetInputData(smoothFilter->GetOutput());
	cleaner->ReleaseDataFlagOn();

	

	vtkSmartPointer<vtkDelaunay3D> delaunay3D =
		vtkSmartPointer<vtkDelaunay3D>::New();
	delaunay3D->SetInputConnection(cleaner->GetOutputPort());
	//delaunay3D->SetInputData(pd2);
	delaunay3D->ReleaseDataFlagOn();
	delaunay3D->Update();



	int numTetras = 0;
	int numLines = 0;
	int numTris = 0;
	int numVerts = 0;
	auto it = segmentedSurface->GetVtkPolyData()->NewCellIterator();
	for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextCell())
	{
		if (it->GetCellType() == VTK_TETRA)
		{
			numTetras++;

		}
		else if (it->GetCellType() == VTK_LINE)
		{
			numLines++;

		}
		else if (it->GetCellType() == VTK_TRIANGLE)
		{
			numTris++;

		}
		else if (it->GetCellType() == VTK_VERTEX)
		{
			numVerts++;

		}
	}
	it->Delete();

	std::stringstream ss;
	MITK_WARN << "numTetras: " << numTetras << std::endl;
	MITK_WARN << "numLines: " << numLines << std::endl;
	MITK_WARN << "numTris: " << numTris << std::endl;
	MITK_WARN << "numVerts: " << numVerts;


	

	vtkSmartPointer<vtkGeometryFilter> geometryFilter =
		vtkSmartPointer<vtkGeometryFilter>::New();
	geometryFilter->SetInputConnection(delaunay3D->GetOutputPort());
	geometryFilter->ReleaseDataFlagOn();
	geometryFilter->Update();

	
	vtkSmartPointer<vtkPolyData> pd = geometryFilter->GetOutput();

	vtkSmartPointer<vtkPolyData> pd3 = hullFilter->GetOutput();


	vtkSmartPointer< vtkMassProperties > massPropTV =
		vtkSmartPointer< vtkMassProperties >::New();
	massPropTV->ReleaseDataFlagOn();
	massPropTV->SetInputData(pd);

	volume = massPropTV->GetVolume() * std::pow(pixelSize, 3.0);
	MITK_WARN << "Total_Volume" << massPropTV->GetVolume() << "_";

	mitk::Surface::Pointer convexHullSurface = mitk::Surface::New();
	convexHullSurface->SetVtkPolyData(pd);

	mitk::Surface::Pointer convexHullSurface2 = mitk::Surface::New();
	convexHullSurface2->SetVtkPolyData(pd3);

	if (showImages){
		mitk::DataNode::Pointer convexHullSurfaceNode = mitk::DataNode::New();
		convexHullSurfaceNode->SetData(convexHullSurface2);
		convexHullSurfaceNode->SetProperty("color", mitk::ColorProperty::New(0.0781, 0.19531, 1.0));
		convexHullSurfaceNode->SetOpacity(0.5);
		convexHullSurfaceNode->SetName("Convex_Hull_Surface");
		this->GetDataStorage()->Add(convexHullSurfaceNode);
	}
	return convexHullSurface;

}

/* Create Mask Of Convex Surface
*/
mitk::Image::Pointer MCTAnalyzer::CreateMaskOfConvexSurface( mitk::Surface::Pointer convexHullSurface,  mitk::Image::Pointer originalImage) {
	mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

	mitk::Image::Pointer additionalInputImage = mitk::Image::New();
	additionalInputImage->Initialize(mitk::MakeScalarPixelType<unsigned int>(), *originalImage->GetGeometry());

	// Arrange the filter
	surfaceToImageFilter->MakeOutputBinaryOn();
	surfaceToImageFilter->SetBackgroundValue(0);
	surfaceToImageFilter->SetInput(convexHullSurface);
	surfaceToImageFilter->SetImage(additionalInputImage);
	surfaceToImageFilter->Update();

	mitk::Image::Pointer test = surfaceToImageFilter->GetOutput();
	MITK_WARN << "Done Masking";

	if (showImages){
		mitk::DataNode::Pointer cuttedSegmentNode2 = mitk::DataNode::New();
		cuttedSegmentNode2->SetData(test);

		cuttedSegmentNode2->SetName("Convex_Surface_Mask");



		this->GetDataStorage()->Add(cuttedSegmentNode2);
	}
	return test;

}

void MCTAnalyzer::DoHoughTransform (const mitk::Image::Pointer  maski){


	/*
	typedef int PixelType;
	typedef itk::Image< PixelType, 3 > RealImageType;
	RealImageType::Pointer segmentedBinaryMask;



	mitk::CastToItkImage(maski, segmentedBinaryMask);

	typedef itk::BinaryThresholdImageFilter<RealImageType, RealImageType>	BinaryThresholdFilterType;

	BinaryThresholdFilterType::Pointer regionOfInterestFilter = BinaryThresholdFilterType::New();
	regionOfInterestFilter->SetInput(segmentedBinaryMask);
	regionOfInterestFilter->SetLowerThreshold(0);
	regionOfInterestFilter->SetUpperThreshold(0);
	regionOfInterestFilter->SetInsideValue(1);
	regionOfInterestFilter->ReleaseDataFlagOn();
	regionOfInterestFilter->SetOutsideValue(0);
	regionOfInterestFilter->Update();


	mitk::Image::Pointer mitkResult = mitk::Image::New();


	mitk::GrabItkImageMemory(regionOfInterestFilter->GetOutput(), mitkResult);

	mitk::DataNode::Pointer nodeOutHTF3D = mitk::DataNode::New();
	nodeOutHTF3D->SetData(mitkResult);
	nodeOutHTF3D->SetName("Accumulator Image2");
	this->GetDataStorage()->Add(nodeOutHTF3D);
	*/
	HoughTransform3DInitParameters initParams3D;
	initParams3D.numSpheres = 100;
	initParams3D.minRadius = 0.0;
	initParams3D.maxRadius = 300.0;
	initParams3D.sigmaGradient = 1;
	initParams3D.variance = 1;
	initParams3D.sphereRadiusRatio = 0.5;
	initParams3D.votingRadiusRatio = 0.15;
	initParams3D.threshold = 1;
	initParams3D.outputThreshold = 0.5;
	initParams3D.gradientThreshold = 1;
	initParams3D.NbOfThreads = 1;
	initParams3D.samplingRatio = 1;

	HoughTransformation3DFilter::Pointer HTF3D = HoughTransformation3DFilter::New();
	HTF3D->SetInput(maski);
	HTF3D->InitFilter(initParams3D);
	HTF3D->Update();
	mitk::Image::Pointer outHTF3D = HTF3D->GetOutput();

	mitk::DataNode::Pointer nodeOutHTF3D2 = mitk::DataNode::New();
	nodeOutHTF3D2->SetData(outHTF3D);
	nodeOutHTF3D2->SetName("Accumulator Image");
	this->GetDataStorage()->Add(nodeOutHTF3D2);

	HoughTransform3DStatistics sphreStats = HTF3D->getStatistics();
	std::vector<mitk::Surface::Pointer> allSpheres = HTF3D->getSphereObjects();
	int numSpheres = allSpheres.size();
	for (int i = 0; i < numSpheres; ++i){
		mitk::Surface::Pointer sphereSurface = allSpheres.at(i);
		mitk::DataNode::Pointer sphereNode = mitk::DataNode::New();
		sphereNode->SetData(sphereSurface);
		sphereNode->SetName("Sphere " + std::to_string(i));
		this->GetDataStorage()->Add(sphereNode);
	}

}


double MCTAnalyzer::op_volume(double i) { return (1 / 6) * 3.14159265359 * std::pow(i , 3.0); }


/*Output results */
void MCTAnalyzer::WriteResults(QString name, QString sample){



	m_EndTime = std::chrono::system_clock::now();
	std::chrono::hours   hh = std::chrono::duration_cast<std::chrono::hours>(m_EndTime - m_StartTime);
	std::chrono::minutes mm = std::chrono::duration_cast<std::chrono::minutes>(m_EndTime - m_StartTime);
	std::chrono::seconds ss = std::chrono::duration_cast<std::chrono::seconds>(m_EndTime - m_StartTime);
	mm %= 60;
	ss %= 60;

	MITK_INFO << "Optimizing took " << hh.count() << "h, " << mm.count() << "m and " << ss.count() << "s";
	

	QString dir = m_Controls.outputDirectoryInput->text();
	/*QRegExp rx("(?:\\s*)(T|t)(\\d+)(?:\\s*)");
	int pos = rx.indexIn(name);     // returns -1 (no match)

	MITK_WARN << pos << " " << rx.cap(0);
	//QStringList list = rx.capturedTexts();*/
	QString fileName = dir + "/" + sample + "_" + name + ".csv";


	MITK_WARN << name << "," << sample; 

	MITK_WARN << fileName ;

	std::ofstream ofs(fileName.toStdString(), std::ios::trunc);


	for (auto const& x : resultMap){
		ofs << x.first;
		ofs << ";";

	}

	ofs << "Time;";

	ofs.precision(15);
	ofs << "\n";
	for (auto const& x : resultMap){
		std::stringstream sstr;
		sstr << x.second;
		std::string modString =  sstr.str();
		std::replace(modString.begin(), modString.end(), '.', ',');
		ofs << modString;
		ofs << ";";
	}

	ofs << hh.count() << "h, " << mm.count() << "m and " << ss.count() << "s";
	ofs << ";";
	ofs.flush();
	ofs.close();

	resultMap.clear();
}

/*Output pore size range*/
void MCTAnalyzer::WritePoreSizeRange(QString name, QString sample){


	QString dir = m_Controls.outputDirectoryInput->text();
	
	QString fileName = dir + "/" + "PSR_" + sample + "_" + name + ".csv";


	MITK_WARN << name << "," << sample;

	MITK_WARN << fileName;

	std::ofstream ofs(fileName.toStdString(), std::ios::trunc);


	for (auto const& x : poreSizeRange){
		ofs << x.first;
		ofs << ";";

	}

	ofs.precision(15);
	ofs << "\n";
	for (auto const& x : poreSizeRange){
		std::stringstream sstr;
		sstr << x.second.size();
		std::string modString = sstr.str();
		std::replace(modString.begin(), modString.end(), '.', ',');
		ofs << modString;
		ofs << ";";
	}

	
	ofs.flush();
	ofs.close();

	if (!(counter == 1 && examinationCounter == 0)){
		MITK_WARN << "clear pore size";
		poreSizeRange.clear();

	}

	
	
}



mitk::Image::Pointer MCTAnalyzer::Generate3DImageFromImageStack(const QString& path) {
	if (!path.isEmpty()){

		mitk::Image::Pointer mImage;

		QDir directory(path);
		QStringList images = directory.entryList(QStringList() << "*.tif" << "*.TIF", QDir::Files);

		vtkSmartPointer<vtkStringArray> sA = vtkSmartPointer<vtkStringArray>::New();

		foreach(QString filename, images) {
			sA->InsertNextValue(path.toStdString() + "/" + filename.toStdString());
			//MITK_WARN << filename.toStdString();

		}

		//std::string filepath = paths.at(i).toStdString() + "/" + sA->GetValue(0);

		//MITK_WARN << sA->GetSize() << " " << sA->GetValue(0);

		vtkSmartPointer<vtkTIFFReader> reader =
			vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileNames(sA);
		reader->Update();
		vtkSmartPointer<vtkImageData> imageData = reader->GetOutput();
		imageData->SetSpacing(1.0f, 1.0f, 1.0f);
		//MITK_WARN << imageData->GetExtent()[0] << " " << imageData->GetExtent()[1] << " " << imageData->GetExtent()[2] << " " << imageData->GetExtent()[3] << " " << imageData->GetExtent()[4] << " " << imageData->GetExtent()[5];

		//MITK_WARN << "Range" << imageData->GetScalarTypeMin();
		//MITK_WARN << "Range" << imageData->GetScalarTypeMax();


		mImage = mitk::Image::New();
		mImage->Initialize(imageData);
		mImage->SetVolume(imageData->GetScalarPointer());


		mitk::ScalarType spacing[3];
		spacing[0] = 1.0f;
		spacing[1] = 1.0f;
		spacing[2] = 1.0f;
		mitk::Point3D origin;
		origin.Fill(0);
		mImage->SetSpacing(spacing);
		mImage->SetOrigin(origin);


		
		return mImage;

	}

	return nullptr;

}


mitk::Image::Pointer MCTAnalyzer::CreateContourFromMask(const mitk::Image::Pointer  maski, const mitk::Image::Pointer segImage){

	typedef int PixelType;
	typedef itk::Image< PixelType, 3 > RealImageType;
	RealImageType::Pointer segmentedBinaryMask;

	RealImageType::Pointer segmentedBinary;


	mitk::CastToItkImage(maski, segmentedBinaryMask);
	mitk::CastToItkImage(segImage, segmentedBinary);

	typedef itk::BinaryThresholdImageFilter<RealImageType, RealImageType>	BinaryThresholdFilterType;

	BinaryThresholdFilterType::Pointer regionOfInterestFilter = BinaryThresholdFilterType::New();
	regionOfInterestFilter->SetInput(segmentedBinaryMask);
	regionOfInterestFilter->SetLowerThreshold(0);
	regionOfInterestFilter->SetUpperThreshold(0);
	regionOfInterestFilter->SetInsideValue(1);
	regionOfInterestFilter->ReleaseDataFlagOn();
	regionOfInterestFilter->SetOutsideValue(0);
	regionOfInterestFilter->Update();


	typedef itk::BinaryContourImageFilter <RealImageType, RealImageType >
		binaryContourImageFilterType;

	binaryContourImageFilterType::Pointer binaryContourFilter
		= binaryContourImageFilterType::New();
	binaryContourFilter->SetInput(segmentedBinaryMask);
	binaryContourFilter->SetFullyConnected(false); // true makes thicker contours
	binaryContourFilter->SetBackgroundValue(0);
	binaryContourFilter->SetForegroundValue(1);
	binaryContourFilter->Update();

	mitk::Image::Pointer mitkResult = mitk::Image::New();


	typedef itk::AddImageFilter <RealImageType, RealImageType >
		AddImageFilterType;

	AddImageFilterType::Pointer addFilter
		= AddImageFilterType::New();
	addFilter->SetInput1(binaryContourFilter->GetOutput());
	addFilter->SetInput2(segmentedBinary);
	addFilter->Update();

	mitk::GrabItkImageMemory(addFilter->GetOutput(), mitkResult);

	if (showImages){
		mitk::DataNode::Pointer dMNode3 = mitk::DataNode::New();
		dMNode3->SetData(mitkResult);
		dMNode3->SetProperty("name", mitk::StringProperty::New("Contour4"));
		this->GetDataStorage()->Add(dMNode3);
	}

	return mitkResult;
}



