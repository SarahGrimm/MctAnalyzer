
#ifndef HoughTransformation2DFilter_h
#define HoughTransformation2DFilter_h

#include <MitkCTSegmentationExports.h>

// MITK
#include <mitkImageToImageFilter.h>
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>
#include <mitkSurface.h>

// ITK
#include <itkHoughTransformRadialVotingImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkGrayscaleDilateImageFilter.h>
#include <itkLabelOverlayImageFilter.h>
#include <itkCastImageFilter.h>

// VTK
#include <vtkRegularPolygonSource.h>



struct CircleStatistics{
public:
	CircleStatistics(){
		origin[0] = 0.0;
		origin[1] = 0.0;
		radius = 0.0;
	}

	mitk::Point2D origin;
	double radius;
};

struct HoughTransform2DStatistics{
public:
	HoughTransform2DStatistics(){
	
	}

	std::vector<CircleStatistics> circles;
	mitk::Image::Pointer circleImage;
};

struct HoughTransform2DInitParameters{
public:
	HoughTransform2DInitParameters(){
		int numSpheres = 1;
		double minRadius = 0.0;
		double maxRadius = 10.0;
		int sigmaGradient = 1.0;
		int variance = 1.0;
		double sphereRadiusRatio = 1.0;
		double votingRadiusRatio = 0.5;
		int threshold = 0.0;
		double outputThreshold = 0.0;
		int gradientThreshold = 0.0;
		int NbOfThreads = 1;
		double samplingRatio = 1.0;
	}

	int numSpheres;
	double minRadius;
	double maxRadius;
	double sigmaGradient;
	double variance;
	double sphereRadiusRatio;
	double votingRadiusRatio;
	double threshold;
	double outputThreshold;
	double gradientThreshold;
	int NbOfThreads;
	double samplingRatio;
};


class MITKCTSEGMENTATION_EXPORT HoughTransformation2DFilter final : public mitk::ImageToImageFilter
{
public:

	mitkClassMacro(HoughTransformation2DFilter, mitk::ImageToImageFilter);
	itkFactorylessNewMacro(Self);

	void InitFilter(HoughTransform2DInitParameters parameterList);
	HoughTransform2DStatistics getStatistics();

	std::vector<mitk::Surface::Pointer> getCircleObjects();

private:

	HoughTransformation2DFilter();
	~HoughTransformation2DFilter();


	HoughTransform2DInitParameters initParams;
	HoughTransform2DStatistics statistics;

	mitk::BaseGeometry::Pointer geometry;


	template<typename TPixel, unsigned int VImageDimension>
	void TrafoPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage);

	void GenerateData() override;

};

#endif