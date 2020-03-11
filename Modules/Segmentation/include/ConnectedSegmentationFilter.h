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

#ifndef ConnectedSegmentationFilter_h
#define ConnectedSegmentationFilter_h

#include <mitkImageToImageFilter.h>
#include <MitkCTSegmentationExports.h>
#include <itkImageToImageFilter.h>

#include <itkOtsuThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include "itkDiscreteGaussianImageFilter.h"
#include "itkMedianImageFilter.h"
#include "mitkColorProperty.h"

/**
 * This class represents the connected segmentation pipeline.
 * 
 * First a median filter is applied to smooth the raw image and remove some noise or small
 * artefacts but preserves egdes. Then the resulting image will be filtered by a lower and upper
 * threshold which should be set by SetLowerThreshold() and SetUpperThreshold(). Afterwards the
 * the largest connected resulting binary segmentation is obtained (can represent the skull or the soft tissue).
 * 
 * \imageMacro{filter_pipeline.png, "Segmentation Filter Pipeline",10}
 *
 * \author	Tobias Stein
 */
class MITKCTSEGMENTATION_EXPORT ConnectedSegmentationFilter final : public mitk::ImageToImageFilter
{
public:

	mitkClassMacro(ConnectedSegmentationFilter, mitk::ImageToImageFilter);
	itkFactorylessNewMacro(Self);

	/**
	 * Sets the lower threshold, under which the grey values are not maintained.
	 *
	 * \param	_arg	by default 400.
	 */
	itkSetMacro(LowerThreshold, double);

	/**
	 * Sets the upper threshold, over which the grey values are not maintained.
	 *
	 * \param	_arg	by default 3000.
	 */
	itkSetMacro(UpperThreshold, double);

	/**
	 * If set true, the median filter will not executed, to provide a fast preview of the
	 * segmentation.
	 *
	 * \param	_arg	default false.
	 */
	itkSetMacro(UseMedian, bool);

	/**
	* This let you choose the number of connected objects which are preserved after thresholding.
	* The largest connected object (by default air) is always ignored, so you'll get the next largest object by default (1).
	*
	* \param	_arg	default 1.
	*/
	itkSetMacro(ConnectedObjects, unsigned int);

private:

	ConnectedSegmentationFilter();
	~ConnectedSegmentationFilter();

	template<typename TPixel, unsigned int VImageDimension>

	void SegmentationPipeline(itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage);

	void GenerateData() override;
	int m_LowerThreshold;
	int m_UpperThreshold;

	unsigned int m_ConnectedObjects;
	bool m_UseMedian;
};

#endif