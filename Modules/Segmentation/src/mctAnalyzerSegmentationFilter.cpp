#include "mctAnalyzerSegmentationFilter.h"
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>
#include "itkMedianImageFilter.h"
#include "mitkImageToSurfaceFilter.h"





MctSegmentationFilter::MctSegmentationFilter()
{

}

MctSegmentationFilter::~MctSegmentationFilter()
{

}


template<typename TPixel, unsigned int VImageDimension>
void MctSegmentationFilter::SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage)
{
  // create a filter pipeline
	typedef itk::Image<TPixel, VImageDimension>									ImageType;
	typedef itk::Image< unsigned int, ImageType::ImageDimension >				IntImageType;
	typedef IntImageType														OutputImageType;

	typedef typename IntImageType::PixelType									InputImagePixelType;
  

	typedef itk::OtsuThresholdImageFilter<ImageType, ImageType>						OtsuThresholdFilterType;
	typedef itk::BinaryThresholdImageFilter<ImageType, ImageType>				BinaryThresholdFilterType;
	typedef itk::ConnectedComponentImageFilter<ImageType, OutputImageType>		ConnectednessFilterType;
	typedef itk::RelabelComponentImageFilter<IntImageType, ImageType>			RelabelFilterType;
	typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType>	RegionGrowerFilterType;

  // otsu filter
  typename OtsuThresholdFilterType::Pointer otsu = OtsuThresholdFilterType::New();
  otsu->SetInput(inputImage);
  otsu->SetInsideValue(0);
  otsu->SetOutsideValue(1);
  otsu->Update();
  std::cout << "otsu filter was applied..." << std::endl;

  //connected component filter
  typename ConnectednessFilterType::Pointer connectedComponent = ConnectednessFilterType::New();
  connectedComponent->SetInput(otsu->GetOutput());
  connectedComponent->SetFullyConnected(true);
  connectedComponent->Update();
  std::cout << "connected component filter was applied..." << std::endl;
  std::cout << "number of objects: " << connectedComponent->GetObjectCount() << std::endl;

  // relabel component filter
  typename RelabelFilterType::Pointer relabelFilter = RelabelFilterType::New();
  relabelFilter->SetInput(connectedComponent->GetOutput());
  relabelFilter->Update();
  std::cout << "relabel component filter was applied..." << std::endl;

  // maintain region of interest (1) and mark subregions (0) as background
  typename BinaryThresholdFilterType::Pointer regionOfInterestFilter = BinaryThresholdFilterType::New();
  regionOfInterestFilter->SetInput(relabelFilter->GetOutput());
  regionOfInterestFilter->SetLowerThreshold(1);
  regionOfInterestFilter->SetUpperThreshold(1);
  regionOfInterestFilter->Update();
  std::cout << "region of interest is maintained..." << std::endl;

  

  // set output
  mitk::GrabItkImageMemory(regionOfInterestFilter->GetOutput(), outputImage);
  MITK_INFO << "filter pipeline was applied!";

}


void MctSegmentationFilter::GenerateData(){
	mitk::Image::Pointer inputImage = this->GetInput(0);
	mitk::Image::Pointer outputImage = this->GetOutput();

	try
	{
		AccessByItk_n(inputImage, SegmentationPipeline, (outputImage));
	}
	catch (const mitk::AccessByItkException& e)
	{
		MITK_ERROR << "Unsupported pixel type or image dimension: " << e.what();
	}

}
