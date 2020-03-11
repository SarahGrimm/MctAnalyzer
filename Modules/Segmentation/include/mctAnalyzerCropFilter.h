#ifndef __mctAnalyzerCropFilter_h
#define __mctAnalyzerCropFilter_h

#include <mitkImageToImageFilter.h>
#include <MitkCTSegmentationExports.h>

#include <itkImageToImageFilter.h>

#include <mitkSurface.h>

#include <itkOtsuThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkAntiAliasBinaryImageFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>




class MITKCTSEGMENTATION_EXPORT MctAnalyzerCropFilter final : public mitk::ImageToImageFilter
{
public:

	mitkClassMacro(MctAnalyzerCropFilter, mitk::ImageToImageFilter);
	itkFactorylessNewMacro(Self);
	void SetBoundingObject(const mitk::BoundingBox::Pointer boundingObject);
	const mitk::BoundingBox::Pointer GetBoundingObject() const;

	void SetMask(const mitk::Image::Pointer boundingObject);
	const mitk::Image::Pointer GetMask() const;


private:

	MctAnalyzerCropFilter();
	~MctAnalyzerCropFilter();

	mitk::Image::Pointer m_mask;
	mitk::BoundingBox::Pointer convexHull;
  template<typename TPixel, unsigned int VImageDimension>
  void SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage);

  void GenerateData() override;

};

#endif

