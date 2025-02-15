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


#ifndef org_imbi_master_mctAnalyzer_Activator_h
#define org_imbi_master_mctAnalyzer_Activator_h

#include <ctkPluginActivator.h>

namespace mitk {

class org_imbi_master_mctAnalyzer_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_imbi_master_mctAnalyzer")
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

}; // org_imbi_master_mctAnalyzer_Activator

}

#endif // org_imbi_master_mctAnalyzer_Activator_h
