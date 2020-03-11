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


#ifndef MCTAnalyzer_h
#define MCTAnalyzer_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <itkImage.h>
#include <mitkImage.h>
#include "mitkSurface.h"
#include <chrono>
#include "ui_MCTAnalyzerControls.h"



/**
  \brief MCTAnalyzer

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class MCTAnalyzer : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void DoImageProcessing();
	void ChooseDirectories();
	void ShowPreview();
	void ManualSegmentation();
	void AddSegment();
	void SetOutputDirectory();
	void AddExamination();
	void toggled(bool);
	void ShowImages(bool);
	void SaveThresholds(int, int, QString);
	void setValue(int);
	void ShowPoreVis(bool);

  protected:

    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes ) override;

    Ui::MCTAnalyzerControls m_Controls;
private:



	struct Pore {
		mitk::Point3D origin;
		double radius;
	};

	struct Threshold {
		int min;
		int max;
	};



	QMap<int, QStringList > pathsForSample;
	QString path;
	QVBoxLayout *verticalLayout;
	int counter = 0;
	int examinationCounter = 0;
	bool showImages = false;
	std::map<QString, mitk::Image::Pointer> images; 
	std::map<int, std::vector<Pore>> poreSizeRange;
	std::map<QString, Threshold> thresholds;

	void RemoveNode(const std::string& name);
	bool ExistsNodeWithName(const std::string& name);
	void DeleteAllNodes();

	std::chrono::time_point<std::chrono::system_clock>          m_StartTime;
	std::chrono::time_point<std::chrono::system_clock>          m_EndTime;
	double center[3];
	std::map<std::string, double> resultMap;
	
	mitk::Image::Pointer Generate3DImageFromImageStack(const QString&);
	mitk::Image::Pointer ScaffoldSegmentation(const mitk::Image::Pointer originalImage, const QString&);
	mitk::Image::Pointer MCTAnalyzer::CreateContourFromMask(const mitk::Image::Pointer  maski, const mitk::Image::Pointer);
	mitk::Surface::Pointer CalculateScaffoldSurface(const mitk::Image::Pointer segmentedImage, double& surface, double, const mitk::Image::Pointer originalImage);
	mitk::Surface::Pointer CalculateConvexSurface(const mitk::Surface::Pointer segmentedImage, double& surface, double);
	mitk::Image::Pointer CreateMaskOfConvexSurface( mitk::Surface::Pointer segmentedImage,  mitk::Image::Pointer originalImage);
	std::vector<double> FindPores(const mitk::Image::Pointer originalImage, double, int&, int&, const mitk::Image::Pointer  mask, const mitk::Surface::Pointer convexHull);
	void DoHoughTransform(const mitk::Image::Pointer  maski);
	void WriteResults(QString name, QString sample);
	void WritePoreSizeRange(QString name, QString sample);
	void GlobalReinit();

	double op_volume(double i);
	double op_volume2(double i) { return (1 / 6) * 3.14159265359 * std::pow(i, 3.0); }
};

#endif // MCTAnalyzer_h
