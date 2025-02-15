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

#include "GreyValueCutter.h"
#include "mitkBoundingObject.h"
#include "GreyValueCutter.txx"
#include "mitkGeometry3D.h"
#include "mitkImageAccessByItk.h"
#include <mitkImageAccessByItk.h>
#include "mitkTimeHelper.h"

#include <math.h>


	void GreyValueCutter::SetBoundingObject(const mitk::BoundingObject *boundingObject)
	{
		m_BoundingObject = const_cast<mitk::BoundingObject *>(boundingObject);
		// Process object is not const-correct so the const_cast is required here
		this->ProcessObject::SetNthInput(1, const_cast<mitk::BoundingObject *>(boundingObject));
	}

	const mitk::BoundingObject *GreyValueCutter::GetBoundingObject() const { return m_BoundingObject.GetPointer(); }
	GreyValueCutter::GreyValueCutter()
		: m_BoundingObject(nullptr),
		m_InsideValue(1),
		m_OutsideValue(0),
		m_AutoOutsideValue(false),
		m_UseInsideValue(false),
		m_OutsidePixelCount(0),
		m_InsidePixelCount(0),
		valueSum(0),
		m_UseWholeInputRegion(false)
	{
		this->SetNumberOfIndexedInputs(2);
		this->SetNumberOfRequiredInputs(2);
		m_InputTimeSelector = mitk::ImageTimeSelector::New();
		m_OutputTimeSelector = mitk::ImageTimeSelector::New();
	}

	GreyValueCutter::~GreyValueCutter() {}
	const mitk::PixelType GreyValueCutter::GetOutputPixelType() { return this->GetInput()->GetPixelType(); }
	void GreyValueCutter::GenerateInputRequestedRegion()
	{
		mitk::Image *output = this->GetOutput();
		if ((output->IsInitialized() == false) || (m_BoundingObject.IsNull()) ||
			(m_BoundingObject->GetTimeGeometry()->CountTimeSteps() == 0))
			return;
		// we have already calculated the spatial part of the
		// input-requested-region in m_InputRequestedRegion in
		// GenerateOutputInformation (which is called before
		// GenerateInputRequestedRegion).
		GenerateTimeInInputRegion(output, this->GetInput());
		GenerateTimeInInputRegion(output, m_BoundingObject.GetPointer());
	}

	void GreyValueCutter::GenerateOutputInformation()
	{
		mitk::Image::Pointer output = this->GetOutput();
		if ((output->IsInitialized()) && (output->GetPipelineMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
			return;

		mitk::Image::Pointer input = this->GetInput();

		if (input.IsNull())
		{
			mitkThrow() << "Input is not a mitk::Image";
		}
		itkDebugMacro(<< "GenerateOutputInformation()");
		unsigned int dimension = input->GetDimension();

		if (dimension < 3)
		{
			mitkThrow() << "ImageCropper cannot handle 1D or 2D Objects.";
		}

		if ((m_BoundingObject.IsNull()) || (m_BoundingObject->GetTimeGeometry()->CountTimeSteps() == 0))
			return;

		mitk::BaseGeometry *boGeometry = m_BoundingObject->GetGeometry();
		mitk::BaseGeometry *inputImageGeometry = input->GetSlicedGeometry();
		// calculate bounding box of bounding-object relative to the geometry
		// of the input image. The result is in pixel coordinates of the input
		// image (because the m_IndexToWorldTransform includes the spacing).
		mitk::BoundingBox::Pointer boBoxRelativeToImage =
			boGeometry->CalculateBoundingBoxRelativeToTransform(inputImageGeometry->GetIndexToWorldTransform());

		// PART I: initialize input requested region. We do this already here (and not
		// later when GenerateInputRequestedRegion() is called), because we
		// also need the information to setup the output.

		// pre-initialize input-requested-region to largest-possible-region
		// and correct time-region; spatial part will be cropped by
		// bounding-box of bounding-object below
		m_InputRequestedRegion = input->GetLargestPossibleRegion();

		// build region out of bounding-box of bounding-object
		mitk::SlicedData::IndexType index = m_InputRequestedRegion.GetIndex(); // init times and channels
		mitk::BoundingBox::PointType min = boBoxRelativeToImage->GetMinimum();
		index[0] = (mitk::SlicedData::IndexType::IndexValueType)(std::ceil(min[0]));
		index[1] = (mitk::SlicedData::IndexType::IndexValueType)(std::ceil(min[1]));
		index[2] = (mitk::SlicedData::IndexType::IndexValueType)(std::ceil(min[2]));

		mitk::SlicedData::SizeType size = m_InputRequestedRegion.GetSize(); // init times and channels
		mitk::BoundingBox::PointType max = boBoxRelativeToImage->GetMaximum();
		size[0] = (mitk::SlicedData::SizeType::SizeValueType)(std::ceil(max[0]) - index[0]);
		size[1] = (mitk::SlicedData::SizeType::SizeValueType)(std::ceil(max[1]) - index[1]);
		size[2] = (mitk::SlicedData::SizeType::SizeValueType)(std::ceil(max[2]) - index[2]);

		mitk::SlicedData::RegionType boRegion(index, size);

		if (m_UseWholeInputRegion == false)
		{
			// crop input-requested-region with region of bounding-object
			if (m_InputRequestedRegion.Crop(boRegion) == false)
			{
				// crop not possible => do nothing: set time size to 0.
				size.Fill(0);
				m_InputRequestedRegion.SetSize(size);
				boRegion.SetSize(size);
				m_BoundingObject->SetRequestedRegion(&boRegion);
				mitkThrow() << "No overlap of the image and the cropping object.";
			}
		}

		// set input-requested-region, because we access it later in
		// GenerateInputRequestedRegion (there we just set the time)
		input->SetRequestedRegion(&m_InputRequestedRegion);

		// PART II: initialize output image

		auto dimensions = new unsigned int[dimension];
		mitk::itk2vtk(m_InputRequestedRegion.GetSize(), dimensions);
		if (dimension > 3)
			memcpy(dimensions + 3, input->GetDimensions() + 3, (dimension - 3) * sizeof(unsigned int));
		output->Initialize(mitk::PixelType(GetOutputPixelType()), dimension, dimensions);
		delete[] dimensions;

		// now we have everything to initialize the transform of the output
		mitk::SlicedGeometry3D *slicedGeometry = output->GetSlicedGeometry();

		// set the transform: use the transform of the input;
		// the origin will be replaced afterwards
		mitk::AffineTransform3D::Pointer indexToWorldTransform = mitk::AffineTransform3D::New();
		indexToWorldTransform->SetParameters(input->GetSlicedGeometry()->GetIndexToWorldTransform()->GetParameters());
		slicedGeometry->SetIndexToWorldTransform(indexToWorldTransform);

		// Position the output Image to match the corresponding region of the input image
		const mitk::SlicedData::IndexType &start = m_InputRequestedRegion.GetIndex();
		mitk::Point3D origin;
		vtk2itk(start, origin);
		inputImageGeometry->IndexToWorld(origin, origin);
		slicedGeometry->SetOrigin(origin);

		m_TimeOfHeaderInitialization.Modified();
	}

void GreyValueCutter::ComputeData(mitk::Image *input3D, int boTimeStep)
	{
		if (input3D != nullptr && input3D->GetPixelType().GetNumberOfComponents() == 1)
		{
			AccessFixedDimensionByItk_2(input3D, CutImage, 3, this, boTimeStep);
		}
		else
		{
			AccessVectorFixedDimensionByItk_n(input3D, CutImage, 3, (this, boTimeStep));
		}
	}

void GreyValueCutter::GenerateData()
	{
		mitk::Image::ConstPointer input = this->GetInput();
		mitk::Image::Pointer output = this->GetOutput();

		if (input.IsNull())
			return;

		if ((output->IsInitialized() == false) || (m_BoundingObject.IsNull()) ||
			(m_BoundingObject->GetTimeGeometry()->CountTimeSteps() == 0))
			return;

		m_InputTimeSelector->SetInput(input);
		m_OutputTimeSelector->SetInput(this->GetOutput());

		mitk::Surface::RegionType outputRegion = output->GetRequestedRegion();
		const mitk::TimeGeometry *outputTimeGeometry = output->GetTimeGeometry();
		const mitk::TimeGeometry *inputTimeGeometry = input->GetTimeGeometry();
		const mitk::TimeGeometry *boundingObjectTimeGeometry = m_BoundingObject->GetTimeGeometry();
		mitk::TimePointType timeInMS;

		int timestep = 0;
		int tstart = outputRegion.GetIndex(3);
		int tmax = tstart + outputRegion.GetSize(3);

		int t;
		for (t = tstart; t < tmax; ++t)
		{
			timeInMS = outputTimeGeometry->TimeStepToTimePoint(t);
			timestep = inputTimeGeometry->TimePointToTimeStep(timeInMS);

			m_InputTimeSelector->SetTimeNr(timestep);
			m_InputTimeSelector->UpdateLargestPossibleRegion();
			m_OutputTimeSelector->SetTimeNr(t);
			m_OutputTimeSelector->UpdateLargestPossibleRegion();

			timestep = boundingObjectTimeGeometry->TimePointToTimeStep(timeInMS);

			ComputeData(m_InputTimeSelector->GetOutput(), timestep);
		}

		m_InputTimeSelector->SetInput(nullptr);
		m_OutputTimeSelector->SetInput(nullptr);

		m_TimeOfHeaderInitialization.Modified();
	}

