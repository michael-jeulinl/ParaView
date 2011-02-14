/*=========================================================================

Program:   ParaView
Module:    pvrenderserver.cxx

Copyright (c) Kitware, Inc.
All rights reserved.
See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "pvserver_common.h"

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  // Init current process type
  vtkPVServerOptions* options = vtkPVServerOptions::New();
  bool success = RealMain(argc, argv, options,
    vtkProcessModule::PROCESS_RENDER_SERVER);
  options->Delete();
  return success? 0 : 1;
}