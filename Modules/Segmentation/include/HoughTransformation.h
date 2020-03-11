

#ifndef HoughTransformation_h
#define HoughTransformation_h

#include <MitkCTSegmentationExports.h>

// ITK
#include <itkHoughTransformRadialVotingImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkGrayscaleDilateImageFilter.h>
#include <itkLabelOverlayImageFilter.h>

// MITK
#include <mitkImage.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkMatrix3x3.h>
#include <vtkVector.h>


class MITKCTSEGMENTATION_EXPORT HoughTransformation
{
public:

	HoughTransformation();
	~HoughTransformation();

	void setInputImage(mitk::Image::Pointer inputImage);

	void calcHoughTransform3D();

private:

	mitk::Image::Pointer inputImage;

};

#endif