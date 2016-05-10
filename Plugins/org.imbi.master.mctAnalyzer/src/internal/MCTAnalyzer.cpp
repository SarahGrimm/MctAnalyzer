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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "MCTAnalyzer.h"
#include "mctAnalyzerSegmentationFilter.h"

#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>

#include <mitkCuboid.h>
#include <vtkImageMedian3D.h>
#include <mitkImageToSurfaceFilter.h>

//itk
#include <itkScalarImageToHistogramGenerator.h>
#include <itkOtsuThresholdCalculator.h>
#include <itkOtsuMultipleThresholdsCalculator.h>
#include "itkExtractImageFilter.h"
#include "itkBinaryMask3DMeshSource.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkNumericTraits.h"
#include "itkMedianImageFilter.h"
// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>

const std::string MCTAnalyzer::VIEW_ID = "org.mitk.views.mctanalyzer";

void MCTAnalyzer::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void MCTAnalyzer::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
}

void MCTAnalyzer::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls.labelWarning->setVisible( false );
      m_Controls.buttonPerformImageProcessing->setEnabled( true );
      return;
    }
  }

  m_Controls.labelWarning->setVisible( true );
  m_Controls.buttonPerformImageProcessing->setEnabled( false );
}

template < typename TPixel, unsigned int VImageDimension >
void MCTAnalyzer::ItkImageProcessing(itk::Image< TPixel, VImageDimension >* itkImage, mitk::BaseGeometry* imageGeometry)
{
	typedef itk::Image< TPixel, VImageDimension > InputImageType;
	typedef typename InputImageType::IndexType    IndexType;

	// instantiate an patient body filter from given file
	typedef itk::MctSegmentationFilter<InputImageType, InputImageType> segmenationFilter;
	typename segmenationFilter::Pointer filter = segmenationFilter::New();
	filter->SetInput(itkImage); // don't forget this

	filter->Update();

	mitk::Image::Pointer resultImage = mitk::GrabItkImageMemory(filter->GetOutput());




	mitk::DataNode::Pointer newNode = mitk::DataNode::New();
	newNode->SetData(resultImage);
	// set some properties
	newNode->SetProperty("binary", mitk::BoolProperty::New(true));
	newNode->SetProperty("name", mitk::StringProperty::New("dumb segmentation"));
	newNode->SetProperty("color", mitk::ColorProperty::New(0.0781, 0.19531, 1.0));

	//newNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));
	//newNode->SetProperty("showVolume", mitk::BoolProperty::New(true));
	//newNode->SetProperty("reslice interpolation", mitk::BoolProperty::New(true));
	//newNode->SetProperty("layer", mitk::IntProperty::New(1));
	//newNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));
	// add result to data tree
	this->GetDataStorage()->Add(newNode);

	mitk::ImageToSurfaceFilter::Pointer surfaceFilter = mitk::ImageToSurfaceFilter::New();
	surfaceFilter->SetInput(resultImage);
	surfaceFilter->SetThreshold(1);
	surfaceFilter->SetSmooth(true);
	//Downsampling
	surfaceFilter->SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro);

	mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
	surfaceNode->SetData(surfaceFilter->GetOutput());
	surfaceNode->SetProperty("color", mitk::ColorProperty::New(0.0781, 0.19531, 1.0));
	surfaceNode->SetOpacity(0.5);
	surfaceNode->SetName("Surface");
	this->GetDataStorage()->Add(surfaceNode);


	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}



void MCTAnalyzer::DoImageProcessing()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode* node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Template", "Please load and select an image before starting image processing.");
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    if (image)
    {
      std::stringstream message;
      std::string name;
      message << "Performing image processing for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();

	  // applies filter
	  MctSegmentationFilter::Pointer filter = MctSegmentationFilter::New();
	  filter->SetInput(image); // don't forget this

	  filter->Update();


	  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
	  newNode->SetData(filter->GetOutput());
	  // set some properties
	  newNode->SetProperty("binary", mitk::BoolProperty::New(true));
	  newNode->SetProperty("name", mitk::StringProperty::New("dumb segmentation"));
	  newNode->SetProperty("color", mitk::ColorProperty::New(0.0781, 0.19531, 1.0));

	  //newNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));
	  //newNode->SetProperty("showVolume", mitk::BoolProperty::New(true));
	  //newNode->SetProperty("reslice interpolation", mitk::BoolProperty::New(true));
	  //newNode->SetProperty("layer", mitk::IntProperty::New(1));
	  //newNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));
	  // add result to data tree
	  this->GetDataStorage()->Add(newNode);

	  mitk::ImageToSurfaceFilter::Pointer surfaceFilter = mitk::ImageToSurfaceFilter::New();
	  surfaceFilter->SetInput(filter->GetOutput());
	  surfaceFilter->SetThreshold(1);
	  surfaceFilter->SetSmooth(true);
	  //Downsampling
	  surfaceFilter->SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro);

	  mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
	  surfaceNode->SetData(surfaceFilter->GetOutput());
	  surfaceNode->SetProperty("color", mitk::ColorProperty::New(0.0781, 0.19531, 1.0));
	  surfaceNode->SetOpacity(0.5);
	  surfaceNode->SetName("Surface");
	  this->GetDataStorage()->Add(surfaceNode);


	  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    }
  }
}
