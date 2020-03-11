
#ifndef HoughTransformation3DFilter_h
#define HoughTransformation3DFilter_h

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
#include <vtkSphereSource.h>


struct SphereStatistics{
public:
	SphereStatistics(){
		// default values...
		origin[0] = 0.0;
		origin[1] = 0.0;
		origin[2] = 0.0;
		radius = 0.0;
	}

	mitk::Point3D origin;
	double radius;
};

struct HoughTransform3DStatistics{
public:
	HoughTransform3DStatistics(){

	}

	std::vector<SphereStatistics> spheres;
	mitk::Image::Pointer accumulatorImage;
};

struct HoughTransform3DInitParameters{
public:
	HoughTransform3DInitParameters(){
		// default values...
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

class MITKCTSEGMENTATION_EXPORT HoughTransformation3DFilter final : public mitk::ImageToImageFilter
{
public:

	mitkClassMacro(HoughTransformation3DFilter, mitk::ImageToImageFilter);
	itkFactorylessNewMacro(Self);

	void InitFilter(HoughTransform3DInitParameters parameterList);
	HoughTransform3DStatistics getStatistics();

	std::vector<mitk::Surface::Pointer> getSphereObjects();

private:

	HoughTransformation3DFilter();
	~HoughTransformation3DFilter();


	HoughTransform3DInitParameters initParams;
	HoughTransform3DStatistics statistics;

	mitk::BaseGeometry::Pointer geometry;


	template<typename TPixel, unsigned int VImageDimension>
	void TrafoPipeline(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image::Pointer outputImage);

	void GenerateData() override;
};

#endif