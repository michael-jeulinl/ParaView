/*=========================================================================

   Program: ParaView
   Module:    pqSelectionManager.h

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.1. 

   See License_v1.1.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#ifndef __pqSelectionManager_h
#define __pqSelectionManager_h

#include "pqComponentsExport.h"
#include "pqServerManagerSelectionModel.h"

#include <QObject>

class vtkSMDisplayProxy;
class vtkSMProxy;
class vtkSelection;
class pqPipelineSource;
class pqRenderModule;
class pqSelectionManagerImplementation;

/// pqSelectionManager is the link between the server manager level
/// selection and the GUI. It keeps a list of selected proxies and
/// corresponding point/cell list proxies.
class PQCOMPONENTS_EXPORT pqSelectionManager : public QObject
{
  Q_OBJECT

public:
  pqSelectionManager(QObject* parent=NULL);
  virtual ~pqSelectionManager();

  /// Returns the mode the manager is in. In SELECT mode, the
  /// interaction involves drawing a rubber-band. In INTERACT
  /// mode, the user can interact with the view by rotating, panning
  // etc.
  int getMode() 
    {
      return this->Mode;
    }

  enum Modes
  {
    SELECT,
    INTERACT
  };

  friend class vtkPQSelectionObserver;

public slots:
  /// Change mode to SELECT
  void switchToSelection();
  /// Change mode to INTERACT
  void switchToInteraction();
  /// Clear all selections. Note that this does not clear
  /// the server manager model selection
  void clearSelection();
  /// Used to keep track of active render module
  void setActiveRenderModule(pqRenderModule*);

private slots:
  void sourceRemoved(pqPipelineSource*);
  void proxyUnRegistered(QString, QString, vtkSMProxy*);

private:
  pqSelectionManagerImplementation* Implementation;
  int Mode;
  int setInteractorStyleToSelect(pqRenderModule*);
  int setInteractorStyleToInteract(pqRenderModule*);
  void processEvents(unsigned long event);
  vtkSMDisplayProxy* getDisplayProxy(pqRenderModule*, vtkSMProxy*);
  void createDisplayProxies(vtkSMProxy*);

  void updateSelection(int* eventpos, pqRenderModule* rm);
  vtkSelection* selectInFrustrum(int* eventpos, pqRenderModule* rm);
  void sendSelection(vtkSelection* sel, vtkSMProxy* proxy);
  vtkSelection* selectOnSurface(pqRenderModule* rm);
  void convertSelection(pqRenderModule* rm, vtkSelection* sel);

  void clearClientDisplays();
  void createNewClientDisplays(pqServerManagerSelection&);
};


#endif

