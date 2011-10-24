/*=========================================================================

  Program:   ParaView
  Module:    vtkSICollaborationManager.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSICollaborationManager.h"

#include "vtkObjectFactory.h"
#include "vtkSMMessage.h"
#include "vtkPVSession.h"
#include "vtkPVSessionServer.h"
#include "vtkPVMultiClientsInformation.h"
#include "vtkReservedRemoteObjectIds.h"
#include "vtkProcessModule.h"
#include "vtkMultiProcessController.h"
#include "vtkCompositeMultiProcessController.h"
#include "vtkCommand.h"

#include <assert.h>
#include <vtkstd/map>
#include <vtkNew.h>
#include <vtkWeakPointer.h>
#include <vtkstd/string>
#include <vtksys/ios/sstream>

//****************************************************************************
//                            Internal class
//****************************************************************************
class vtkSICollaborationManager::vtkInternal : public vtkCommand
{
public:
  static vtkInternal* New()
    {
    return new vtkInternal();
    }

  vtkInternal()
    {
    this->DisableBroadcast = false;
    this->ServerState.set_location(vtkPVSession::DATA_SERVER_ROOT);
    this->ServerState.set_global_id(vtkReservedRemoteObjectIds::RESERVED_COLLABORATION_COMMUNICATOR_ID);
    this->ServerState.SetExtension(DefinitionHeader::client_class, "vtkSMCollaborationManager");
    this->ServerState.SetExtension(DefinitionHeader::server_class, "vtkSICollaborationManager");

    vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
    this->ServerSession = vtkPVSessionServer::SafeDownCast(pm->GetSession());
    if(this->ServerSession)
      {
      this->MultiProcessController =
          vtkCompositeMultiProcessController::SafeDownCast(this->ServerSession->GetController(vtkPVSession::CLIENT));
      if(this->MultiProcessController)
        {
        // We don't need to remove it later the vtkCommand take care of that when
        // the object get deleted
        this->MultiProcessController->AddObserver(
            vtkCompositeMultiProcessController::CompositeMultiProcessControllerChanged, this);
        }
      }
    }

  bool UpdateUserNamesAndMaster(vtkSMMessage* msg)
    {
    bool findChanges = false;
    this->DisableBroadcast = true;
    int size = msg->ExtensionSize(ClientsInformation::user);
    for(int i=0; i < size; ++i)
      {
      const ClientsInformation_ClientInfo* user =
          &msg->GetExtension(ClientsInformation::user, i);
      int id = user->user();
      findChanges = findChanges || (this->UserNames[id] != user->name());
      this->UserNames[id] = user->name().c_str();
      if(user->is_master() && this->MultiProcessController)
        {
        findChanges = findChanges ||
                      (this->MultiProcessController->GetMasterController() != id);
        this->MultiProcessController->SetMasterController(id);
        }
      }
    this->DisableBroadcast = false;
    return findChanges;
    }

  vtkSMMessage* BuildServerStateMessage()
    {
    this->ServerInformations->CopyFromObject(NULL);
    int master = this->ServerInformations->GetMasterId();

    this->ServerState.ClearExtension(ClientsInformation::user);
    this->ServerState.ExtensionSize(ClientsInformation::user); // force the clear to work
    for(int i=0; i < this->ServerInformations->GetNumberOfClients(); i++)
      {
      ClientsInformation_ClientInfo* user =
          this->ServerState.AddExtension(ClientsInformation::user);
      int userId = this->ServerInformations->GetClientId(i);
      user->set_user(userId);
      if(this->UserNames[userId].empty())
        {
        vtksys_ios::ostringstream newUserName;
        newUserName << "User " << userId;
        this->UserNames[userId] = newUserName.str().c_str();
        }
      user->set_name(this->UserNames[userId]);
      if(userId == master)
        {
        user->set_is_master(true);
        }
      }

    return &this->ServerState;
    }

  void BroadcastToClients(vtkSMMessage* msg)
    {
    if(this->ServerSession && !this->DisableBroadcast)
      {
      this->ServerSession->BroadcastToClients(msg);
      }
    }

  void Execute(vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(eventId), void* vtkNotUsed(callData))
    {
    // A client has disconnect, let's notify the clients left
    this->BroadcastToClients(this->BuildServerStateMessage());
    }

  vtkWeakPointer<vtkPVSessionServer>   ServerSession;
  vtkNew<vtkPVMultiClientsInformation> ServerInformations;
  vtkSMMessage                         ServerState;
  vtkstd::map<int, vtkstd::string>     UserNames;
  vtkWeakPointer<vtkCompositeMultiProcessController>   MultiProcessController;
  bool                                 DisableBroadcast;
};

//****************************************************************************
vtkStandardNewMacro(vtkSICollaborationManager);
//----------------------------------------------------------------------------
vtkSICollaborationManager::vtkSICollaborationManager()
{
  this->Internal = vtkInternal::New();
}

//----------------------------------------------------------------------------
vtkSICollaborationManager::~vtkSICollaborationManager()
{
  this->Internal->Delete();
  this->Internal = NULL;
}

//----------------------------------------------------------------------------
void vtkSICollaborationManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkSICollaborationManager::Push(vtkSMMessage* msg)
{
  if(this->Internal->UpdateUserNamesAndMaster(msg))
    {
    this->Internal->BroadcastToClients(this->Internal->BuildServerStateMessage());
    }
}

//----------------------------------------------------------------------------
void vtkSICollaborationManager::Pull(vtkSMMessage* msg)
{
  msg->CopyFrom(*this->Internal->BuildServerStateMessage());
}