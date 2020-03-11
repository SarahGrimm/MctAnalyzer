#ifndef __mctAnalyzerDistanceMapFilter_h
#define __mctAnalyzerDistanceMapFilter_h

#include <mitkImageToImageFilter.h>
#include <MitkCTSegmentationExports.h>

#include <itkImageToImageFilter.h>



#include <itkOtsuThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkAntiAliasBinaryImageFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>




class MITKCTSEGMENTATION_EXPORT MctAnalyzerDistanceMapFilter final : public mitk::ImageToImageFilter
{
public:

	mitkClassMacro(MctAnalyzerDistanceMapFilter, mitk::ImageToImageFilter);
	itkFactorylessNewMacro(Self);
	void SetBoundingObject(const mitk::BoundingBox::Pointer boundingObject);
	const mitk::BoundingBox::Pointer GetBoundingObject() const;


private:

	MctAnalyzerDistanceMapFilter();
	~MctAnalyzerDistanceMapFilter();

	mitk::BoundingBox::Pointer convexHull;
 
  template<typename TPixel, unsigned int VImageDimension>
  void SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage);

  void GenerateData() override;

};

#endif

