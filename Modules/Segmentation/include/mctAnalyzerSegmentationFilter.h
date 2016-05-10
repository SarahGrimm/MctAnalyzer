#ifndef __mctAnalyzerSegmentationFilter_h
#define __mctAnalyzerSegmentationFilter_h

#include <mitkImageToImageFilter.h>
#include <SegmentationExports.h>

#include <itkImageToImageFilter.h>



#include <itkOtsuThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkAntiAliasBinaryImageFilter.h>




class Segmentation_EXPORT MctSegmentationFilter final : public mitk::ImageToImageFilter
{
public:

	mitkClassMacro(MctSegmentationFilter, mitk::ImageToImageFilter);
	itkFactorylessNewMacro(Self);


private:

  MctSegmentationFilter();
  ~MctSegmentationFilter();
 
  template<typename TPixel, unsigned int VImageDimension>
  void SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage);

  void GenerateData() override;

};

#endif

