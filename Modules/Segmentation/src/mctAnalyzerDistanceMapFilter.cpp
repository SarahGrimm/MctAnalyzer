#include "mctAnalyzerDistanceMapFilter.h"
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>

#include "itkImage.h"

#include "itkImageRegionIterator.h"

#include "itkDanielssonDistanceMapImageFilter.h"
#include "itkSignedDanielssonDistanceMapImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkFastChamferDistanceImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"


#include <iostream>




MctAnalyzerDistanceMapFilter::MctAnalyzerDistanceMapFilter()
{

}

MctAnalyzerDistanceMapFilter::~MctAnalyzerDistanceMapFilter()
{

}

void MctAnalyzerDistanceMapFilter::SetBoundingObject(const mitk::BoundingBox::Pointer boundingObject)
{
	convexHull = boundingObject;
	
}

const mitk::BoundingBox::Pointer MctAnalyzerDistanceMapFilter::GetBoundingObject() const
{
	return convexHull;
}


template<typename TPixel, unsigned int VImageDimension>
void MctAnalyzerDistanceMapFilter::SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage)
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



	typedef  itk::DanielssonDistanceMapImageFilter< ImageType, OutputImageType  >  DanielssonDistanceMapImageFilterType;
	typedef  itk::SignedDanielssonDistanceMapImageFilter< ImageType, OutputImageType  > SignedDanielssonDistanceMapImageFilterType;
	typedef  itk::SignedMaurerDistanceMapImageFilter< ImageType, FloatImageType  > SignedMaurerDistanceMapImageFilterType;
	
	


  /*DanielssonDistanceMapImageFilterType::Pointer distanceMapImageFilter =
	  DanielssonDistanceMapImageFilterType::New();
  distanceMapImageFilter->SetInput(regionOfInterestFilter->GetOutput());
  distanceMapImageFilter->InputIsBinaryOn();*/

  //SignedDanielssonDistanceMapImageFilterType::Pointer distanceMapImageFilter =
	//  SignedDanielssonDistanceMapImageFilterType::New();
  //distanceMapImageFilter->SetInput(regionOfInterestFilter->GetOutput());
  //distanceMapImageFilter->SetInsideIsPositive(true);


	mitk::BoundingBox::PointType min = convexHull->GetMinimum();
	mitk::BoundingBox::PointType max = convexHull->GetMaximum();


	ImageType::IndexType start;
	start[0] = std::ceil(min[0]);
	start[1] = std::ceil(min[1]);
	start[2] = std::ceil(min[2]);

	ImageType::SizeType size;
	size[0] = std::ceil(max[0]) - start[0];
	size[1] = std::ceil(max[1]) - start[1];
	size[2] = std::ceil(max[2]) - start[2];

	ImageType::RegionType desiredRegion;
	desiredRegion.SetSize(size);
	desiredRegion.SetIndex(start);

	typedef itk::RegionOfInterestImageFilter< ImageType, ImageType > FilterType;
	FilterType::Pointer filter = FilterType::New();

	filter->SetRegionOfInterest(desiredRegion);
	filter->SetInput(inputImage);
	filter->ReleaseDataFlagOn();

  SignedMaurerDistanceMapImageFilterType::Pointer distanceMapImageFilter =
	  SignedMaurerDistanceMapImageFilterType::New();
  distanceMapImageFilter->SetInput(filter->GetOutput());

  distanceMapImageFilter->ReleaseDataFlagOn();
  
  

  mitk::GrabItkImageMemory(distanceMapImageFilter->GetOutput(), outputImage);
  MITK_INFO << "filter pipeline was applied!";

}


void MctAnalyzerDistanceMapFilter::GenerateData() {
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
