#include "mctAnalyzerBinarySegmentationFilter.h"
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>
#include "itkMedianImageFilter.h"
#include "mitkImageToSurfaceFilter.h"
#include "itkImage.h"
#include <itkShapeLabelObject.h>
#include "itkConnectedThresholdImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryThinningImageFilter3D.h"
#include "itkHoughTransformRadialVotingImageFilter.h"
#include "itkDanielssonDistanceMapImageFilter.h"
#include "itkSignedDanielssonDistanceMapImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkFastChamferDistanceImageFilter.h"
#include "itkRegionalMaximaImageFilter.h"
#include "itkValuedRegionalMaximaImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkBinaryContourImageFilter.h"		
#include <iostream>




MctAnalyzerBinarySegmentationFilter::MctAnalyzerBinarySegmentationFilter()
{

}

MctAnalyzerBinarySegmentationFilter::~MctAnalyzerBinarySegmentationFilter()
{

}


template<typename TPixel, unsigned int VImageDimension>
void MctAnalyzerBinarySegmentationFilter::SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage)
{




	MITK_WARN << VImageDimension;
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
	typedef itk::ShapeLabelObject< LabelType, VImageDimension > ShapeLabelObjectType;
	typedef itk::LabelMap< ShapeLabelObjectType >         LabelMapType;
	typedef itk::Image< InternalPixelType, VImageDimension >    InternalImageType;


	typedef itk::BinaryThresholdImageFilter<ImageType, ImageType>				BinaryThresholdFilterType;
	
	typedef itk::BinaryContourImageFilter <ImageType, ImageType >
		binaryContourImageFilterType;
	typedef itk::ConnectedComponentImageFilter<ImageType, OutputImageType>		ConnectednessFilterType;
	typedef itk::RelabelComponentImageFilter<IntImageType, ImageType>			RelabelFilterType;
	typedef itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType3>	RegionGrowerFilterType;
	typedef itk::LabelImageToShapeLabelMapFilter<OutputImageType2, LabelMapType > I2LType;
	typedef itk::BinaryThinningImageFilter3D< ImageType, ImageType > ThinningFilterType;
	typedef itk::HoughTransformRadialVotingImageFilter< ImageType, InternalImageType > HoughTransformFilterType;
	
	
	
	/*binaryContourImageFilterType::Pointer binaryContourFilter
		= binaryContourImageFilterType::New();
	binaryContourFilter->SetInput(inputImage);
	binaryContourFilter->SetForegroundValue(1);
	binaryContourFilter->SetBackgroundValue(0);
	//binaryContourFilter->SetFullyConnected(true);
	binaryContourFilter->Update();*/

	typename ConnectednessFilterType::Pointer connectedComponent = ConnectednessFilterType::New();
	connectedComponent->SetInput(inputImage);
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
  
  

	mitk::GrabItkImageMemory(connectedComponent->GetOutput(), outputImage);
  MITK_INFO << "filter pipeline was applied!";

}


void MctAnalyzerBinarySegmentationFilter::GenerateData() {
	mitk::Image::Pointer inputImage = this->GetInput(0);
	mitk::Image::Pointer outputImage = this->GetOutput();

	try
	{
		AccessFixedDimensionByItk_n(inputImage, SegmentationPipeline,2, (outputImage));
	}
	catch (const mitk::AccessByItkException& e)
	{
		MITK_ERROR << "Unsupported pixel type or image dimension: " << e.what();
	}

}
