/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

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
// Qmitk
#include "MCTAnalyzer.h"
#include "mctAnalyzerCropFilter.h"
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
#include <vector>
#include <algorithm>
#include "mitkSurface.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include <vtkSphereSource.h>
#include <vtkSmoothPolyDataFilter.h>
#include <mitkImagePixelReadAccessor.h>


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

void MCTAnalyzer::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file

	verticalLayout = new QVBoxLayout;
	path = "/";
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
 
  connect(m_Controls.pushButton, SIGNAL(clicked()), this, SLOT(AddSegment()));
  connect(m_Controls.outputDirectoryButton, SIGNAL(clicked()), this, SLOT(SetOutputDirectory()));
  connect(m_Controls.all, SIGNAL(clicked(bool)), this, SLOT(toggled(bool)));
  connect(m_Controls.c_showImages, SIGNAL(clicked(bool)), this, SLOT(ShowImages(bool)));
 
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

void MCTAnalyzer::AddSegment(){
	QHBoxLayout *horizontalLayout = new QHBoxLayout;


	QString qstr;
	qstr.append("Sample");
	qstr.append(QString::number(counter));
	

	QLineEdit *l = new QLineEdit();
	l->setFixedWidth(50);
	l->setText(qstr);
	l->setObjectName(qstr);



	QLineEdit *le = new QLineEdit();
	QString lineEditQstr;
	lineEditQstr.append("lineEdit");
	lineEditQstr.append(QString::number(counter));
	le->setObjectName(lineEditQstr);

	QPushButton *train_button = new QPushButton();
	train_button->setText(tr("..."));
	train_button->setFixedWidth(20);
	train_button->setObjectName(QString(QString::number(counter)));
	QObject::connect(train_button, SIGNAL(clicked()), this, SLOT(ChooseDirectories()));

	QLineEdit *ple = new QLineEdit();
	QString plineEditQstr;
	plineEditQstr.append("pixel");
	plineEditQstr.append(QString::number(counter));
	ple->setFixedWidth(50);
	ple->setText("11.78");
	ple->setObjectName(plineEditQstr);

	horizontalLayout->addWidget(l);
	horizontalLayout->addWidget(le);
	horizontalLayout->addWidget(train_button);
	horizontalLayout->addWidget(ple);

	verticalLayout->addLayout(horizontalLayout);


	m_Controls.widget_7->setLayout(verticalLayout);
	counter++;


}


void MCTAnalyzer::ChooseDirectories(){

	QString objectName = ((QPushButton*)sender())->objectName();

	QString lineEditQstr;
	lineEditQstr.append("lineEdit");
	lineEditQstr.append(objectName);

	QLineEdit* lineEdit = m_Controls.widget_7->findChild<QLineEdit*>(lineEditQstr);

	/*QMessageBox msgBox;
	msgBox.setWindowTitle("Hello");
	msgBox.setText("You Clicked " + ((QPushButton*)sender())->objectName());
	msgBox.exec();


	QFileDialog w;
	QStringList paths;
	w.setFileMode(QFileDialog::DirectoryOnly);
	w.setOption(QFileDialog::DontUseNativeDialog, true);
	QListView *l = w.findChild<QListView*>("listView");
	if (l) {
		l->setSelectionMode(QAbstractItemView::MultiSelection);
	}
	QTreeView *t = w.findChild<QTreeView*>();
	if (t) {
		t->setSelectionMode(QAbstractItemView::MultiSelection);
	}

	QString s;
	if (w.exec()){
		paths = w.selectedFiles();
		MITK_WARN << paths.size();
		for (int i = 0; i < paths.size(); ++i){
			QFile f(paths.at(i));
			QFileInfo fileInfo(f.fileName());
			QString filename(fileInfo.fileName());
			s.append(filename);
			s.append(";");
		}
			
	}*/
	QString lineText = lineEdit->text();

	QString s;
	
	if (!lineText.isEmpty()){
		s.append(lineText);
		s.append(";");
	}
	
	

	QString dir = QFileDialog::getExistingDirectory(NULL, tr("Open Directory"),
		path,
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	path = dir;

	s.append(dir);
	//pathsForSample.insert(counter, paths);

	lineEdit->setText(s);


	/*for (int i = 0; i < paths.size(); ++i){
		QDir directory(paths.at(i));
		QStringList images = directory.entryList(QStringList() << "*.tif" << "*.TIF", QDir::Files);
		foreach(QString filename, images) {
			MITK_WARN << filename;
		}
	}*/
	
}

void MCTAnalyzer::SetOutputDirectory(){

	QString dir = QFileDialog::getExistingDirectory(NULL, tr("Open Directory"),
		"/",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	m_Controls.outputDirectoryInput->setText(dir);

}

mitk::Image::Pointer MCTAnalyzer::ScaffoldSegmentation(const mitk::Image::Pointer originalImage) {

	MctSegmentationFilter::Pointer segmentedBinaryFilter = MctSegmentationFilter::New();
	segmentedBinaryFilter->SetInput(originalImage);
	segmentedBinaryFilter->Update();

	mitk::Image::Pointer segmentedBinaryImage = segmentedBinaryFilter->GetOutput();
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

	double center[3];
	centerOfMassFilter->GetCenter(center);

	std::cout << "Center of mass is " << center[0] << " " << center[1] << " " << center[2] << std::endl;
	//typedef itk::Image<int, 3> InputImageType;
	//typename InputImageType::IndexType inputIndex;


	/*itk::Index<3> currentIndex;
	


	mitk::Point3D point;
		
	point[0] = center[0];
	point[1] = center[1];
	point[2] = center[2];
	mitk::BaseGeometry::Pointer geo = originalImage->GetGeometry();
	geo->WorldToIndex(point, currentIndex);

		
	if (geo->IsIndexInside(currentIndex))
	{
		//for (int i = 0; i < 3; i++){

		mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
		plane->InitializeStandardPlane(geo, mitk::PlaneGeometry::Frontal, currentIndex[1], true, false);


		mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New();
		extractor->SetInput(originalImage);
		extractor->SetWorldGeometry(plane);
		extractor->Update();
		mitk::Image::Pointer extractedPlane;
		extractedPlane = extractor->GetOutput();
		


		typedef itk::Image< unsigned int, 3 >  ImageType;
		typedef itk::ConfidenceConnectedImageFilter<ImageType, ImageType> ConfidenceConnectedFilterType;

		ImageType::Pointer maskConvexHullImageITK;
		mitk::CastToItkImage(originalImage, maskConvexHullImageITK);

		ConfidenceConnectedFilterType::Pointer confidenceConnectedFilter = ConfidenceConnectedFilterType::New();
		confidenceConnectedFilter->SetInitialNeighborhoodRadius(3);
		confidenceConnectedFilter->SetMultiplier(3);
		confidenceConnectedFilter->SetNumberOfIterations(25);
		confidenceConnectedFilter->SetReplaceValue(255);

		// Set seed
		ImageType::IndexType seed;
		seed[0] = currentIndex[0];
		seed[1] = currentIndex[1];
		seed[2] = currentIndex[2];
		confidenceConnectedFilter->SetSeed(seed);
		confidenceConnectedFilter->SetInput(maskConvexHullImageITK);

		
			

			MctAnalyzerBinarySegmentationFilter::Pointer mF = MctAnalyzerBinarySegmentationFilter::New();
			mF->SetInput(extractedPlane); // don't forget this
			mF->Update();

			mitk::DataNode::Pointer surfaceNode2 = mitk::DataNode::New();
			surfaceNode2->SetData(mitk::GrabItkImageMemory(confidenceConnectedFilter->GetOutput()));
			//surfaceNode->SetProperty("color", mitk::ColorProperty::New(0.0781, 0.19531, 1.0));
			//surfaceNode->SetOpacity(0.5);
			surfaceNode2->SetName("slices_Surfce2");
			this->GetDataStorage()->Add(surfaceNode2);

			mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
			surfaceNode->SetData(extractedPlane);
			//surfaceNode->SetProperty("color", mitk::ColorProperty::New(0.0781, 0.19531, 1.0));
			//surfaceNode->SetOpacity(0.5);
			surfaceNode->SetName("slices_Surfce");
			this->GetDataStorage()->Add(surfaceNode);
		//}
	}
*/
	



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

mitk::Surface::Pointer MCTAnalyzer::CalculateConvexSurface(const mitk::Surface::Pointer segmentedSurface, double& volume, double pixelSize) {
	vtkSmartPointer<vtkSphereSource> sphereSource =
		vtkSmartPointer<vtkSphereSource>::New();
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


	vtkSmartPointer<vtkCleanPolyData> cleaner =
		vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner->SetInputConnection(smoothFilter->GetOutputPort());
	cleaner->ReleaseDataFlagOn();

	vtkSmartPointer<vtkDelaunay3D> delaunay3D =
		vtkSmartPointer<vtkDelaunay3D>::New();
	delaunay3D->SetInputConnection(cleaner->GetOutputPort());
	delaunay3D->ReleaseDataFlagOn();
	delaunay3D->Update();

	vtkSmartPointer<vtkGeometryFilter> geometryFilter =
		vtkSmartPointer<vtkGeometryFilter>::New();
	geometryFilter->SetInputConnection(delaunay3D->GetOutputPort());
	geometryFilter->ReleaseDataFlagOn();
	geometryFilter->Update();

	vtkSmartPointer<vtkPolyData> pd = geometryFilter->GetOutput();

	vtkSmartPointer< vtkMassProperties > massPropTV =
		vtkSmartPointer< vtkMassProperties >::New();
	massPropTV->ReleaseDataFlagOn();
	massPropTV->SetInputData(pd);

	volume = massPropTV->GetVolume() * std::pow(pixelSize, 3.0);
	MITK_WARN << "Total_Volume" << massPropTV->GetVolume() << "_";

	mitk::Surface::Pointer convexHullSurface = mitk::Surface::New();
	convexHullSurface->SetVtkPolyData(pd);

	if (showImages){
		mitk::DataNode::Pointer convexHullSurfaceNode = mitk::DataNode::New();
		convexHullSurfaceNode->SetData(convexHullSurface);
		convexHullSurfaceNode->SetProperty("color", mitk::ColorProperty::New(0.0781, 0.19531, 1.0));
		convexHullSurfaceNode->SetOpacity(0.5);
		convexHullSurfaceNode->SetName("Convex_Hull_Surface");
		this->GetDataStorage()->Add(convexHullSurfaceNode);
	}
	return convexHullSurface;

}

mitk::Image::Pointer MCTAnalyzer::CreateMaskOfConvexSurface( mitk::Surface::Pointer convexHullSurface,  mitk::Image::Pointer originalImage) {
	mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

	mitk::Image::Pointer additionalInputImage = mitk::Image::New();
	additionalInputImage->Initialize(mitk::MakeScalarPixelType<unsigned int>(), *originalImage->GetGeometry());

	// Arrange the filter
	surfaceToImageFilter->MakeOutputBinaryOn();
	surfaceToImageFilter->SetInput(convexHullSurface);
	surfaceToImageFilter->SetImage(additionalInputImage);
	surfaceToImageFilter->Update();

	mitk::Image::Pointer test = surfaceToImageFilter->GetOutput();
	MITK_WARN << "Done Masking";

	if (showImages){
		mitk::DataNode::Pointer cuttedSegmentNode2 = mitk::DataNode::New();
		cuttedSegmentNode2->SetData(test);

		cuttedSegmentNode2->SetName("Argh2222");



		this->GetDataStorage()->Add(cuttedSegmentNode2);
	}
	return test;

}

void MCTAnalyzer::FindPores(const mitk::Image::Pointer segmentedBinaryImage, double pixelSize, int& lowerPercentile, int& upperPercentile, std::vector<double>& filteredRadii, const mitk::Image::Pointer  maski, const mitk::Surface::Pointer convexHull){
	
	
	
	
	
	
	
	
	MctAnalyzerDistanceMapFilter::Pointer dF = MctAnalyzerDistanceMapFilter::New();
	dF->SetInput(segmentedBinaryImage); // don't forget this
	dF->Update();
	mitk::Image::Pointer distanceMap = dF->GetOutput();

	MctAnalyzerCropFilter::Pointer cF = MctAnalyzerCropFilter::New();
	cF->SetInput(distanceMap);
	cF->SetBoundingObject(maski);
	cF->Update();

	mitk::Image::Pointer test = cF->GetOutput();

	
	

	if (showImages){
		mitk::DataNode::Pointer dMNode = mitk::DataNode::New();
		dMNode->SetData(test);
		dMNode->SetProperty("name", mitk::StringProperty::New("distance_Map"));
		this->GetDataStorage()->Add(dMNode);
	}

	

	/*MctAnalyzerCropFilter::Pointer cF = MctAnalyzerCropFilter::New();
	cF->SetInput(mF->GetOutput());
	cF->SetBoundingObject(maski);
	cF->Update();

	mitk::Image::Pointer test = cF->GetOutput();*/
	

	mitk::BaseGeometry::Pointer boGeometry = convexHull->GetGeometry();
	mitk::BaseGeometry::Pointer inputImageGeometry = test->GetSlicedGeometry();

	mitk::BoundingBox::Pointer boBoxRelativeToImage =
		boGeometry->CalculateBoundingBoxRelativeToTransform(inputImageGeometry->GetIndexToWorldTransform());


	MctAnalyzerRegionalMaximaFilter::Pointer mF = MctAnalyzerRegionalMaximaFilter::New();
	mF->SetInput(test); // don't forget this
	mF->SetBoundingObject(boBoxRelativeToImage);
	mF->Update();

	mitk::Image::Pointer regionalMaxima = mF->GetOutput();



	/*mitk::CastToItkImage(mImage, originalImageITK);


	ItkInputImageIteratorType  inputIt(clonedImage, inputRegionOfInterest);*/


	unsigned int* rdims = regionalMaxima->GetDimensions();
	
	if (showImages){
		mitk::DataNode::Pointer rNode = mitk::DataNode::New();
		rNode->SetData(regionalMaxima);
		rNode->SetProperty("name", mitk::StringProperty::New("circle_origin_Map"));
		this->GetDataStorage()->Add(rNode);
	}

	/*
	mitk::PointSet::Pointer pointSet = mitk::PointSet::New();





	mitk::Point3D point;
	mitk::ImagePixelReadAccessor<unsigned char, 3> readAccess(regionalMaxima);
	itk::Index<3> index;
	int cou = 0;
	for (int z = 0; z < rdims[2]; z++)
	{
		for (int y = 0; y < rdims[1]; y++)
		{
			for (int x = 0; x < rdims[0]; x++)
			{

				index = { { static_cast<itk::IndexValueType>(x), static_cast<itk::IndexValueType>(y), static_cast<itk::IndexValueType>(z) } };
				float distance = 0.0;

				try
				{

					distance = readAccess.GetPixelByIndex(index);
					if (distance != 0){

						point[0] = x;
						point[1] = y;
						point[2] = z;
						pointSet->InsertPoint(point);
						cou++;
					}
				}
				catch (mitk::Exception& e)
				{
					MITK_ERROR << "Image read exception!" << e.what();
				}

				/*itk::Index<3> currentIndex;
				currentIndex[0] = x;
				currentIndex[1] = y;
				currentIndex[2] = z;

				mitk::Point3D point;
				point[0] = x;
				point[1] = y;
				point[2] = z;
				readAccess.GetPixelByIndex(currentIndex);



				if (regionalMaxima->GetPixelValueByIndex(currentIndex) != 0){
				pointSet->InsertPoint(point);
				}*/
			//}
		//}
	//}


	//MITK_WARN << "COunte" << cou;
	std::vector<double> radii;

	mitk::PointSet::Pointer pointSet = mF->GetPointSet();

	int numberOfPoints = pointSet->GetSize();
	for (int i = 0; i < numberOfPoints; i++)
	{
		mitk::Point3D point = pointSet->GetPoint(i);
		itk::Index<3> currentIndex;
		currentIndex[0] = point[0];
		currentIndex[1] = point[1];
		currentIndex[2] = point[2];


		double radius = distanceMap->GetPixelValueByIndex(currentIndex);
		radii.push_back(radius);

	}


	std::sort(radii.begin(), radii.end());
	int vectorSize = radii.size();

	for (int i = 0; i <= vectorSize; i++) {
		radii[i] = radii[i] * 2 * pixelSize;
	}

	double cLower, cUpper;

	cLower = *std::min_element(radii.begin(), radii.end());
	cUpper = *std::max_element(radii.begin(), radii.end());

	MITK_WARN << "Biggest" << cUpper << " Lowest" << cLower;

	
	lowerPercentile = ceil(vectorSize*0.05);

	upperPercentile = ceil(vectorSize*0.95);

	MITK_WARN << "UpperPercentile" << upperPercentile << " lowerPercentile" << lowerPercentile;


	


	for (int i = lowerPercentile; i <= upperPercentile; i++) {
		filteredRadii.push_back(radii[i]);
	}

	MITK_WARN << "Count" << filteredRadii.size();


	
}

void MCTAnalyzer::PreprocessingWriter() {

	if (m_Controls.cB_SV->isChecked()){
		resultMap.insert(std::pair<std::string, double>("SV", 0.0));
	}
	if (m_Controls.cB_SR->isChecked()){
		resultMap.insert(std::pair<std::string, double>("SR", 0.0));
	}
	if (m_Controls.cB_P->isChecked()){
		resultMap.insert(std::pair<std::string, double>("P", 0.0));
	}
	if (m_Controls.cB_TMD_SV->isChecked()){
		resultMap.insert(std::pair<std::string, double>("mean(TMD_SV)", 0.0));
		resultMap.insert(std::pair<std::string, double>("sd(TMD_SV)", 0.0));
	}
	if (m_Controls.cB_TV->isChecked()) {
		resultMap.insert(std::pair<std::string, double>("TV", 0.0));
	}
	if (m_Controls.cB_SS->isChecked()) {
		resultMap.insert(std::pair<std::string, double>("SS", 0.0));
	}
	if (m_Controls.cB_PN->isChecked()) {
		resultMap.insert(std::pair<std::string, double>("PN", 0.0));
	}
	if (m_Controls.cB_PSR->isChecked()) {
		resultMap.insert(std::pair<std::string, double>("PSR_Min", 0.0));
		resultMap.insert(std::pair<std::string, double>("PSR_Max", 0.0));
	}
	if (m_Controls.cB_PV->isChecked()) {
		resultMap.insert(std::pair<std::string, double>("PV", 0.0));
	}
	if (m_Controls.cB_PS->isChecked()) {
		resultMap.insert(std::pair<std::string, double>("mean(PS)", 0.0));
		resultMap.insert(std::pair<std::string, double>("sd(PS)", 0.0));
	}
	if (m_Controls.cB_TMD_TV->isChecked()) {
		resultMap.insert(std::pair<std::string, double>("mean(TMD_TV)", 0.0));
		resultMap.insert(std::pair<std::string, double>("sd(TMD_TV)", 0.0));
	}

}

void MCTAnalyzer::WriteResults(QString name, QString sample){



	m_EndTime = std::chrono::system_clock::now();
	std::chrono::hours   hh = std::chrono::duration_cast<std::chrono::hours>(m_EndTime - m_StartTime);
	std::chrono::minutes mm = std::chrono::duration_cast<std::chrono::minutes>(m_EndTime - m_StartTime);
	std::chrono::seconds ss = std::chrono::duration_cast<std::chrono::seconds>(m_EndTime - m_StartTime);
	mm %= 60;
	ss %= 60;

	MITK_INFO << "Optimizing took " << hh.count() << "h, " << mm.count() << "m and " << ss.count() << "s";
	

	QString dir = m_Controls.outputDirectoryInput->text();
	QRegExp rx("(?:\\s*)(T|t)(\\d+)(?:\\s*)");
	int pos = rx.indexIn(name);     // returns -1 (no match)

	MITK_WARN << pos << " " << rx.cap(0);
	//QStringList list = rx.capturedTexts();
	QString fileName = dir + "/" + sample + "_" + rx.cap(0) + ".csv";


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
	
	

	for (int i = 0; i<allPButtons.count(); ++i)
	{
		MITK_WARN << "hhh";
		QString pixelQstr;
		pixelQstr.append("pixel");
		pixelQstr.append(QString::number(i));
		MITK_WARN << pixelQstr;
		QString lineEditQstr;
		lineEditQstr.append("lineEdit");
		lineEditQstr.append(QString::number(i));

		QString sampleQstr;
		sampleQstr.append("Sample");
		sampleQstr.append(QString::number(i));


		QLineEdit* lineEdit = m_Controls.widget_7->findChild<QLineEdit*>(pixelQstr);

		double pixelSize = lineEdit->text().toDouble();

		MITK_WARN << "PixelSize" << pixelSize;

		QString lineText = m_Controls.widget_7->findChild<QLineEdit*>(lineEditQstr)->text();

		QString sampleText = m_Controls.widget_7->findChild<QLineEdit*>(sampleQstr)->text();
		

		QStringList directoryPaths = lineText.split(";");
		MITK_WARN << "PathsCount" << directoryPaths.count();



		for (int m = 0; m < directoryPaths.count(); ++m)
		{

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

			std::vector<double> filteredRadii;

			QString path = directoryPaths.at(m);
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










			segmentedBinaryImage = ScaffoldSegmentation(mImage);
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

			
			if (m_Controls.cB_TV->isChecked() ||  m_Controls.cB_P->isChecked()
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
				MITK_WARN << "Hallo";

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

				FindPores(segmentedBinaryImage, pixelSize, lowerPercentile, upperPercentile, filteredRadii, maskCon, convexHullSurface);
				
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

				
				for (int i = 0; i <= filteredRadii.size(); i++) {
					PV += (4 / 3) * 3.14159265359 * std::pow(filteredRadii[i] / 2, 3.0);
				}

				MITK_WARN << "Total Pore Volume " << PV;
				resultMap["PV"] = PV;
			}

				mitk::RenderingManager::GetInstance()->RequestUpdateAll();

			}

			WriteResults(path, sampleText);

		}
	}
	}
}
