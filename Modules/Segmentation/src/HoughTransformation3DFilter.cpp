

#include "HoughTransformation3DFilter.h"



HoughTransformation3DFilter::HoughTransformation3DFilter(){
	initParams = HoughTransform3DInitParameters();
}

HoughTransformation3DFilter::~HoughTransformation3DFilter(){

}

void HoughTransformation3DFilter::InitFilter(HoughTransform3DInitParameters parameterList){
	this->initParams = parameterList;
}

HoughTransform3DStatistics HoughTransformation3DFilter::getStatistics(){
	return statistics;
}

std::vector<mitk::Surface::Pointer> HoughTransformation3DFilter::getSphereObjects(){
	std::vector<mitk::Surface::Pointer> spheres;

	int numSpheres = statistics.spheres.size();
	for (int i = 0; i < numSpheres; ++i){
		mitk::Point3D origin = statistics.spheres.at(i).origin;
		double radius = statistics.spheres.at(i).radius;

		vtkSmartPointer<vtkSphereSource> sphereSource =
			vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetCenter(origin[0], origin[1], origin[2]);
		sphereSource->SetRadius(radius);
		sphereSource->SetThetaResolution(50);
		sphereSource->SetPhiResolution(50);
		sphereSource->Update();

		mitk::Surface::Pointer sphereSurface = mitk::Surface::New();
		sphereSurface->SetVtkPolyData(sphereSource->GetOutput());

		spheres.push_back(sphereSurface);
	}

	return spheres;
}

void HoughTransformation3DFilter::GenerateData() {
	mitk::Image::Pointer inputImage = this->GetInput(0);
	mitk::Image::Pointer outputImage = this->GetOutput();

	geometry = inputImage->GetGeometry();

	try
	{
		AccessFixedDimensionByItk_n(inputImage, TrafoPipeline, 3, (outputImage));
	}
	catch (const mitk::AccessByItkException& e)
	{
		MITK_ERROR << "Unsupported pixel type or image dimension: " << e.what();
	}

}

template<typename TPixel, unsigned int VImageDimension>
void HoughTransformation3DFilter::TrafoPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage)
{
	// typedefs...
	typedef  float           OutputPixelType;

	typedef itk::Image<TPixel, VImageDimension>	ImageType;
	typedef itk::Image< OutputPixelType, ImageType::ImageDimension >    OutputImageType;

	ImageType::SpacingType spacing;

	spacing = inputImage->GetSpacing();

	// calc HoughTransform...
	typedef itk::HoughTransformRadialVotingImageFilter< ImageType,
		OutputImageType > HoughTransformFilterType;
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
	OutputImageType::Pointer localAccumulator = houghFilter->GetOutput();

	// get Spheres...
	HoughTransformFilterType::SpheresListType spheres;
	spheres = houghFilter->GetSpheres();

	typedef HoughTransformFilterType::SpheresListType SpheresListType;
	SpheresListType::const_iterator itSpheres = spheres.begin();

	unsigned int count = 1;
	while (itSpheres != spheres.end())
	{
		mitk::Point3D pIndex;
		pIndex[0] = (*itSpheres)->GetObjectToParentTransform()->GetOffset()[0];
		pIndex[1] = (*itSpheres)->GetObjectToParentTransform()->GetOffset()[1];
		pIndex[2] = (*itSpheres)->GetObjectToParentTransform()->GetOffset()[2];

		mitk::Point3D pWorld;
		geometry->IndexToWorld(pIndex, pWorld);

		double r = (*itSpheres)->GetRadius()[0];
		SphereStatistics stat; stat.origin = pWorld; stat.radius = r;
		statistics.spheres.push_back(stat);

		itSpheres++;
		count++;
	}

	// save accumulator image...
	statistics.accumulatorImage = mitk::ImportItkImage(localAccumulator);

	// save Output Image...
	mitk::GrabItkImageMemory(localAccumulator, outputImage);
	MITK_INFO << "filter pipeline was applied!";

}
