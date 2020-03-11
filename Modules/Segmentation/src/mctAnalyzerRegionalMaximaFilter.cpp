#include "mctAnalyzerRegionalMaximaFilter.h"
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>

#include "itkImage.h"

#include "itkImageRegionIterator.h"

#include "itkRegionalMaximaImageFilter.h"
#include "itkHConvexImageFilter.h"

#include "itkDiscreteGaussianImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include <iostream>




MctAnalyzerRegionalMaximaFilter::MctAnalyzerRegionalMaximaFilter() : convexHull(nullptr)
{
	pointSet = mitk::PointSet::New();
}

MctAnalyzerRegionalMaximaFilter::~MctAnalyzerRegionalMaximaFilter()
{

}

void MctAnalyzerRegionalMaximaFilter::SetBoundingObject(const mitk::BoundingBox::Pointer boundingObject)
{
	convexHull = boundingObject;
	
}

const mitk::BoundingBox::Pointer MctAnalyzerRegionalMaximaFilter::GetBoundingObject() const
{
	return convexHull;
}

const mitk::PointSet::Pointer MctAnalyzerRegionalMaximaFilter::GetPointSet() const
{
	return pointSet;
}

template<typename TPixel, unsigned int VImageDimension>
void MctAnalyzerRegionalMaximaFilter::SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage)
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



	typedef itk::RegionalMaximaImageFilter <FloatImageType, CharType >		RegionalMaximaImageFilter;


	
	typedef itk::DiscreteGaussianImageFilter<
		ImageType, FloatImageType >  GaussianFilterType;


	GaussianFilterType::Pointer gfilter = GaussianFilterType::New();
	gfilter->SetInput(inputImage);
	gfilter->SetVariance(0.6);
	//gfilter->SetUseImageSpacingOn();
	gfilter->SetMaximumKernelWidth(5);
	gfilter->ReleaseDataFlagOn();
	gfilter->Update();

	typedef itk::HConvexImageFilter < FloatImageType, CharType > ConvexType;
	ConvexType::Pointer convex = ConvexType::New();
	convex->SetInput(gfilter->GetOutput());
	convex->SetFullyConnected(true);
	convex->SetHeight(1);
	convex->ReleaseDataFlagOn();
	convex->Update();


	RegionalMaximaImageFilter::Pointer rfilter = RegionalMaximaImageFilter::New();
	rfilter->SetInput(gfilter->GetOutput());
	rfilter->SetFullyConnected(true);
	rfilter->ReleaseDataFlagOn();
	rfilter->Update();


	const CharType* maxFO = rfilter->GetOutput();

	typedef itk::ImageRegionIteratorWithIndex< CharType > ItkInputImageIteratorType;
	ItkInputImageIteratorType  inputIt(rfilter->GetOutput(), maxFO->GetLargestPossibleRegion());
	mitk::Point3D p;
	int counter = 0;

	

	for (inputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt)
	{

		if (inputIt.Value() != 0) {
			vtk2itk(inputIt.GetIndex(), p);
			pointSet->InsertPoint(p);
			counter++;
		}

	}

	mitk::GrabItkImageMemory(rfilter->GetOutput(), outputImage);
	  MITK_INFO << "filter pipeline was applied! " << counter;

}


void MctAnalyzerRegionalMaximaFilter::GenerateData() {
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
