#include "mctAnalyzerSegmentationFilter.h"
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>

#include "itkImage.h"

#include "itkConnectedThresholdImageFilter.h"
#include "itkImageRegionIterator.h"

#include "itkMedianImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkFCMClassifierInitializationImageFilter.h"
#include "itkFuzzyClassifierImageFilter.h"
#include "itkKFCMSClassifierInitializationImageFilter.h"
#include "itkFuzzyClassifierImageFilter.h"
#include "itkMSKFCMClassifierInitializationImageFilter.h"
#include <itkMaskNegatedImageFilter.h>
#include <iostream>





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
	typedef itk::Image< float, ImageType::ImageDimension >				FloatImageType;
	
	typedef itk::Image<unsigned char, ImageType::ImageDimension>  CharType;
	
	typedef unsigned short                                LabelType;
	typedef  unsigned char   OutputPixelType;
	typedef  float           InternalPixelType;
	InputImageType::IndexType localIndex;

	typedef typename IntImageType::PixelType									InputImagePixelType;
	typedef itk::Image< LabelType, VImageDimension >            OutputImageType2;
	typedef itk::Image< OutputPixelType, VImageDimension >  OutputImageType3;

	typedef itk::Image< InternalPixelType, VImageDimension >    InternalImageType;

	typedef itk::OtsuThresholdImageFilter<ImageType, ImageType>					OtsuThresholdFilterType;
	typedef itk::BinaryThresholdImageFilter<ImageType, ImageType>				BinaryThresholdFilterType;

	

	typedef itk::MedianImageFilter<ImageType, ImageType> MedianFilterType;
	typedef itk::ConnectedComponentImageFilter<ImageType, OutputImageType>		ConnectednessFilterType;
	typedef itk::RelabelComponentImageFilter<IntImageType, ImageType>			RelabelFilterType;
	typedef itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType3>	RegionGrowerFilterType;
	
	/*typename MedianFilterType::Pointer median;
	median = MedianFilterType::New();
	median->SetInput(inputImage);
	MedianFilterType::InputSizeType radius;
	radius.Fill(1);
	median->SetRadius(radius);
	median->Update();*/
	//MITK_DEBUG << "median filter with" << radius << "was applied...";
	//mitk::ProgressBar::GetInstance()->Progress();
	//binary->SetInput(median->GetOutput());
	


	// otsu filter
	/*typedef itk::MedianImageFilter<ImageType, ImageType > FilterType;
	FilterType::Pointer medianFilter = FilterType::New();
	FilterType::InputSizeType radius;
	medianFilter->SetInput(inputImage);
	medianFilter->SetRadius(2);
	medianFilter->Update();*/

  typename OtsuThresholdFilterType::Pointer otsu = OtsuThresholdFilterType::New();
  //otsu->SetInput(median->GetOutput());
  otsu->SetInput(inputImage);
  otsu->SetInsideValue(1);
  otsu->SetOutsideValue(0);
  otsu->ReleaseDataFlagOn();
  otsu->Update();
  std::cout << "otsu filter was applied..." << otsu->GetThreshold() << std::endl;


  //connected component filter
  typename ConnectednessFilterType::Pointer connectedComponent = ConnectednessFilterType::New();
  connectedComponent->SetInput(otsu->GetOutput());
  connectedComponent->SetFullyConnected(true);
  connectedComponent->ReleaseDataFlagOn();
  connectedComponent->Update();
 
  std::cout << "connected component filter was applied..." << std::endl;
  std::cout << "number of objects: " << connectedComponent->GetObjectCount() << std::endl;

  
  // relabel component filter 
  typename RelabelFilterType::Pointer relabelFilter = RelabelFilterType::New();
  relabelFilter->SetInput(connectedComponent->GetOutput());
  relabelFilter->ReleaseDataFlagOn();
  relabelFilter->Update();
  std::cout << "relabel component filter was applied..." << std::endl;
  std::cout << "relabel component filter was applied..." << relabelFilter->GetSizeOfObjectsInPixels()[0] << std::endl;

  // maintain region of interest (1) and mark subregions (0) as background
  typename BinaryThresholdFilterType::Pointer regionOfInterestFilter = BinaryThresholdFilterType::New();
  regionOfInterestFilter->SetInput(relabelFilter->GetOutput());
  regionOfInterestFilter->SetLowerThreshold(0);
  regionOfInterestFilter->SetUpperThreshold(0);
  regionOfInterestFilter->SetInsideValue(1);
  regionOfInterestFilter->ReleaseDataFlagOn();
  regionOfInterestFilter->SetOutsideValue(0);
  regionOfInterestFilter->Update();
  std::cout << "region of interest is maintained..." << std::endl;


  
  

  mitk::GrabItkImageMemory(regionOfInterestFilter->GetOutput(), outputImage);
  MITK_INFO << "filter pipeline was applied!";

}


void MctSegmentationFilter::GenerateData() {
	mitk::Image::Pointer inputImage = this->GetInput(0);
	mitk::Image::Pointer outputImage = this->GetOutput();

	try
	{
		AccessFixedDimensionByItk_n(inputImage, SegmentationPipeline,3, (outputImage));
	}
	catch (const mitk::AccessByItkException& e)
	{
		MITK_ERROR << "Unsupported pixel type or image dimension: " << e.what();
	}

}
