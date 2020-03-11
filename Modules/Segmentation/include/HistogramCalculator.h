/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef HistogramCalculator_h
#define HistogramCalculator_h

#include <mitkImageToImageFilter.h>
#include <MitkCTSegmentationExports.h>
#include <mitkBaseData.h>

#include <itkOtsuThresholdImageFilter.h>
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkMedianImageFilter.h"
#include <itkScalarImageToHistogramGenerator.h>

/**
 * \brief	The HistogrammCalculator is used to calculate the lower and upper threshold of a
 * 			given image.
 *
 * It works by setting a specific amount of thresholds which are calculated. The amount can be selected by SetAmountThresholds() (default is 0). 
 * As alternative you can directly choose the segmentation type by SetSegmentationType().
 * After that, the lower and upper thresholds are selected out of the computed ones in a manner which fits the segmentation type.
 * 
 * Use GetLowerThreshold() and GetUpperThreshold() after you called Update() on this filter to get the thresholds.
 * You can choose the image modality by SetImageModality() and the segmentation type which has a impact on the calculated thresholds.
 *
 * You may reuse the output of this filter if UseMedian() is true. You'll get the median processed input image. Otherwise you'll get an emtpy image.
 */
class MITKCTSEGMENTATION_EXPORT HistogramCalculator final : public mitk::ImageToImageFilter
{
public:
	enum ImageModalityType { CT, CBCT };
	enum SegmentationThreshold {SoftTissue, Bone};

	mitkClassMacro(HistogramCalculator, mitk::ImageToImageFilter);
	itkFactorylessNewMacro(Self);

	itkGetMacro(LowerThreshold, int);
	itkGetMacro(UpperThreshold, int);

	/**
	* Sets the amount of thresholds which should be calculated
	*
	* \param	_arg	by default 0. If you don't change this, it will be ignored and the segmentation type is used to choose the number of thresholds.
	*/
	itkSetMacro(AmountThresholds, int);
	itkSetMacro(ImageModality, ImageModalityType);
	itkSetMacro(SegmentationType, SegmentationThreshold);

	/**
	* Uses a 1x1 median filter.
	*
	* \param	_arg	by default true.
	*/
	itkSetMacro(UseMedian, bool);

private:
	HistogramCalculator();
	~HistogramCalculator();

	template < typename TPixel, unsigned int VImageDimension >
	void CalculateThresholds(itk::Image< TPixel, VImageDimension >* itkImage, mitk::Image::Pointer output);

	void CalculateBoneThreshold(itk::Array<int> *thresholds);

	void CalculateSkinThreshold(itk::Array<int> *thresholds);

	void GenerateData() override;

	int m_LowerThreshold;
	int m_UpperThreshold;
	int m_AmountThresholds;
	bool m_UseMedian;
	ImageModalityType m_ImageModality;
	SegmentationThreshold m_SegmentationType;
};

#endif