#ifndef __mctAnalyzerRegionalMaximaFilter_h
#define __mctAnalyzerRegionalMaximaFilter_h

#include <mitkImageToImageFilter.h>
#include <MitkCTSegmentationExports.h>

#include <itkImageToImageFilter.h>

#include <mitkSurface.h>
#include "mitkPointSet.h"






class MITKCTSEGMENTATION_EXPORT MctAnalyzerRegionalMaximaFilter final : public mitk::ImageToImageFilter
{
public:

	mitkClassMacro(MctAnalyzerRegionalMaximaFilter, mitk::ImageToImageFilter);
	itkFactorylessNewMacro(Self);
	void SetBoundingObject(const mitk::BoundingBox::Pointer boundingObject);
	const mitk::BoundingBox::Pointer GetBoundingObject() const;

	const mitk::PointSet::Pointer GetPointSet() const;


private:

	MctAnalyzerRegionalMaximaFilter();
	~MctAnalyzerRegionalMaximaFilter();
 
	mitk::BoundingBox::Pointer convexHull;

	mitk::PointSet::Pointer pointSet;

  template<typename TPixel, unsigned int VImageDimension>
  void SegmentationPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage);

  void GenerateData() override;

};

#endif

