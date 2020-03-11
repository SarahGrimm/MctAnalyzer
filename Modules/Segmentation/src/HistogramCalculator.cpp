#include "HistogramCalculator.h"
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>
#include "itkMedianImageFilter.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkProgressBar.h"
#include <itkScalarImageToHistogramGenerator.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkOtsuMultipleThresholdsCalculator.h>
#include "itkNumericTraits.h"
#include "math.h"


HistogramCalculator::HistogramCalculator()
{
	m_AmountThresholds = 0;
	m_UseMedian = true;
}


HistogramCalculator::~HistogramCalculator()
{
}


template < typename TPixel, unsigned int VImageDimension>
void HistogramCalculator::CalculateThresholds(itk::Image< TPixel, VImageDimension >* itkImage, mitk::Image::Pointer output){

	typedef itk::Image< TPixel, VImageDimension >									InputImageType;
	typedef itk::Statistics::ScalarImageToHistogramGenerator<InputImageType>		HistogramGeneratorType;
	typedef HistogramGeneratorType::HistogramType									HistogramType;
	typedef itk::OtsuMultipleThresholdsCalculator< HistogramType >					CalculatorType;
	typedef itk::MedianImageFilter<InputImageType, InputImageType>					MedianFilterType;
	typedef itk::RescaleIntensityImageFilter<InputImageType, InputImageType>		RescaleTypeOut;

	unsigned short steps = 4;
	if (!m_UseMedian){
		steps -= 1;
	}

	mitk::ProgressBar::GetInstance()->AddStepsToDo(steps);

	InputImageType::Pointer input;
	if (m_UseMedian){
		typename MedianFilterType::Pointer median = MedianFilterType::New();
		median->SetInput(itkImage);
		MedianFilterType::InputSizeType radius;
		radius.Fill(1);
		median->SetRadius(radius);
		median->Update();
		MITK_INFO << "median filter with" << radius << "was applied...";
		mitk::GrabItkImageMemory(median->GetOutput(), output);
		input = median->GetOutput();
		mitk::ProgressBar::GetInstance()->Progress();
	}
	else{
		input = itkImage;
	}

	HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();

	histogramGenerator->SetInput(input);
	histogramGenerator->SetNumberOfBins(128);
	histogramGenerator->Compute();

	mitk::ProgressBar::GetInstance()->Progress();

	const HistogramType * histogram = histogramGenerator->GetOutput();


	typename CalculatorType::Pointer otsuCalculator = CalculatorType::New();

	int numberOfThresholds;
	if (m_ImageModality == ImageModalityType::CBCT){
		numberOfThresholds = 4;
	}
	else if (m_ImageModality == ImageModalityType::CT){
		numberOfThresholds = 4;
	}

	if (m_AmountThresholds > 0){
		numberOfThresholds = m_AmountThresholds;
	}

	otsuCalculator->SetInputHistogram(histogram);
	otsuCalculator->SetNumberOfThresholds(numberOfThresholds);
	otsuCalculator->SetValleyEmphasis(false);
	otsuCalculator->Compute();

	MITK_DEBUG << "otsu finsihed!";

	mitk::ProgressBar::GetInstance()->Progress();

	const CalculatorType::OutputType &thresholdVector = otsuCalculator->GetOutput();

	typedef CalculatorType::OutputType::const_iterator ThresholdItType;

	m_LowerThreshold = 300;

	unsigned int digits = 2;

	typedef int IntType;
	typedef itk::Array<IntType>	IntArrayType;
	IntArrayType *thresholds = new IntArrayType(numberOfThresholds);

	int i = 0;
	for (ThresholdItType itNum = thresholdVector.begin(); itNum != thresholdVector.end(); ++itNum)
	{
		int currentThreshold = std::round(static_cast<itk::NumericTraits<CalculatorType::MeasurementType>::PrintType>(*itNum));
		thresholds->SetElement(i, currentThreshold);

		MITK_INFO << "OtsuThreshold["
			<< (int)(itNum - thresholdVector.begin())
			<< "] = "
			<< currentThreshold;
		i++;
	}
	if (m_SegmentationType == SegmentationThreshold::Bone){
		CalculateBoneThreshold(thresholds);
	}
	else if (m_SegmentationType == SegmentationThreshold::SoftTissue){
		CalculateSkinThreshold(thresholds);
	}

	MITK_INFO << "result: [" << m_LowerThreshold << " | " << m_UpperThreshold << "]";
	mitk::ProgressBar::GetInstance()->Progress();
}

void HistogramCalculator::GenerateData(){
	mitk::Image::Pointer inputImage = this->GetInput();
	mitk::Image::Pointer outputImage = this->GetOutput();
	if (inputImage){
		AccessByItk_1(inputImage, CalculateThresholds, outputImage);
	}
	else{
		MITK_ERROR << "input image for threshold calculation was null";
	}
}


void HistogramCalculator::CalculateBoneThreshold(itk::Array<int> *thresholds){
	int lower = 0;
	int upper = 0;
	if (m_ImageModality == ImageModalityType::CBCT){
		lower = 500; upper = 800;
	}
	else if (m_ImageModality == ImageModalityType::CT){
		lower = 150; upper = 450;
	}
	int currentThreshold = (upper + lower) / 2;
	

	for (int i = 0; i < thresholds->GetSize(); i++){
		currentThreshold = thresholds->GetElement(i);
		if (currentThreshold > lower && currentThreshold < upper){
			m_LowerThreshold = currentThreshold;
		}
	}
	m_UpperThreshold = 3071;
}

void HistogramCalculator::CalculateSkinThreshold(itk::Array<int> *thresholds){
	int lowerMin = 0;
	int lowerMax = 0;
	int upperMin = 0;
	int upperMax = 0;

	if (m_ImageModality == ImageModalityType::CT){
		lowerMin = -600; lowerMax = -300; upperMin = 150; upperMax = 450;
	}

	int currentThreshold = 300;

	for (int i = 0; i < thresholds->GetSize(); i++){
		currentThreshold = thresholds->GetElement(i);
		if (currentThreshold > lowerMin && currentThreshold < lowerMax){
			m_LowerThreshold = currentThreshold;
		}
		else if (currentThreshold > upperMin && currentThreshold < upperMax){
			m_UpperThreshold = currentThreshold;
		}
	}

}