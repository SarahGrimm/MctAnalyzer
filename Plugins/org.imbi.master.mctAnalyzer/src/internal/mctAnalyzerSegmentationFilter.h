#ifndef __mctAnalyzerSegmentationFilter_h
#define __mctAnalyzerSegmentationFilter_h

#include <itkImage.h>
#include "itkImage.h"
#include <mitkPixelType.h>
#include <itkImageToImageFilter.h>
#include "itkTransform.h"


#include <itkOtsuThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkAntiAliasBinaryImageFilter.h>
#include "itksys/SystemTools.hxx"
#include <sstream>

namespace itk
{

/*

  \class itk::CTToPatientBodyFilter
  \brief segments the patient body on CT images

*/

template <class TInputImage, class TOutputImage = Image< unsigned short, TInputImage::ImageDimension > >
class ITK_EXPORT MctSegmentationFilter :
    public ImageToImageFilter< TInputImage, TOutputImage>
{
public:

  typedef TInputImage								InputImageType;
  typedef typename InputImageType::Pointer			InputImagePointer;
  typedef typename InputImageType::RegionType		InputImageRegionType; 
  typedef typename InputImageType::PixelType		InputImagePixelType; 
  typedef typename InputImageType::IndexType		IndexType;
  typedef typename InputImageType::SizeType			SizeType;

  typedef itk::Image< unsigned short, InputImageType::ImageDimension >		IntImageType;
  typedef IntImageType														OutputImageType;

  //typedef TOutputImage							OutputImageType;
  typedef typename OutputImageType::Pointer			OutputImagePointer;
  typedef typename OutputImageType::RegionType		OutputImageRegionType; 
  typedef typename OutputImageType::PixelType		OutputImagePixelType; 

  /** Dimension of the input and output images. */
  itkStaticConstMacro (ImageDimension, unsigned int, TInputImage::ImageDimension);
  
  /** Standard "Self" typedef.   */
  typedef MctSegmentationFilter Self;
  
  /** Standard super class typedef support. */
  typedef ImageToImageFilter< InputImageType, OutputImageType > Superclass;

  /** Smart pointer typedef support  */
  typedef SmartPointer<Self> Pointer;

  /** Run-time type information (and related methods) */
  itkTypeMacro(MctSegmentationFilter, ImageToImageFilter);
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);

protected:

  // define filter types
  typedef itk::OtsuThresholdImageFilter<InputImageType, InputImageType>				OtsuThresholdFilterType;
  typedef itk::ConnectedComponentImageFilter<InputImageType, OutputImageType>		ConnectednessFilterType;
  typedef itk::RelabelComponentImageFilter<IntImageType, InputImageType>			RelabelFilterType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType>		RegionGrowerFilterType;
  typedef itk::BinaryThresholdImageFilter<InputImageType, InputImageType>			BinaryThresholdFilterType;

  MctSegmentationFilter();
  virtual ~MctSegmentationFilter()   {}
  MctSegmentationFilter(const Self&) {}
  void operator=(const Self&)        {}

  virtual void GenerateData();

  void PrintSelf(std::ostream& os, Indent indent) const;
};

} // end namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "mctAnalyzerSegmentationFilter.txx"

#endif

#endif

