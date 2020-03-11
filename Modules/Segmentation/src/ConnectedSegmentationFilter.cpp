#include "ConnectedSegmentationFilter.h"
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>
#include "itkMedianImageFilter.h"
#include "mitkProgressBar.h"

ConnectedSegmentationFilter::ConnectedSegmentationFilter()
{
	m_LowerThreshold = 350;
	m_UpperThreshold = 3071;
	m_UseMedian = true;
	m_ConnectedObjects = 1;
}


ConnectedSegmentationFilter::~ConnectedSegmentationFilter()
{
}


template<typename TPixel, unsigned int VImageDimension>
void ConnectedSegmentationFilter::SegmentationPipeline(itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage)
{
	typedef itk::Image<TPixel, VImageDimension> ImageType;
	typedef itk::Image< unsigned int, ImageType::ImageDimension > IntImageType;
	typedef IntImageType OutputImageType;

	typedef itk::MedianImageFilter<ImageType, ImageType> MedianFilterType;
	typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> BinaryThresholdFilterType;
	typedef itk::ConnectedComponentImageFilter<ImageType, OutputImageType> ConnectednessFilterType;
	typedef itk::RelabelComponentImageFilter<IntImageType, ImageType> RelabelFilterType;

	unsigned int steps = 5;

	if (!m_UseMedian){
		steps--;
	}

	mitk::ProgressBar::GetInstance()->AddStepsToDo(steps);

	typename BinaryThresholdFilterType::Pointer binary = BinaryThresholdFilterType::New();

	typename MedianFilterType::Pointer median;
	if (m_UseMedian){
		/* median filter */
		median = MedianFilterType::New();
		median->SetInput(inputImage);
		MedianFilterType::InputSizeType radius;
		radius.Fill(1);
		median->SetRadius(radius);
		median->Update();
		MITK_DEBUG << "median filter with" << radius << "was applied...";
		mitk::ProgressBar::GetInstance()->Progress();
		binary->SetInput(median->GetOutput());
	}
	else{
		binary->SetInput(inputImage);
	}
	
	/* threshold */
	binary->SetLowerThreshold(m_LowerThreshold);
	binary->SetUpperThreshold(m_UpperThreshold);
	binary->ReleaseDataFlagOn();
	binary->Update();
	MITK_DEBUG << "binary threshold with lower value '" << m_LowerThreshold << "' and upper value '" << m_UpperThreshold << "' was applied...";
	mitk::ProgressBar::GetInstance()->Progress();

	/* connected component filter to label the objects */
	typename ConnectednessFilterType::Pointer connectedComponent = ConnectednessFilterType::New();
	connectedComponent->SetInput(binary->GetOutput());
	connectedComponent->SetFullyConnected(true);
	connectedComponent->ReleaseDataFlagOn();
	connectedComponent->Update();
	MITK_DEBUG << "connected component filter was applied...";
	mitk::ProgressBar::GetInstance()->Progress();

	/* relabel component filter */
	typename RelabelFilterType::Pointer relabelFilter = RelabelFilterType::New();
	relabelFilter->SetInput(connectedComponent->GetOutput());
	relabelFilter->Update();
	MITK_DEBUG << "relabel component filter was applied...";
	mitk::ProgressBar::GetInstance()->Progress();

	/* get the largest object */
	typename BinaryThresholdFilterType::Pointer regionOfInterestFilter = BinaryThresholdFilterType::New();
	regionOfInterestFilter->SetInput(relabelFilter->GetOutput());
	regionOfInterestFilter->SetLowerThreshold(1);
	regionOfInterestFilter->SetUpperThreshold(m_ConnectedObjects);
	regionOfInterestFilter->SetInsideValue(1);
	regionOfInterestFilter->ReleaseDataFlagOn();
	regionOfInterestFilter->SetOutsideValue(0);
	regionOfInterestFilter->Update();
	MITK_DEBUG << "region of interest is maintained...";
	mitk::ProgressBar::GetInstance()->Progress();

	mitk::GrabItkImageMemory(regionOfInterestFilter->GetOutput(), outputImage);
	MITK_DEBUG << "filter pipeline was applied!";
}


void ConnectedSegmentationFilter::GenerateData(){
	mitk::Image::Pointer inputImage = this->GetInput(0);
	mitk::Image::Pointer outputImage = this->GetOutput();

	if (m_ConnectedObjects < 1){
		MITK_ERROR << "Number of connected objects can't be less than 1";
		return;
	}

	if (m_LowerThreshold != NULL && m_UpperThreshold != NULL && m_UpperThreshold > m_LowerThreshold){
		try
		{
			AccessByItk_n(inputImage, SegmentationPipeline, (outputImage));
		}
		catch (const mitk::AccessByItkException& e)
		{
			MITK_ERROR << "Unsupported pixel type or image dimension: " << e.what();
		}
	}
	else{
		MITK_ERROR << "lower or upper threshold was null";
	}

}

