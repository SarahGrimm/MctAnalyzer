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

#include "QmitkBoneSegmentationWidget.h"

#include <HistogramCalculator.h>
#include <QMessageBox>

#include "mitkImageCast.h"
#include "NodeHelper.h"
#include "ConnectedSegmentationFilter.h"




#define FRW_LOG MITK_INFO("Bone Segmentation Widget")
#define FRW_WARN MITK_WARN("Bone Segmentation Widget")
#define FRW_DEBUG MITK_DEBUG("Bone Segmentation Widget")

int QmitkBoneSegmentationWidget::globalNodeID = 0;

/* VIEW MANAGEMENT */
QmitkBoneSegmentationWidget::QmitkBoneSegmentationWidget(QWidget* parent)
: QWidget(parent)
{
  CreateQtPartControl(this);
}

QmitkBoneSegmentationWidget::QmitkBoneSegmentationWidget()
{
	CreateQtPartControl(this);
}



QmitkBoneSegmentationWidget::~QmitkBoneSegmentationWidget()
{
 
}


void QmitkBoneSegmentationWidget::CreateQtPartControl(QWidget *parent)
{
	m_Controls.setupUi(parent);
	m_SegmentationPreview = true;

	m_Controls.thresholdSlider->setOrientation(Qt::Orientation::Horizontal);
	int lowerThreshold = -1024;
	int upperThreshold = 3071;

	//configuring initial values

	m_Controls.thresholdSlider->setRange(lowerThreshold, upperThreshold);
	m_Controls.thresholdSlider->setMaximumValue(upperThreshold);
	m_Controls.thresholdSlider->setMinimumValue(300);

	m_Controls.lowerThresholdSpin->setRange(lowerThreshold, upperThreshold);
	m_Controls.upperThresholdSpin->setRange(lowerThreshold, upperThreshold);

	m_Controls.lowerThresholdSpin->setValue(300);
	m_Controls.upperThresholdSpin->setValue(upperThreshold);

	//m_Controls.otsuThresholds->setVisible(false);
	//m_Controls.automaticLowerThreshold->setVisible(false);
    this->CreateConnections();
  
}

void QmitkBoneSegmentationWidget::SetDataStorage(mitk::DataStorage::Pointer ds){
	dataStorage = ds;


	m_ThresholdFeedbackNode = dataStorage->GetNamedNode("segmentation");
	if (!m_ThresholdFeedbackNode) {
		m_ThresholdFeedbackNode = NodeHelper::BuildBaseDataNode(NULL, true, true, "segmentation", 0.3, mitk::ColorProperty::New(1.0, 0.74, 0.0));
	}

	//m_ThresholdFeedbackNode->SetProperty("helper object", mitk::BoolProperty::New(true));

	if (!dataStorage->Exists(m_ThresholdFeedbackNode)){
		dataStorage->Add(m_ThresholdFeedbackNode);
	}

}


void QmitkBoneSegmentationWidget::SetMinThreshold(int min) {

	minThreshold = min;

}

void QmitkBoneSegmentationWidget::SetMaxThreshold(int max) {

	maxThreshold = max;

}

void QmitkBoneSegmentationWidget::SetImageNode(mitk::Image::Pointer node, QString name, int lowerTreshold, int upperThreshold){

	imageNode = node;
	segmentName = name;

	mitk::CastToItkImage(imageNode, originalImageITK);


	StatisticsImageFilterType::Pointer statisticsImageFilter
		= StatisticsImageFilterType::New();
	statisticsImageFilter->SetInput(originalImageITK);
	statisticsImageFilter->Update();

	min = statisticsImageFilter->GetMinimum();
	max = statisticsImageFilter->GetMaximum();

	std::cout << "Mean: " << statisticsImageFilter->GetMean() << std::endl;
	std::cout << "Std.: " << statisticsImageFilter->GetSigma() << std::endl;
	std::cout << "Min: " << statisticsImageFilter->GetMinimum() << std::endl;
	std::cout << "Max: " << statisticsImageFilter->GetMaximum() << std::endl;

	m_Controls.thresholdSlider->setRange(min, max);
	m_Controls.lowerThresholdSpin->setRange(min, max);
	m_Controls.upperThresholdSpin->setRange(min, max);

	
	if (lowerTreshold == -1 || upperThreshold == -1) {
		lowerTreshold = min;
		upperThreshold = max;
	}

	m_Controls.thresholdSlider->setMaximumValue(upperThreshold);
	m_Controls.thresholdSlider->setMinimumValue(lowerTreshold);

	m_Controls.lowerThresholdSpin->setValue(lowerTreshold);
	m_Controls.upperThresholdSpin->setValue(upperThreshold);


	if (m_ThresholdFeedbackNode){
		if (imageNode){
			MITK_WARN << "Segmentation preview...";

			ConnectedSegmentationFilter::Pointer segmentationFilter = ConnectedSegmentationFilter::New();
			segmentationFilter->SetLowerThreshold(lowerTreshold);
			segmentationFilter->SetUpperThreshold(upperThreshold);
			segmentationFilter->SetInput(imageNode);
			segmentationFilter->SetUseMedian(false);
			segmentationFilter->Update();

			MITK_WARN << "... done";
			m_ThresholdFeedbackNode->SetData(segmentationFilter->GetOutput());

			m_ThresholdFeedbackNode->SetIntProperty("layer", 1000);
			NodeHelper::setCrosshairVisibility(false, dataStorage);
			MITK_WARN << "... done2";
		}
	}

	//m_ThresholdFeedbackNode->SetData(NULL);
	m_Controls.thresholdSlider->setEnabled(true);
	m_Controls.upperThresholdSpin->setEnabled(true);
	m_Controls.lowerThresholdSpin->setEnabled(true);
}


void QmitkBoneSegmentationWidget::CreateConnections()
{
	connect(m_Controls.automaticLowerThreshold, SIGNAL(clicked()), this, SLOT(CalculateThresholds()));
	connect(m_Controls.thresholdSlider, SIGNAL(sliderReleased()), this, SLOT(UpdateSegmentationPreview()));
	connect(m_Controls.thresholdSlider, SIGNAL(minimumValueChanged(double)), this, SLOT(UpdateSpinLower(double)));
	connect(m_Controls.thresholdSlider, SIGNAL(maximumValueChanged(double)), this, SLOT(UpdateSpinUpper(double)));
	connect(m_Controls.previewCB, SIGNAL(stateChanged(int)), this, SLOT(PreviewCBChecked(int)));
	connect(m_Controls.immediateSegmentation, SIGNAL(clicked()), this, SLOT(DoSkullSegmentationCT()));
}

mitk::Image::Pointer QmitkBoneSegmentationWidget::ValidateImage(mitk::DataNode::Pointer node){
	mitk::Image::Pointer rawImage;
	if (!node){
		MITK_ERROR << "node was null";
		return NULL;
	}
	else {
		rawImage = static_cast<mitk::Image*> (node->GetData());
		if (!rawImage){
			MITK_ERROR << "node was no mitk image";

			return NULL;
		}
		return rawImage;
	}
}

void QmitkBoneSegmentationWidget::PreviewCBChecked(int state){
	// unchecked
	if (state == 0){
		m_SegmentationPreview = false;
		dataStorage->Remove(m_ThresholdFeedbackNode);
		m_ThresholdFeedbackNode->SetData(NULL);
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
	// checked
	else if (state == 2){
		m_SegmentationPreview = true;
		dataStorage->Add(m_ThresholdFeedbackNode);
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
}

void QmitkBoneSegmentationWidget::CalculateThresholds(){
	mitk::Image::Pointer rawImage = imageNode;
	if (!rawImage){
		return;
	}

	m_Controls.automaticLowerThreshold->setDisabled(true);
	m_Controls.automaticLowerThreshold->repaint();
	
	OtsuThresholdFilterType::Pointer otsu = OtsuThresholdFilterType::New();
	//otsu->SetInput(median->GetOutput());
	otsu->SetInput(originalImageITK);
	otsu->SetInsideValue(1);
	otsu->SetOutsideValue(0);
	otsu->ReleaseDataFlagOn();
	otsu->Update();
	std::cout << "otsu filter was applied..." << otsu->GetThreshold() << std::endl;


	int lowerThreshold = otsu->GetThreshold();
	int upperThreshold = max;

	m_Controls.thresholdSlider->setMinimumValue(lowerThreshold);
	m_Controls.thresholdSlider->setMaximumValue(upperThreshold);
	 
	m_Controls.thresholdSlider->repaint();
	m_Controls.automaticLowerThreshold->setDisabled(false);
	
	UpdateSegmentationPreview();
}

void QmitkBoneSegmentationWidget::DoSkullSegmentationCT()
{
	
	mitk::Image* image = imageNode;
	
	this->hide();
	dataStorage->Remove(m_ThresholdFeedbackNode);
	int lowerThreshold = m_Controls.thresholdSlider->minimumValue();
	int upperThreshold = m_Controls.thresholdSlider->maximumValue();

	MITK_WARN << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH";
	emit SubmitThresholds(lowerThreshold, upperThreshold, segmentName);
	
	
	/*if (image)
	{
		// remove preview
		m_Controls.previewCB->setChecked(false);
		
		MITK_INFO << " - Segmentation ...";
		m_Controls.immediateSegmentation->setDisabled(true);
		m_Controls.immediateSegmentation->repaint();

		AddNodeToDataStorage(StartSkullSegmentation(image, m_Controls.thresholdSlider->minimumValue(), m_Controls.thresholdSlider->maximumValue(), true));

		MITK_INFO << "... done";
		NodeHelper::setCrosshairVisibility(false, dataStorage);
		m_Controls.immediateSegmentation->setDisabled(false);
	}
	else {
		QMessageBox::information(NULL, "Segmentation", "Something went wrong!");
	}*/
}

mitk::DataNode::Pointer QmitkBoneSegmentationWidget::StartSkullSegmentation(mitk::Image::Pointer image, int lowerThreshold, int upperThreshold, bool useMedian){
	ConnectedSegmentationFilter::Pointer segmentationFilter = ConnectedSegmentationFilter::New();
	segmentationFilter->SetLowerThreshold(lowerThreshold);
	segmentationFilter->SetUpperThreshold(upperThreshold);
	segmentationFilter->SetInput(image);
	segmentationFilter->SetUseMedian(useMedian);
	segmentationFilter->Update();

	return NodeHelper::CreateSegmentatioNode(segmentationFilter->GetOutput(), "Skull segmentation");
}



void QmitkBoneSegmentationWidget::AddNodeToDataStorage(mitk::DataNode::Pointer node){
	globalNodeID++;
	std::string nodeName = node->GetName() + " " + std::to_string(globalNodeID);
	node->SetName(nodeName);
	dataStorage->Add(node);
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkBoneSegmentationWidget::UpdateSpinLower(double lowerThreshold){
	m_Controls.lowerThresholdSpin->setValue((int)lowerThreshold);
}

void QmitkBoneSegmentationWidget::UpdateSpinUpper(double upperThreshold){
	m_Controls.upperThresholdSpin->setValue((int)upperThreshold);
}

void QmitkBoneSegmentationWidget::UpdateSegmentationPreview(){
	MITK_WARN << "changed1";
	if (!m_SegmentationPreview){
		return;
	}
	int lowerThreshold = m_Controls.thresholdSlider->minimumValue();
	int upperThreshold = m_Controls.thresholdSlider->maximumValue();

	mitk::Image::Pointer image;
	MITK_WARN << "changed";
	if (m_ThresholdFeedbackNode){
		image = imageNode;
		if (image){
			MITK_WARN << "Segmentation preview...";
	
			ConnectedSegmentationFilter::Pointer segmentationFilter = ConnectedSegmentationFilter::New();
			segmentationFilter->SetLowerThreshold(lowerThreshold);
			segmentationFilter->SetUpperThreshold(upperThreshold);
			segmentationFilter->SetInput(image);
			segmentationFilter->SetUseMedian(false);
			segmentationFilter->Update();

			MITK_WARN << "... done";
			m_ThresholdFeedbackNode->SetData(segmentationFilter->GetOutput());
			
			m_ThresholdFeedbackNode->SetIntProperty("layer", 1000);
			NodeHelper::setCrosshairVisibility(false, dataStorage);
			MITK_WARN << "... done2";
		}
	}
}