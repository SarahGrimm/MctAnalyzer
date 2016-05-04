#ifndef _mctAnalyzerSegmentationFilter_txx
#define _mctAnalyzerSegmentationFilter_txx

#include "mctAnalyzerSegmentationFilter.h"
#include "itkImageRegionIterator.h"
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include "itkLabelToRGBImageFilter.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>
#include <mitkImage.h>
#include <Qt>

#include "itksys/SystemTools.hxx"
#include <sstream>

namespace itk
{


template< class TInputImage, class TOutputImage> 
MctSegmentationFilter<TInputImage, TOutputImage>::MctSegmentationFilter()
{
 int i = 0; 
}


template< class TInputImage, class TOutputImage>
void MctSegmentationFilter<TInputImage, TOutputImage>::GenerateData()
{
  // create a filter pipeline
  
  // otsu filter
  typename OtsuThresholdFilterType::Pointer otsu = OtsuThresholdFilterType::New();
  otsu->SetInput(this->GetInput());
  otsu->SetInsideValue(0);
  otsu->SetOutsideValue(1);
  otsu->Update();
  std::cout << "otsu filter was applied..." << std::endl;

  //connected component filter
  typename ConnectednessFilterType::Pointer connectedComponent = ConnectednessFilterType::New();
  connectedComponent->SetInput(otsu->GetOutput());
  connectedComponent->SetFullyConnected(true);
  connectedComponent->Update();
  std::cout << "connected component filter was applied..." << std::endl;
  std::cout << "number of objects: " << connectedComponent->GetObjectCount() << std::endl;

  // relabel component filter
  typename RelabelFilterType::Pointer relabelFilter = RelabelFilterType::New();
  relabelFilter->SetInput(connectedComponent->GetOutput());
  relabelFilter->Update();
  std::cout << "relabel component filter was applied..." << std::endl;

  // maintain region of interest (1) and mark subregions (0) as background
  typename BinaryThresholdFilterType::Pointer regionOfInterestFilter = BinaryThresholdFilterType::New();
  regionOfInterestFilter->SetInput(relabelFilter->GetOutput());
  regionOfInterestFilter->SetLowerThreshold(1);
  regionOfInterestFilter->SetUpperThreshold(1);
  regionOfInterestFilter->Update();
  std::cout << "region of interest is maintained..." << std::endl;

  typename RegionGrowerFilterType::Pointer regionGrower = RegionGrowerFilterType::New();
  regionGrower->SetInput(regionOfInterestFilter->GetOutput());

  //make sure to segment only the surrounding area
  regionGrower->SetLower(0);
  regionGrower->SetUpper(0);

  InputImageRegionType region = regionOfInterestFilter->GetOutput()->GetLargestPossibleRegion();
  SizeType size = region.GetSize();

  InputImageType::IndexType seed;
  //std::cout << "size: " << size << "..." << std::endl;
  // Seed 1: (0, 0, 0)
  seed[0] = 0;
  seed[1] = 0;
  seed[2] = 0;
  regionGrower->SetSeed(seed);
  std::cout << "seed: " << seed << " is inside? " << region.IsInside(seed) << std::endl;

  regionGrower->Update();
  std::cout << "region grower was applied.." << std::endl;

  //merge holes from region grower(0) with region of interest(1)
  typename BinaryThresholdFilterType::Pointer merger = BinaryThresholdFilterType::New();
  merger->SetInput(regionGrower->GetOutput());
  merger->SetLowerThreshold(0);
  merger->SetUpperThreshold(0);
  merger->Update();
  std::cout << "merger was applied.." << std::endl;


  // set output
  this->GraftOutput(merger->GetOutput());
  std::cout << "...all filters are applied!" << std::endl;

}


template<class TInputImage, class TOutputImage>
void MctSegmentationFilter<TInputImage, TOutputImage>::PrintSelf(std::ostream& os, Indent indent) const
{
 // Superclass::PrintSelf(os,indent);
}

} // end namespace itk

#endif

