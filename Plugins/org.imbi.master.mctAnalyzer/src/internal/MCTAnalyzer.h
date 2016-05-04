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

  protected:

    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes ) override;

    Ui::MCTAnalyzerControls m_Controls;
private:
	template < typename TPixel, unsigned int VImageDimension >
	void ItkImageProcessing(itk::Image< TPixel, VImageDimension >* itkImage, mitk::BaseGeometry* imageGeometry);

};

#endif // MCTAnalyzer_h
