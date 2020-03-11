#include "mctAnalyzerCropFilter.h"
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>

#include "itkImage.h"
#include <itkMaskNegatedImageFilter.h>	
#include <iostream>
#include "mitkImageCast.h"
#include "itkMaskImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"




MctAnalyzerCropFilter::MctAnalyzerCropFilter() : m_mask(nullptr), convexHull(nullptr)
{
	
	


}

MctAnalyzerCropFilter::~MctAnalyzerCropFilter()
{

}

void MctAnalyzerCropFilter::SetMask(const mitk::Image::Pointer boundingObject)
{
	m_mask = boundingObject;
	MITK_WARN << "bluuuuuuuuuuuuuuuuub";
	// Process object is not const-correct so the const_cast is required here
	
	MITK_WARN << "bluuuuuuuuuuuuuuuuub4";
}

const mitk::Image::Pointer MctAnalyzerCropFilter::GetMask() const
{
	return m_mask;
}


void MctAnalyzerCropFilter::SetBoundingObject(const mitk::BoundingBox::Pointer boundingObject)
{
	convexHull = boundingObject;
	MITK_WARN << "bluuuuuuuuuuuuuuuuub";
	// Process object is not const-correct so the const_cast is required here

	MITK_WARN << "bluuuuuuuuuuuuuuuuub4";
}

const mitk::BoundingBox::Pointer MctAnalyzerCropFilter::GetBoundingObject() const
{
	return convexHull;
}

template<typename TPixel, unsigned int VImageDimension>
void MctAnalyzerCropFilter::SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage)
{
	typedef itk::Image<TPixel, VImageDimension>									ImageType;

	typedef itk::Image< float, ImageType::ImageDimension >				FloatImageType;

	ImageType::Pointer mask;
	mitk::CastToItkImage(m_mask, mask);

	/*typedef itk::MaskImageFilter< ImageType, FloatImageType > MaskFilterType2;
	MaskFilterType2::Pointer maskFilter2 = MaskFilterType2::New();
	maskFilter2->SetInput(inputImage);


	maskFilter2->SetMaskImage(mask);
	maskFilter2->Update();*/

  
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
	filter->SetInput(mask);
	filter->ReleaseDataFlagOn();

	typedef itk::MultiplyImageFilter <ImageType, ImageType >
		MultiplyImageFilterType;

	MultiplyImageFilterType::Pointer multiplyFilter
		= MultiplyImageFilterType::New();
	multiplyFilter->SetInput1(inputImage);
	multiplyFilter->SetInput2(filter->GetOutput());
	multiplyFilter->ReleaseDataFlagOn();

	mitk::GrabItkImageMemory(multiplyFilter->GetOutput(), outputImage);
  MITK_INFO << "filter pipeline was applied!";

}


void MctAnalyzerCropFilter::GenerateData() {
	mitk::Image::Pointer inputImage = this->GetInput();
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
