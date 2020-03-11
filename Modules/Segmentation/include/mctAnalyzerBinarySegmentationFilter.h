#ifndef __mctAnalyzerBinarySegmentationFilter_h
#define __mctAnalyzerBinarySegmentationFilter_h

#include <mitkImageToImageFilter.h>
#include <MitkCTSegmentationExports.h>

#include <itkImageToImageFilter.h>



#include <itkOtsuThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkAntiAliasBinaryImageFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>




class MITKCTSEGMENTATION_EXPORT MctAnalyzerBinarySegmentationFilter final : public mitk::ImageToImageFilter
{
public:

	mitkClassMacro(MctAnalyzerBinarySegmentationFilter, mitk::ImageToImageFilter);
	itkFactorylessNewMacro(Self);


private:

	MctAnalyzerBinarySegmentationFilter();
	~MctAnalyzerBinarySegmentationFilter();
 
  template<typename TPixel, unsigned int VImageDimension>
  void SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage);

  void GenerateData() override;

};

#endif

