

#include "HoughTransformation2DFilter.h"



HoughTransformation2DFilter::HoughTransformation2DFilter(){
	initParams = HoughTransform2DInitParameters();
}

HoughTransformation2DFilter::~HoughTransformation2DFilter(){

}

void HoughTransformation2DFilter::InitFilter(HoughTransform2DInitParameters parameterList){
	this->initParams = parameterList;
}

HoughTransform2DStatistics HoughTransformation2DFilter::getStatistics(){
	return statistics;
}

std::vector<mitk::Surface::Pointer> HoughTransformation2DFilter::getCircleObjects(){
	std::vector<mitk::Surface::Pointer> circles;

	int numSpheres = statistics.circles.size();
	for (int i = 0; i < numSpheres; ++i){
		mitk::Point2D origin = statistics.circles.at(i).origin;
		double radius = statistics.circles.at(i).radius;

		vtkSmartPointer<vtkRegularPolygonSource> polygonSource =
			vtkSmartPointer<vtkRegularPolygonSource>::New();

		polygonSource->GeneratePolygonOff();
		polygonSource->SetNumberOfSides(50);
		polygonSource->SetRadius(radius);
		polygonSource->SetCenter(origin[0], origin[1], 0);
		polygonSource->Update();

		mitk::Surface::Pointer sphereSurface = mitk::Surface::New();
		sphereSurface->SetVtkPolyData(polygonSource->GetOutput());

		circles.push_back(sphereSurface);
	}

	return circles;
}

void HoughTransformation2DFilter::GenerateData() {
	mitk::Image::Pointer inputImage = this->GetInput(0);
	mitk::Image::Pointer outputImage = this->GetOutput();

	geometry = inputImage->GetGeometry();

	try
	{
		AccessFixedDimensionByItk_n(inputImage, TrafoPipeline, 2, (outputImage));
	}
	catch (const mitk::AccessByItkException& e)
	{
		MITK_ERROR << "Unsupported pixel type or image dimension: " << e.what();
	}

}

template<typename TPixel, unsigned int VImageDimension>
void HoughTransformation2DFilter::TrafoPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage)
{
	// typedefs...
	typedef  float           InternalPixelType;
	typedef  unsigned char   OutputPixelType;

	typedef itk::Image<TPixel, VImageDimension>	ImageType;
	typedef itk::Image< InternalPixelType, ImageType::ImageDimension >    InternalImageType;
	typedef itk::Image< OutputPixelType, ImageType::ImageDimension >  OutputImageType;

	ImageType::IndexType localIndex;
	ImageType::SpacingType spacing;

	spacing = inputImage->GetSpacing();

	// calc HoughTransform...
	typedef itk::HoughTransformRadialVotingImageFilter< ImageType,
		InternalImageType > HoughTransformFilterType;
	HoughTransformFilterType::Pointer houghFilter = HoughTransformFilterType::New();
	houghFilter->SetInput(inputImage);
	houghFilter->SetNumberOfSpheres(initParams.numSpheres);
	houghFilter->SetMinimumRadius(initParams.minRadius);
	houghFilter->SetMaximumRadius(initParams.maxRadius);
	houghFilter->SetSigmaGradient(initParams.sigmaGradient);
	houghFilter->SetVariance(initParams.variance);
	houghFilter->SetSphereRadiusRatio(initParams.sphereRadiusRatio);
	houghFilter->SetVotingRadiusRatio(initParams.votingRadiusRatio);
	houghFilter->SetThreshold(initParams.threshold);
	houghFilter->SetOutputThreshold(initParams.outputThreshold);
	houghFilter->SetGradientThreshold(initParams.gradientThreshold);
	houghFilter->SetNbOfThreads(initParams.NbOfThreads);
	houghFilter->SetSamplingRatio(initParams.samplingRatio);
	houghFilter->Update();

	// Accumulator Image...
	//InternalImageType::Pointer localAccumulator = houghFilter->GetOutput();

	// get Circles...
	HoughTransformFilterType::SpheresListType circles;
	circles = houghFilter->GetSpheres();

	// Computing circles output...
	OutputImageType::Pointer  localOutputImage = OutputImageType::New();

	OutputImageType::RegionType region;
	region.SetSize(inputImage->GetLargestPossibleRegion().GetSize());
	region.SetIndex(inputImage->GetLargestPossibleRegion().GetIndex());
	localOutputImage->SetRegions(region);
	localOutputImage->SetOrigin(inputImage->GetOrigin());
	localOutputImage->SetSpacing(inputImage->GetSpacing());
	localOutputImage->Allocate();
	localOutputImage->FillBuffer(0);

	typedef HoughTransformFilterType::SpheresListType SpheresListType;
	SpheresListType::const_iterator itSpheres = circles.begin();

	unsigned int count = 1;
	while (itSpheres != circles.end())
	{	
		mitk::Point3D pIndex;
		pIndex[0] = (*itSpheres)->GetObjectToParentTransform()->GetOffset()[0];
		pIndex[1] = (*itSpheres)->GetObjectToParentTransform()->GetOffset()[1];
		pIndex[2] = 0;

		mitk::Point3D pWorld;
		geometry->IndexToWorld(pIndex, pWorld);

		double r = (*itSpheres)->GetRadius()[0];
		CircleStatistics stat; stat.origin[0] = pWorld[0]; stat.origin[1] = pWorld[1]; stat.radius = r;
		statistics.circles.push_back(stat);

		for (double angle = 0; angle <= 2 * vnl_math::pi; angle += vnl_math::pi / 60.0)
		{
			localIndex[0] =
				(long int)((*itSpheres)->GetObjectToParentTransform()->GetOffset()[0]
				+ ((*itSpheres)->GetRadius()[0] * vcl_cos(angle)) / spacing[0]);
			localIndex[1] =
				(long int)((*itSpheres)->GetObjectToParentTransform()->GetOffset()[1]
				+ ((*itSpheres)->GetRadius()[1] * vcl_sin(angle)) / spacing[1]);
			OutputImageType::RegionType outputRegion =
				localOutputImage->GetLargestPossibleRegion();

			if (outputRegion.IsInside(localIndex))
			{
				localOutputImage->SetPixel(localIndex, count);
			}
		}
		itSpheres++;
		count++;
	}

	int radius = 2;
	typedef itk::BinaryBallStructuringElement< OutputPixelType, ImageType::ImageDimension >
		SEType;
	SEType sE;
	sE.SetRadius(radius);
	sE.CreateStructuringElement();

	typedef itk::GrayscaleDilateImageFilter< OutputImageType, OutputImageType, SEType >
		DilateFilterType;
	DilateFilterType::Pointer grayscaleDilate = DilateFilterType::New();
	grayscaleDilate->SetKernel(sE);
	grayscaleDilate->SetInput(localOutputImage);
	grayscaleDilate->Update();

	typedef itk::RGBPixel< unsigned char >   RGBPixelType;
	typedef itk::Image< RGBPixelType, ImageType::ImageDimension >  RGBImageType;
	typedef itk::LabelOverlayImageFilter< ImageType, OutputImageType, RGBImageType > OverlayType;
	OverlayType::Pointer overlay = OverlayType::New();
	overlay->SetInput(inputImage);
	overlay->SetLabelImage(grayscaleDilate->GetOutput());
	overlay->Update();

	statistics.circleImage = mitk::ImportItkImage(overlay->GetOutput());

	// save Output Image...
	mitk::GrabItkImageMemory(overlay->GetOutput(), outputImage);
	MITK_INFO << "filter pipeline was applied!";

}