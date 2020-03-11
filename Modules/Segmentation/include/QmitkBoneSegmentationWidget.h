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

#ifndef _QmitkBoneSegmentationWidget_H_INCLUDED
#define _QmitkBoneSegmentationWidget_H_INCLUDED

#include "ui_QmitkBoneSegmentationWidget.h"
#include "QmitkStdMultiWidget.h"
#include <MitkCTSegmentationExports.h>
#include <itkOtsuThresholdImageFilter.h>
#include "itkStatisticsImageFilter.h"

#include <mitkImage.h>


class MITKCTSEGMENTATION_EXPORT QmitkBoneSegmentationWidget : public QWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
public:

	QmitkBoneSegmentationWidget(QWidget* parent);
	QmitkBoneSegmentationWidget();
	virtual ~QmitkBoneSegmentationWidget();
	mitk::DataNode::Pointer m_ThresholdFeedbackNode;
	void SetImageNode(mitk::Image::Pointer node, QString name, int lowerTreshold, int upperThreshold);
	void SetDataStorage(mitk::DataStorage::Pointer ds);
	QString GetSegmentName();
	void SetMinThreshold(int);
	void SetMaxThreshold(int);


signals:
	void SubmitThresholds(int, int, QString);

  protected slots:
  void CalculateThresholds();
  void UpdateSegmentationPreview();
  void UpdateSpinLower(double lowerThreshold);
  void UpdateSpinUpper(double upperThreshold);
  void PreviewCBChecked(int state);
  void DoSkullSegmentationCT();



protected:

  void CreateQtPartControl(QWidget *parent);

  void CreateConnections();

  Ui::QmitkBoneSegmentationWidget m_Controls;  ///< gui widget

  mitk::Image::Pointer imageNode;
  mitk::DataStorage::Pointer dataStorage;


private:

	typedef int PixelType;
	typedef itk::Image< PixelType, 3 > RealImageType;

	typedef itk::StatisticsImageFilter< RealImageType > StatisticsImageFilterType;

	typedef itk::OtsuThresholdImageFilter<RealImageType, RealImageType>	OtsuThresholdFilterType;

	RealImageType::Pointer originalImageITK;

	QString segmentName;
	int min = 0;
	int max = 0;

	int minThreshold = -1;
	int maxThreshold = -1;

	static int globalNodeID;
	mitk::Image::Pointer ValidateImage(mitk::DataNode::Pointer node);
	bool m_SegmentationPreview;
	mitk::DataNode::Pointer m_Segmentation;
	void AddNodeToDataStorage(mitk::DataNode::Pointer node);
	mitk::DataNode::Pointer StartSkullSegmentation(mitk::Image::Pointer image, int lowerThreshold, int upperThreshold, bool useMedian);
};
#endif // _QmitkFiducialRegistrationWidget_H_INCLUDED
