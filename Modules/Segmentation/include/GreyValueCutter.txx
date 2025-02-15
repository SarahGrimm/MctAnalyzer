/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKBOUNDINGOBJECTCUTTER_TXX
#define MITKBOUNDINGOBJECTCUTTER_TXX

#include "itkImageRegionIteratorWithIndex.h"
#include "mitkImageToItk.h"
#include "mitkStatusBar.h"


  template <typename TPixel, unsigned int VImageDimension, typename TOutputPixel>
  void CutImageWithOutputTypeSelect(itk::Image<TPixel, VImageDimension> *inputItkImage,
	  GreyValueCutter *cutter,
                                    int /* boTimeStep */,
                                    TOutputPixel * /* dummy */)
  {
    typedef itk::Image<TPixel, VImageDimension> ItkInputImageType;
    typedef itk::Image<TOutputPixel, VImageDimension> ItkOutputImageType;
    typedef typename itk::ImageBase<VImageDimension>::RegionType ItkRegionType;
    typedef itk::ImageRegionIteratorWithIndex<ItkInputImageType> ItkInputImageIteratorType;
    typedef itk::ImageRegionIteratorWithIndex<ItkOutputImageType> ItkOutputImageIteratorType;

    if (cutter->m_BoundingObject.IsNull())
      return;

    if (inputItkImage == nullptr)
    {
      mitk::StatusBar::GetInstance()->DisplayErrorText(
        "An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
      std::cout << " image is NULL...returning" << std::endl;
      return;
    }

    // PART 1: convert m_InputRequestedReg ion (type mitk::SlicedData::RegionType)
    // into ITK-image-region (ItkImageType::RegionType)
    // unfortunately, we cannot use input->GetRequestedRegion(), because it
    // has been destroyed by the mitk::CastToItkImage call of PART 1
    // (which sets the m_RequestedRegion to the LargestPossibleRegion).
    // Thus, use our own member m_InputRequestedRegion insead.

    // first convert the index
    typename ItkRegionType::IndexType::IndexValueType tmpIndex[3];
    mitk::itk2vtk(cutter->m_InputRequestedRegion.GetIndex(), tmpIndex);
    typename ItkRegionType::IndexType index;
    index.SetIndex(tmpIndex);

    // then convert the size
    typename ItkRegionType::SizeType::SizeValueType tmpSize[3];
    mitk::itk2vtk(cutter->m_InputRequestedRegion.GetSize(), tmpSize);
    typename ItkRegionType::SizeType size;
    size.SetSize(tmpSize);

    // create the ITK-image-region out of index and size
    ItkRegionType inputRegionOfInterest(index, size);

    // PART 2: get access to the MITK output image via an ITK image
    typename mitk::ImageToItk<ItkOutputImageType>::Pointer outputimagetoitk =
      mitk::ImageToItk<ItkOutputImageType>::New();
    outputimagetoitk->SetInput(cutter->m_OutputTimeSelector->GetOutput());
    outputimagetoitk->Update();
    typename ItkOutputImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

    // PART 3: iterate over input and output using ITK iterators

    // create the iterators
    ItkInputImageIteratorType inputIt(inputItkImage, inputRegionOfInterest);
    ItkOutputImageIteratorType outputIt(outputItkImage, outputItkImage->GetLargestPossibleRegion());

    // Cut the boundingbox out of the image by iterating through
    // all pixels and checking if they are inside using IsInside()
    cutter->m_OutsidePixelCount = 0;
    cutter->m_InsidePixelCount = 0;
    mitk::Point3D p;
    mitk::BaseGeometry *inputGeometry = cutter->GetInput()->GetGeometry();

    TOutputPixel outsideValue;
    if (cutter->m_AutoOutsideValue)
    {
      outsideValue = itk::NumericTraits<TOutputPixel>::min();
    }
    else
    {
      outsideValue = (TOutputPixel)cutter->m_OutsideValue;
    }
    // shall we use a fixed value for each inside pixel?
    if (cutter->GetUseInsideValue())
    {
      TOutputPixel insideValue = (TOutputPixel)cutter->m_InsideValue;
      // yes, use a fixed value for each inside pixel (create a binary mask of the bounding object)
      for (inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
      {
        vtk2itk(inputIt.GetIndex(), p);
        inputGeometry->IndexToWorld(p, p);
        if (cutter->m_BoundingObject->IsInside(p))
        {
          outputIt.Set(insideValue);
          ++cutter->m_InsidePixelCount;
		  cutter->valueSum += insideValue;
        }
        else
        {
          outputIt.Set(outsideValue);
          ++cutter->m_OutsidePixelCount;
        }
      }
    }
    else
    {
      // no, use the pixel value of the original image (normal cutting)
      for (inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
      {
        vtk2itk(inputIt.GetIndex(), p);
        inputGeometry->IndexToWorld(p, p);
        if (cutter->m_BoundingObject->IsInside(p))
        {
          outputIt.Set((TOutputPixel)inputIt.Value());
			//MITK_WARN << inputIt.Value();
          cutter->m_InsidePixelCount++;
		  //MITK_WARN << inputIt.Value();
		  cutter->valueSum += inputIt.Value();
        }
        else
        {
          outputIt.Set(outsideValue);
          ++cutter->m_OutsidePixelCount;
        }
      }
    }


	MITK_WARN << "Summe" << cutter->valueSum;
	MITK_WARN << "PixelCount" << cutter->m_InsidePixelCount;
	MITK_WARN << "Mean" << cutter->valueSum / cutter->m_InsidePixelCount;

  }

  template <typename TPixel, unsigned int VImageDimension, typename TOutputPixel>
  void CutImageWithOutputTypeSelect(itk::VectorImage<TPixel, VImageDimension> *inputItkImage,
	  GreyValueCutter *cutter,
                                    int /* boTimeStep */,
                                    TOutputPixel * /* dummy */)
  {
    typedef itk::VectorImage<TPixel, VImageDimension> ItkInputImageType;
    typedef itk::VectorImage<TOutputPixel, VImageDimension> ItkOutputImageType;
    typedef typename itk::ImageBase<VImageDimension>::RegionType ItkRegionType;
    typedef itk::ImageRegionIteratorWithIndex<ItkInputImageType> ItkInputImageIteratorType;
    typedef itk::ImageRegionIteratorWithIndex<ItkOutputImageType> ItkOutputImageIteratorType;

    if (cutter->m_BoundingObject.IsNull())
      return;

    if (inputItkImage == nullptr)
    {
      mitk::StatusBar::GetInstance()->DisplayErrorText(
        "An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
      std::cout << " image is NULL...returning" << std::endl;
      return;
    }

    // PART 1: convert m_InputRequestedReg ion (type mitk::SlicedData::RegionType)
    // into ITK-image-region (ItkImageType::RegionType)
    // unfortunately, we cannot use input->GetRequestedRegion(), because it
    // has been destroyed by the mitk::CastToItkImage call of PART 1
    // (which sets the m_RequestedRegion to the LargestPossibleRegion).
    // Thus, use our own member m_InputRequestedRegion insead.

    // first convert the index
    typename ItkRegionType::IndexType::IndexValueType tmpIndex[3];
    mitk::itk2vtk(cutter->m_InputRequestedRegion.GetIndex(), tmpIndex);
    typename ItkRegionType::IndexType index;
    index.SetIndex(tmpIndex);

    // then convert the size
    typename ItkRegionType::SizeType::SizeValueType tmpSize[3];
    mitk::itk2vtk(cutter->m_InputRequestedRegion.GetSize(), tmpSize);
    typename ItkRegionType::SizeType size;
    size.SetSize(tmpSize);

    // create the ITK-image-region out of index and size
    ItkRegionType inputRegionOfInterest(index, size);

    // PART 2: get access to the MITK output image via an ITK image
    typename mitk::ImageToItk<ItkOutputImageType>::Pointer outputimagetoitk =
      mitk::ImageToItk<ItkOutputImageType>::New();
    outputimagetoitk->SetInput(cutter->m_OutputTimeSelector->GetOutput());
    outputimagetoitk->Update();
    typename ItkOutputImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

    // PART 3: iterate over input and output using ITK iterators

    // create the iterators
    ItkInputImageIteratorType inputIt(inputItkImage, inputRegionOfInterest);
    ItkOutputImageIteratorType outputIt(outputItkImage, outputItkImage->GetLargestPossibleRegion());

    // Cut the boundingbox out of the image by iterating through
    // all pixels and checking if they are inside using IsInside()
    cutter->m_OutsidePixelCount = 0;
    cutter->m_InsidePixelCount = 0;
    mitk::Point3D p;
    mitk::BaseGeometry *inputGeometry = cutter->GetInput()->GetGeometry();

    typename ItkOutputImageType::PixelType outsideValue;
    outsideValue.SetSize(outputItkImage->GetVectorLength());
    if (cutter->m_AutoOutsideValue)
    {
      outsideValue.Fill(itk::NumericTraits<TOutputPixel>::min());
    }
    else
    {
      outsideValue.Fill(cutter->m_OutsideValue);
    }
    // shall we use a fixed value for each inside pixel?
    if (cutter->GetUseInsideValue())
    {
      typename ItkOutputImageType::PixelType insideValue;
      insideValue.Fill(cutter->m_InsideValue);
      // yes, use a fixed value for each inside pixel (create a binary mask of the bounding object)
      for (inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
      {
        vtk2itk(inputIt.GetIndex(), p);
        inputGeometry->IndexToWorld(p, p);
        if (cutter->m_BoundingObject->IsInside(p))
        {
          outputIt.Set(insideValue);
          ++cutter->m_InsidePixelCount;
		  
        }
        else
        {
          outputIt.Set(outsideValue);
          ++cutter->m_OutsidePixelCount;
        }
      }
    }
    else
    {
      // no, use the pixel value of the original image (normal cutting)
      for (inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
      {
        vtk2itk(inputIt.GetIndex(), p);
        inputGeometry->IndexToWorld(p, p);
        if (cutter->m_BoundingObject->IsInside(p))
        {
          outputIt.Set(inputIt.Get());
          ++cutter->m_InsidePixelCount;
        }
        else
        {
          outputIt.Set(outsideValue);
          ++cutter->m_OutsidePixelCount;
        }
      }
    }
  }

  template <typename TPixel, unsigned int VImageDimension>
  void CutImage(itk::Image<TPixel, VImageDimension> *inputItkImage, GreyValueCutter *cutter, int boTimeStep)
  {
    TPixel *dummy = nullptr;
    CutImageWithOutputTypeSelect<TPixel, VImageDimension, TPixel>(inputItkImage, cutter, boTimeStep, dummy);
  }

  template <typename TPixel, unsigned int VImageDimension>
  void CutImage(itk::VectorImage<TPixel, VImageDimension> *inputItkImage,
	  GreyValueCutter *cutter,
                int boTimeStep)
  {
    TPixel *dummy = nullptr;
    CutImageWithOutputTypeSelect<TPixel, VImageDimension, TPixel>(inputItkImage, cutter, boTimeStep, dummy);
  }



#include "mitkImageCast.h"

#endif // of MITKBOUNDINGOBJECTCUTTER_TXX