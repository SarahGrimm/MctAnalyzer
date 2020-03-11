
#define _USE_MATH_DEFINES

#include <math.h>

#include "HoughTransformation.h"

// TEST
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <itkTIFFImageIO.h>
// TEST

HoughTransformation::HoughTransformation(){

}

HoughTransformation::~HoughTransformation(){

}

void HoughTransformation::setInputImage(mitk::Image::Pointer inputImage){
	this->inputImage = inputImage;
}

void HoughTransformation::calcHoughTransform3D(){

	clock_t beginT, endT;
	beginT = clock();

	const    unsigned int    Dimension = 2;
	typedef  unsigned char   InputPixelType;
	typedef  float           InternalPixelType;
	typedef  unsigned char   OutputPixelType;

	typedef itk::Image< InternalPixelType, Dimension >  InputImageType;
	typedef itk::Image< InternalPixelType, Dimension >    InternalImageType;
	typedef itk::Image< OutputPixelType, Dimension >  OutputImageType;

	InputImageType::IndexType localIndex;
	InputImageType::SpacingType spacing;

	// Cast InputImage
	InputImageType::Pointer localImage;
	mitk::CastToItkImage(inputImage, localImage);
	spacing = localImage->GetSpacing();


	typedef itk::HoughTransformRadialVotingImageFilter< InputImageType,
		InternalImageType > HoughTransformFilterType;
	HoughTransformFilterType::Pointer houghFilter = HoughTransformFilterType::New();
	houghFilter->SetInput(localImage);
	houghFilter->SetNumberOfSpheres(50);
	houghFilter->SetMinimumRadius(6);
	houghFilter->SetMaximumRadius(7.5);
	houghFilter->SetSigmaGradient(1);
	houghFilter->SetVariance(1);
	houghFilter->SetSphereRadiusRatio(0.5);
	houghFilter->SetVotingRadiusRatio(0.15);
	houghFilter->SetThreshold(100);
	houghFilter->SetOutputThreshold(0.5);
	houghFilter->SetGradientThreshold(1);
	houghFilter->SetNbOfThreads(1);
	houghFilter->SetSamplingRatio(1);
	houghFilter->Update();


	InternalImageType::Pointer localAccumulator = houghFilter->GetOutput();

	HoughTransformFilterType::SpheresListType circles;
	circles = houghFilter->GetSpheres();

	endT = clock();
	std::cout << (static_cast< double >(endT - beginT) / static_cast< double >(CLOCKS_PER_SEC)) << std::endl;

	std::cout << "Found " << circles.size() << " circle(s)." << std::endl;


	// Computing the circles output
	OutputImageType::Pointer  localOutputImage = OutputImageType::New();

	OutputImageType::RegionType region;
	region.SetSize(localImage->GetLargestPossibleRegion().GetSize());
	region.SetIndex(localImage->GetLargestPossibleRegion().GetIndex());
	localOutputImage->SetRegions(region);
	localOutputImage->SetOrigin(localImage->GetOrigin());
	localOutputImage->SetSpacing(localImage->GetSpacing());
	localOutputImage->Allocate();
	localOutputImage->FillBuffer(0);

	typedef HoughTransformFilterType::SpheresListType SpheresListType;
	SpheresListType::const_iterator itSpheres = circles.begin();

	unsigned int count = 1;
	while (itSpheres != circles.end())
	{
		std::cout << "Center: ";
		std::cout << (*itSpheres)->GetObjectToParentTransform()->GetOffset()
			<< std::endl;
		std::cout << "Radius: " << (*itSpheres)->GetRadius()[0] << std::endl;

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
	typedef itk::BinaryBallStructuringElement< OutputPixelType, Dimension >
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
	typedef itk::Image< RGBPixelType, Dimension >  RGBImageType;
	typedef itk::LabelOverlayImageFilter< InputImageType, OutputImageType, RGBImageType > OverlayType;
	OverlayType::Pointer overlay = OverlayType::New();
	overlay->SetInput(localImage);
	overlay->SetLabelImage(grayscaleDilate->GetOutput());
	overlay->Update();

	typedef itk::ImageFileWriter< RGBImageType > RGBWriterType;
	RGBWriterType::Pointer rgbwriter = RGBWriterType::New();
	rgbwriter->SetInput(overlay->GetOutput());
	rgbwriter->SetFileName("C:/Users/Ortho/Downloads/Hough/code/output.png");
	rgbwriter->Update();

	try
	{
		rgbwriter->Update();
	}
	catch (itk::ExceptionObject & excep)
	{
		std::cerr << "Exception caught !" << std::endl;
		std::cerr << excep << std::endl;
	}

	typedef  itk::ImageFileWriter< InternalImageType  > InputWriterType;
	InputWriterType::Pointer writer2 = InputWriterType::New();
	writer2->SetFileName("C:/Users/Ortho/Downloads/Hough/code/accumulator.tif");
	writer2->SetInput(localAccumulator);

	try
	{
		writer2->Update();
	}
	catch (itk::ExceptionObject & excep)
	{
		std::cerr << "Exception caught !" << std::endl;
		std::cerr << excep << std::endl;
	}
	
}

