/** @file
  Copyright ©2021 Liu Yi, efikarl@yeah.net

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
--*/
#include <Uefi.h>

#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/ZeroGuid.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiAttributeQuestionLib.h>
#include <Library/UefiHiiServicesLib.h>

#include "HiiAttributeQuestionLib.h"

LIST_ENTRY    mFormSetList            = INITIALIZE_LIST_HEAD_VARIABLE(mFormSetList);
LIST_ENTRY    mGotoTmpList            = INITIALIZE_LIST_HEAD_VARIABLE(mGotoTmpList);
LIST_ENTRY    gAttributeQuestionList  = INITIALIZE_LIST_HEAD_VARIABLE(gAttributeQuestionList);

BOOLEAN AttributeQuestionIsOp (
  UINT8                                 OpCode
) {
  switch (OpCode) {
  case EFI_IFR_ONE_OF_OP: case EFI_IFR_CHECKBOX_OP: case EFI_IFR_NUMERIC_OP: case EFI_IFR_STRING_OP: case EFI_IFR_ORDERED_LIST_OP:
    return TRUE;
  default:
    break;
  }
  return FALSE;
}

EFI_STATUS
AttributeQuestionFormSetRegister (
  IN     EFI_HII_HANDLE                 HiiHandle,
  IN     EFI_GUID                       *FormSetId
) {
  EFI_STATUS                            Status;
  EFI_HII_PACKAGE_LIST_HEADER           *PackageList;
  UINTN                                 PackageListSize;
  EFI_HII_PACKAGE_HEADER                *Package;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  UINT8                                 Index;
  EFI_GUID                              *ClassGuid;
  UINT8                                 ClassGuidCount;
  BOOLEAN                               FormSetFound;
  FORMSET_NODE                          *NodeOfFormSet;
  UINTN                                 InTheFormSet;

  DEBUG ((DEBUG_VERBOSE, "%a().+\n", __FUNCTION__));
  Package     = NULL;
  OpCodeData  = NULL;
  //
  // Get HII PackageList
  //
  PackageList     = NULL;
  PackageListSize = 0;
  Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, HiiHandle, &PackageListSize, PackageList);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    PackageList = AllocateZeroPool (PackageListSize);
    ASSERT (PackageList != NULL);
    Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, HiiHandle, &PackageListSize, PackageList);
  }
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  ASSERT (PackageList != NULL);
  //
  // get form package from this hii package list
  //
  Package = (EFI_HII_PACKAGE_HEADER *) ((UINTN) PackageList + sizeof (EFI_HII_PACKAGE_LIST_HEADER));
  for (Package = (EFI_HII_PACKAGE_HEADER *) ((UINTN) PackageList + sizeof (EFI_HII_PACKAGE_LIST_HEADER));
       (UINTN) Package < (UINTN) PackageList + PackageListSize;
       Package = (EFI_HII_PACKAGE_HEADER *) ((UINTN) Package + (UINTN) Package->Length)) {
    if (Package->Type != EFI_HII_PACKAGE_FORMS) {
      continue;
    }
    //
    // search formset in this form package
    //
    InTheFormSet = 0;
    for (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) Package + sizeof (EFI_HII_PACKAGE_HEADER));
         (UINTN) OpCodeData < (UINTN) Package + Package->Length;
         OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) OpCodeData + OpCodeData->Length)) {
      if (InTheFormSet) {
        if (OpCodeData->Scope) {
          InTheFormSet++;
        } else if (OpCodeData->OpCode == EFI_IFR_END_OP) {
          InTheFormSet--;
        }
        if (!InTheFormSet) {
          NodeOfFormSet->FormSetSize = (UINTN) OpCodeData + OpCodeData->Length - (UINTN) NodeOfFormSet->FormSet;
          NodeOfFormSet->FormSet = AllocateCopyPool(NodeOfFormSet->FormSetSize, NodeOfFormSet->FormSet);
          if (!NodeOfFormSet->FormSet) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
          }
          Status = AttributeQuestionFormListInit(NodeOfFormSet->FormSet, NodeOfFormSet->FormSetSize, &NodeOfFormSet->FormList);
          if (EFI_ERROR (Status)) {
            goto Done;
          }
        }
      }
      if (OpCodeData->OpCode != EFI_IFR_FORM_SET_OP) {
        continue;
      }
      FormSetFound = FALSE;
      if (FormSetId == NULL) {
        FormSetFound = TRUE;
      } else if (CompareGuid (FormSetId, &((EFI_IFR_FORM_SET *) OpCodeData)->Guid)) {
        FormSetFound = TRUE;
      } else if (((EFI_IFR_OP_HEADER *) OpCodeData)->Length > sizeof (EFI_IFR_FORM_SET)) {
        ClassGuid       = (EFI_GUID *) (OpCodeData + sizeof (EFI_IFR_FORM_SET));
        ClassGuidCount  = (UINT8) (((EFI_IFR_FORM_SET *) OpCodeData)->Flags & 0x3);
        for (Index = 0; Index < ClassGuidCount; Index++) {
          if (CompareGuid (FormSetId, ClassGuid + Index)) {
            FormSetFound = TRUE;
            break;
          }
        }
      }
      if (FormSetFound) {
        InTheFormSet = 1;
        //
        // init mFormSetList database
        //
        NodeOfFormSet = AllocateZeroPool(sizeof(FORMSET_NODE));
        if (!NodeOfFormSet) {
          Status = EFI_OUT_OF_RESOURCES;
          goto Done;
        }
        InsertTailList(&mFormSetList, &NodeOfFormSet->Link);
        NodeOfFormSet->Signature    = FORMSET_NODE_SIGNATURE;
        NodeOfFormSet->HiiHandle    = HiiHandle;
        NodeOfFormSet->FormSet      = (EFI_IFR_FORM_SET *) OpCodeData;
        NodeOfFormSet->FormSetSize  = 0;
        InitializeListHead(&NodeOfFormSet->FormList);
      }
    }
  }
  FreePool (PackageList);

  Status = EFI_SUCCESS;
Done:
  DEBUG ((DEBUG_VERBOSE, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

VOID
AttributeQuestionFormSetFree (
  VOID
) {
  LIST_ENTRY                            *Link0;
  LIST_ENTRY                            *NextLink0;
  LIST_ENTRY                            *Link1;
  LIST_ENTRY                            *NextLink1;
  FORMSET_NODE                          *NodeOfFormSet;
  FORM_NODE                             *NodeOfForm;
  //
  // free mFormSetList
  //
  BASE_LIST_FOR_EACH_SAFE(Link0, NextLink0, &mFormSetList) {
    NodeOfFormSet = CR(Link0, FORMSET_NODE, Link, FORMSET_NODE_SIGNATURE);
    BASE_LIST_FOR_EACH_SAFE(Link1, NextLink1, &NodeOfFormSet->FormList) {
      NodeOfForm = CR(Link1, FORM_NODE, Link, FORM_NODE_SIGNATURE);
      RemoveEntryList(&NodeOfForm->Link);
      FreePool(NodeOfForm);
    }
    RemoveEntryList(&NodeOfFormSet->Link);
    FreePool(NodeOfFormSet);
  }
}

EFI_STATUS
AttributeQuestionFormSetRegisterAll (
  VOID
) {
  EFI_STATUS                            Status = EFI_NOT_STARTED;
  EFI_HII_HANDLE                        *HiiHandles;
  UINTN                                 Index;

  HiiHandles = HiiGetHiiHandles (NULL);
  if (!HiiHandles[0]) {
    return EFI_NOT_READY;
  }
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    Status = AttributeQuestionFormSetRegister(HiiHandles[Index], NULL);
    DEBUG ((DEBUG_INFO, "%a(): Index = %02d, Status = %r\n", __FUNCTION__, Index, Status));
  }

  if (EFI_ERROR(Status)) {
    AttributeQuestionFormSetFree();
  }
  return Status;
}

FORMSET_NODE *
AttributeQuestionGetFormSet (
  IN      CONST EFI_HII_HANDLE          HiiHandle,
  IN      CONST EFI_GUID                *FormSetId
) {
  FORMSET_NODE                          *NodeOfFormSet = NULL;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormSetList) {
    NodeOfFormSet = CR(Link, FORMSET_NODE, Link, FORMSET_NODE_SIGNATURE);
    if (!HiiHandle) {
      if (CompareGuid(FormSetId, &NodeOfFormSet->FormSet->Guid)) {
        break;
      }
    } else {
      if ((HiiHandle == NodeOfFormSet->HiiHandle) && CompareGuid(FormSetId, &NodeOfFormSet->FormSet->Guid)) {
        break;
      }
    }
  }
  return NodeOfFormSet;
}

EFI_HII_HANDLE *
AttributeQuestionGetHiiHandle (
  IN      CONST EFI_GUID                *FormSetId
) {
  EFI_HII_HANDLE                        HiiHandle      = NULL;
  FORMSET_NODE                          *NodeOfFormSet = NULL;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormSetList) {
    NodeOfFormSet = CR(Link, FORMSET_NODE, Link, FORMSET_NODE_SIGNATURE);
    //
    // address compare 1st to increase potential to get the right handle
    //
    if (CompareGuid(FormSetId, &NodeOfFormSet->FormSet->Guid)) {
      HiiHandle = NodeOfFormSet->HiiHandle;
      break;
    }
  }
  return HiiHandle;
}

EFI_STATUS
AttributeQuestionFormListInit (
  IN      CONST EFI_IFR_FORM_SET        *FormSet,
  IN            UINTN                   FormSetSize,
  IN            LIST_ENTRY              *FormList
) {
  EFI_STATUS                            Status;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  EFI_IFR_FORM                          *Form;
  UINTN                                 InTheForm;
  FORM_NODE                             *NodeOfForm;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;
  //
  // for one formset
  //
  DEBUG ((DEBUG_VERBOSE, "%a().+\n", __FUNCTION__));
  //
  // for each formset
  //
  OpCodeData = (EFI_IFR_OP_HEADER *) FormSet;
  while ((UINTN) (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) OpCodeData + OpCodeData->Length)) < (UINTN) FormSet + FormSetSize) {
    if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode != EFI_IFR_FORM_OP) {
      continue;
    }
    Form = (EFI_IFR_FORM *) OpCodeData;
    InTheForm=1;
    while ((UINTN) (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) OpCodeData + OpCodeData->Length)) < (UINTN) FormSet + FormSetSize) {
      if (OpCodeData->Scope) {
        InTheForm++;
      } else if (OpCodeData->OpCode == EFI_IFR_END_OP) {
        InTheForm--;
      }
      if (!InTheForm) {
        break;
      }
    }
    //
    // create FORM_NODE
    //
    NodeOfForm = AllocateZeroPool(sizeof(FORM_NODE));
    if (!NodeOfForm) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }
    InsertTailList(FormList, &NodeOfForm->Link);
    NodeOfForm->Signature = FORM_NODE_SIGNATURE;
    NodeOfForm->Form      = Form;
    NodeOfForm->FormSize  = (UINTN) OpCodeData + OpCodeData->Length - (UINTN) Form;
  }

  Status = EFI_SUCCESS;
Done:
  if (EFI_ERROR (Status)) {
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, FormList) {
      NodeOfForm = CR(Link, FORM_NODE, Link, FORM_NODE_SIGNATURE);
      RemoveEntryList(&NodeOfForm->Link);
      FreePool(NodeOfForm);
    }
  }
  DEBUG ((DEBUG_VERBOSE, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

FORM_NODE *
AttributeQuestionGetForm (
  IN      CONST EFI_GUID                *FormSetId,
  IN      CONST UINT16                  FormId
) {
  FORMSET_NODE                          *NodeOfFormSet  = NULL;
  FORM_NODE                             *NodeOfForm     = NULL;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  NodeOfFormSet = AttributeQuestionGetFormSet(NULL, FormSetId);
  if (!NodeOfFormSet) {
    goto Done;
  }

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &NodeOfFormSet->FormList) {
    NodeOfForm = CR(Link, FORM_NODE, Link, FORM_NODE_SIGNATURE);
    if (NodeOfForm->Form->FormId == FormId) {
      break;
    }
  }

Done:
  return NodeOfForm;
}

BOOLEAN
AttributeQuestionIsFirstForm (
  IN      CONST EFI_GUID                *FormSetId,
  IN      CONST UINT16                  FormId
) {
  FORMSET_NODE                          *NodeOfFormSet  = NULL;
  FORM_NODE                             *NodeOfForm     = NULL;

  NodeOfFormSet = AttributeQuestionGetFormSet(NULL, FormSetId);
  if (!NodeOfFormSet) {
    goto Done;
  }
  if (IsListEmpty(&NodeOfFormSet->FormList)) {
    return TRUE;
  }
  NodeOfForm = CR(NodeOfFormSet->FormList.ForwardLink, FORM_NODE, Link, FORM_NODE_SIGNATURE);
  if (FormId == NodeOfForm->Form->FormId) {
    return TRUE;
  }

Done:
  return FALSE;
}

EFI_STATUS
AttributeQuestionListInit (
  IN            FORMSET_NODE            *RootNodeOfFormSet,
  IN            EFI_HII_HANDLE           ThisHiiHandle,
  IN            EFI_GUID                *ThisFormSetId
) {
  EFI_STATUS                            Status;
  FORMSET_NODE                          *ThisNodeOfFormSet;
  FORM_NODE                             *NodeOfForm;
  AQ_GOTO                               *NodeOfGoto;
  EFI_IFR_QUESTION_HEADER               *Question;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  AQ_NODE                               *AttributeQuestion;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;
  BOOLEAN                               FirstFormHasBeenGot;
  EFI_IFR_FORM                          *FirstForm;
  EFI_HII_HANDLE                        NextThisHiiHandle;
  EFI_GUID                              *NextThisFormSetId;

  DEBUG ((DEBUG_VERBOSE, "%a().+\n", __FUNCTION__));

  if (!(ThisHiiHandle && ThisFormSetId)) {
    //
    // reset mGotoTmpList for new form list
    //
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mGotoTmpList) {
      NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
      RemoveEntryList(&NodeOfGoto->Link);
      FreePool(NodeOfGoto);
    }
    ThisHiiHandle = RootNodeOfFormSet->HiiHandle;
    ThisFormSetId =&RootNodeOfFormSet->FormSet->Guid;
  }
  ThisNodeOfFormSet = AttributeQuestionGetFormSet(NULL, ThisFormSetId);
  //
  // for each attribute question in this each form
  //
  FirstFormHasBeenGot = FALSE;
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &ThisNodeOfFormSet->FormList) {
    NodeOfForm = CR(Link, FORM_NODE, Link, FORM_NODE_SIGNATURE);
    if (!FirstFormHasBeenGot) {
      FirstForm = NodeOfForm->Form;
      FirstFormHasBeenGot = TRUE;
    }
    OpCodeData = (EFI_IFR_OP_HEADER *) NodeOfForm->Form;
    while ((UINTN) (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) OpCodeData + OpCodeData->Length)) < (UINTN) NodeOfForm->Form + NodeOfForm->FormSize) {
      if (AttributeQuestionIsOp(OpCodeData->OpCode)) {
        //
        // for each attribute question
        //
        Question = (EFI_IFR_QUESTION_HEADER *) &((EFI_IFR_REF5 *) OpCodeData)->Question;
        //
        // create AQ_NODE
        //
        AttributeQuestion = AllocateZeroPool(sizeof(AQ_NODE));
        if (!AttributeQuestion) {
          Status = EFI_OUT_OF_RESOURCES;
          goto Done;
        }
        InsertTailList(&gAttributeQuestionList, &AttributeQuestion->Link);
        AttributeQuestion->Signature          = AQ_NODE_SIGNATURE;
        AttributeQuestion->RootHiiHandle      = RootNodeOfFormSet->HiiHandle;
        AttributeQuestion->RootFormSetId      =&RootNodeOfFormSet->FormSet->Guid;
        AttributeQuestion->RootFormId         = FirstForm->FormId;
        AttributeQuestion->ThisHiiHandle      = ThisHiiHandle;
        AttributeQuestion->ThisFormSetId      = ThisFormSetId;
        AttributeQuestion->QuestionId         = Question->QuestionId;
        AttributeQuestion->Title[0]           = RootNodeOfFormSet->FormSet->FormSetTitle;
        AttributeQuestion->Title[1]           = FirstForm->FormTitle;
        AttributeQuestion->Prompt             = Question->Header.Prompt;
        InitializeListHead(&AttributeQuestion->GotoList);
      } else if (OpCodeData->OpCode == EFI_IFR_REF_OP) {
        if (OpCodeData->Length >= sizeof(EFI_IFR_REF3)) {
          NextThisFormSetId = &((EFI_IFR_REF3*) OpCodeData)->FormSetId;
          NextThisHiiHandle = AttributeQuestionGetHiiHandle(NextThisFormSetId);
          Status = AttributeQuestionListInit(RootNodeOfFormSet, NextThisHiiHandle, NextThisFormSetId);
          if (Status == EFI_OUT_OF_RESOURCES) {
            goto Done;
          }
        }
      }
    }
  }

  Status = EFI_SUCCESS;
Done:
  if (Status == EFI_OUT_OF_RESOURCES) {
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &gAttributeQuestionList) {
      AttributeQuestion = CR(Link, AQ_NODE, Link, AQ_NODE_SIGNATURE);
      RemoveEntryList(&AttributeQuestion->Link);
      FreePool(AttributeQuestion);
    }
  }
  DEBUG ((DEBUG_VERBOSE, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

VOID
DebugGotoNode (
  AQ_GOTO                               *NodeOfGoto
) {
  DEBUG ((DEBUG_INFO, "@AQ_GOTO:\n"));
  DEBUG ((DEBUG_INFO,
    "  FormSetId      : %g\n"
    "  FormId         : %04x\n"
    "  Title[0]       : %s\n"
    "  Title[1]       : %s\n"
    "  Depth          : %u\n",
    NodeOfGoto->FormSetId, NodeOfGoto->FormId,
    HiiGetString(NodeOfGoto->HiiHandle[0], NodeOfGoto->Title[0], NULL),
    HiiGetString(NodeOfGoto->HiiHandle[1], NodeOfGoto->Title[1], NULL),
    NodeOfGoto->Depth
    ));
}

VOID
AttributeQuestionPrint (
  AQ_NODE                               *AttributeQuestion
) {
  CHAR16                                Url[QUESTION_URL_LENGTH];
  AQ_GOTO                               *NodeOfGoto;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  ZeroMem(Url, sizeof(Url));
  StrCatS(Url, QUESTION_URL_LENGTH, QUESTION_URL_SLASH);
  StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(AttributeQuestion->RootHiiHandle, AttributeQuestion->Title[0], NULL));
  StrCatS(Url, QUESTION_URL_LENGTH, QUESTION_URL_SLASH);
  StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(AttributeQuestion->RootHiiHandle, AttributeQuestion->Title[1], NULL));
  StrCatS(Url, QUESTION_URL_LENGTH, QUESTION_URL_SLASH);
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &AttributeQuestion->GotoList) {
    NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
    StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(NodeOfGoto->HiiHandle[0], NodeOfGoto->Title[0], NULL));
    StrCatS(Url, QUESTION_URL_LENGTH, QUESTION_URL_SLASH);
    StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(NodeOfGoto->HiiHandle[1], NodeOfGoto->Title[1], NULL));
    StrCatS(Url, QUESTION_URL_LENGTH, QUESTION_URL_SLASH);
  }
  StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(AttributeQuestion->ThisHiiHandle, AttributeQuestion->Prompt, NULL));
  DEBUG((DEBUG_INFO, "Url: %s\n", Url));
}

EFI_STATUS
AttributeQuestionGoThruForm (
  IN            AQ_NODE                 *AttributeQuestion,
  IN            EFI_HII_HANDLE          LastGotoHandle,
  IN            UINT16                  LastGotoPrompt,
  IN            FORMSET_NODE            *LastNodeOfFormSet,
  IN            UINT16                  LastFormId
) {
  EFI_STATUS                            Status;
  FORMSET_NODE                          *NextNodeOfFormSet;
  FORM_NODE                             *LastNodeOfForm;
  EFI_IFR_QUESTION_HEADER               *Question;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;
  AQ_GOTO                               *NodeOfGoto;
  EFI_HII_HANDLE                        NextGotoHandle;
  UINT16                                NextGotoPrompt;

  DEBUG ((DEBUG_INFO, "%a().+ LastFormId = %04x\n", __FUNCTION__, LastFormId));
  //
  // resolve LastNodeOfFormSet and LastNodeOfForm
  //
  if (!LastNodeOfFormSet) {
    //
    // if no LastNodeOfFormSet, AttributeQuestion->Root* as last
    //
    LastNodeOfFormSet = AttributeQuestionGetFormSet(AttributeQuestion->RootHiiHandle, AttributeQuestion->RootFormSetId);
    if (!LastNodeOfFormSet) {
      Status = EFI_NOT_READY;
      goto Done;
    }
    //
    // if no LastFormId, 1st form from LastNodeOfFormSet->FormList as last
    //
    if (!LastFormId) {
      if (!IsListEmpty(&LastNodeOfFormSet->FormList)) {
        LastNodeOfForm = CR(LastNodeOfFormSet->FormList.ForwardLink, FORM_NODE, Link, FORM_NODE_SIGNATURE);
      } else {
        Status = EFI_NOT_READY;
        goto Done;
      }
    }
  } else {
    //
    // get the right LastNodeOfForm from NodeOfFormSet->FormList
    //
    if (!LastFormId) {
      if (!IsListEmpty(&LastNodeOfFormSet->FormList)) {
        LastNodeOfForm = CR(LastNodeOfFormSet->FormList.ForwardLink, FORM_NODE, Link, FORM_NODE_SIGNATURE);
      } else {
        Status = EFI_NOT_READY;
        goto Done;
      }
    } else {
      LastNodeOfForm = NULL;
      BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &LastNodeOfFormSet->FormList) {
        LastNodeOfForm = CR(Link, FORM_NODE, Link, FORM_NODE_SIGNATURE);
        if (LastNodeOfForm->Form->FormId == LastFormId) {
          break;
        }
      }
      if (!LastNodeOfForm) {
        Status = EFI_NOT_READY;
        goto Done;
      }
    }
    //
    // create AQ_GOTO
    //
    NodeOfGoto = AllocateZeroPool(sizeof(AQ_GOTO));
    if (!NodeOfGoto) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }
    InsertTailList(&mGotoTmpList, &NodeOfGoto->Link);
    NodeOfGoto->Signature     = AQ_GOTO_SIGNATURE;
    NodeOfGoto->FormSetId     = &LastNodeOfFormSet->FormSet->Guid;
    NodeOfGoto->FormId        = LastNodeOfForm->Form->FormId;
    NodeOfGoto->HiiHandle[0]  = LastGotoHandle;
    NodeOfGoto->HiiHandle[1]  = AttributeQuestionGetHiiHandle(NodeOfGoto->FormSetId);
    NodeOfGoto->Title[0]      = LastGotoPrompt;
    NodeOfGoto->Title[1]      = LastNodeOfForm->Form->FormTitle;
    if (IsNodeAtEnd(&mGotoTmpList, mGotoTmpList.ForwardLink)) {
      NodeOfGoto->Depth = 1;
    } else {
      NodeOfGoto->Depth = (CR(NodeOfGoto->Link.BackLink, AQ_GOTO, Link, AQ_GOTO_SIGNATURE))->Depth + 1;
    }
    DebugGotoNode(NodeOfGoto);
    //
    // in case of goto each other: as success and return
    //
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mGotoTmpList) {
      NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
      if (!IsNodeAtEnd(&mGotoTmpList, &NodeOfGoto->Link)) {
        if ((NodeOfGoto->FormId == LastFormId) && (NodeOfGoto->FormSetId == &LastNodeOfFormSet->FormSet->Guid)) {
          Status = EFI_SUCCESS;
          goto Done;
        }
      }
    }
    if (AttributeQuestion->RootFormSetId == &LastNodeOfFormSet->FormSet->Guid) {
      if (AttributeQuestionIsFirstForm(&LastNodeOfFormSet->FormSet->Guid, LastFormId)) {
        Status = EFI_SUCCESS;
        goto Done;
      }
    }
  }
  OpCodeData = (EFI_IFR_OP_HEADER *) LastNodeOfForm->Form;
  while ((UINTN) (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) OpCodeData + OpCodeData->Length)) < (UINTN) LastNodeOfForm->Form + LastNodeOfForm->FormSize) {
    if (CompareGuid(AttributeQuestion->ThisFormSetId, &LastNodeOfFormSet->FormSet->Guid)) {
      //
      // search question or goto form
      //
      if (AttributeQuestionIsOp(OpCodeData->OpCode)) {
        //
        // for each attribute question
        //
        Question = (EFI_IFR_QUESTION_HEADER *) &((EFI_IFR_REF5 *) OpCodeData)->Question;
        if (AttributeQuestion->QuestionId == Question->QuestionId) {
          Status = EFI_SUCCESS;
          //
          // insert mGotoTmpList to attribute question goto list
          //
          BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mGotoTmpList) {
            NodeOfGoto = AllocateCopyPool(sizeof(AQ_GOTO), CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE));
            if (!NodeOfGoto) {
              Status = EFI_OUT_OF_RESOURCES;
              break;
            }
            InsertTailList(&AttributeQuestion->GotoList, &NodeOfGoto->Link);
          }
          goto Done;
        }
      } else if (OpCodeData->OpCode == EFI_IFR_REF_OP) {
        if (OpCodeData->Length < sizeof(EFI_IFR_REF3)) {
          NextGotoHandle    = LastNodeOfFormSet->HiiHandle;
          NextGotoPrompt    = ((EFI_IFR_REF *) OpCodeData)->Question.Header.Prompt;
          Status = AttributeQuestionGoThruForm(AttributeQuestion, NextGotoHandle, NextGotoPrompt, LastNodeOfFormSet, ((EFI_IFR_REF *) OpCodeData)->FormId);
          //
          // pop last NodeOfGoto
          //
          Link = mGotoTmpList.BackLink;
          if (IsNodeAtEnd(&mGotoTmpList, Link)) {
            NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
            RemoveEntryList(&NodeOfGoto->Link);
            FreePool(NodeOfGoto);
          }
          if (Status == EFI_OUT_OF_RESOURCES) {
            goto Done;
          }
        }
      }
    } else {
      //
      // just goto formset
      //
      if (OpCodeData->OpCode == EFI_IFR_REF_OP) {
        if (OpCodeData->Length >= sizeof(EFI_IFR_REF3)) {
          NextGotoHandle    = LastNodeOfFormSet->HiiHandle;
          NextGotoPrompt    = ((EFI_IFR_REF *) OpCodeData)->Question.Header.Prompt;
          NextNodeOfFormSet = AttributeQuestionGetFormSet(NULL, &((EFI_IFR_REF3 *) OpCodeData)->FormSetId);
          Status = AttributeQuestionGoThruForm(AttributeQuestion, NextGotoHandle, NextGotoPrompt, NextNodeOfFormSet, 0);
          //
          // pop last NodeOfGoto
          //
          Link = mGotoTmpList.BackLink;
          if (IsNodeAtEnd(&mGotoTmpList, Link)) {
            NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
            RemoveEntryList(&NodeOfGoto->Link);
            FreePool(NodeOfGoto);
          }
          if (Status == EFI_OUT_OF_RESOURCES) {
            goto Done;
          }
        }
      }
    }
  }
  //
  // EFI_NOT_FOUND as EFI_SUCCESS
  //
  Status = EFI_SUCCESS;

Done:
  if (Status == EFI_OUT_OF_RESOURCES) {
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &AttributeQuestion->GotoList) {
      NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
      RemoveEntryList(&NodeOfGoto->Link);
      FreePool(NodeOfGoto);
    }

    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mGotoTmpList) {
      NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
      RemoveEntryList(&NodeOfGoto->Link);
      FreePool(NodeOfGoto);
    }
  }
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

VOID
DebugAttributeQuestion (
  AQ_NODE                               *AttributeQuestion
) {
  DEBUG ((DEBUG_INFO, "@AQ_NODE:\n"));
  DEBUG ((DEBUG_INFO,
    "  RootHiiHandle: %08p\n"
    "  RootFormSetId: %g\n"
    "  ThisHiiHandle: %08p\n"
    "  ThisFormSetId: %g\n"
    "  RootFormId   : %04x\n"
    "  QuestionId   : %04x\n",
    AttributeQuestion->RootHiiHandle, AttributeQuestion->RootFormSetId,
    AttributeQuestion->ThisHiiHandle, AttributeQuestion->ThisFormSetId,
    AttributeQuestion->RootFormId,    AttributeQuestion->QuestionId
    ));
  DEBUG ((DEBUG_INFO,
    "  Title[0]     : %s\n"
    "  Title[1]     : %s\n"
    "  Prompt       : %s\n",
    HiiGetString(AttributeQuestion->RootHiiHandle, AttributeQuestion->Title[0], NULL),
    HiiGetString(AttributeQuestion->RootHiiHandle, AttributeQuestion->Title[1], NULL),
    HiiGetString(AttributeQuestion->RootHiiHandle, AttributeQuestion->Prompt  , NULL)
    ));
}

EFI_STATUS
AttributeQuestionInit (
  IN     EFI_HII_HANDLE                 HiiHandle,
  IN OUT EFI_GUID                       *FormSetId
) {
  //
  // for one formset
  //
  EFI_STATUS                            Status;
  FORMSET_NODE                          *NodeOfFormSet;
  AQ_NODE                               *AttributeQuestion;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  DEBUG ((DEBUG_INFO, "%a().+\n", __FUNCTION__));
  //
  // free before next work
  //
  AttributeQuestionFree();
  //
  // Load Formset 1st
  //
  Status = AttributeQuestionFormSetRegisterAll();
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  NodeOfFormSet = AttributeQuestionGetFormSet(HiiHandle, FormSetId);
  if (!NodeOfFormSet) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }
  //
  // 1st stage init
  //
  Status = AttributeQuestionListInit(NodeOfFormSet, NULL, NULL);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // 2nd stage init
  //
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &gAttributeQuestionList) {
    AttributeQuestion = CR(Link, AQ_NODE, Link, AQ_NODE_SIGNATURE);
    DebugAttributeQuestion(AttributeQuestion);
    Status = AttributeQuestionGoThruForm(AttributeQuestion, NULL, 0, NULL, 0);
    if (EFI_ERROR (Status) && Status != EFI_UNSUPPORTED) {
      goto Done;
    }
    AttributeQuestionPrint(AttributeQuestion);
  }

  Status = EFI_SUCCESS;
Done:
  if (EFI_ERROR (Status)) {
    AttributeQuestionFree();
  }
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

AQ_NODE *
AttributeQuestionGetSelf (
  IN      CONST EFI_GUID                *RootFormSetId,
  IN      CONST EFI_GUID                *ThisFormSetId,
  IN      CONST UINT16                  QuestionId
) {
  AQ_NODE                               *AttributeQuestion = NULL;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &gAttributeQuestionList) {
    AttributeQuestion = CR(Link, AQ_NODE, Link, AQ_NODE_SIGNATURE);
    if ((AttributeQuestion->QuestionId == QuestionId) &&
        CompareGuid(AttributeQuestion->RootFormSetId, RootFormSetId) &&
        CompareGuid(AttributeQuestion->ThisFormSetId, ThisFormSetId)) {
      break;
    }
  }
  return AttributeQuestion;
}

VOID
AttributeQuestionFree (
  VOID
) {
  AQ_NODE                               *AttributeQuestion;
  LIST_ENTRY                            *Link0;
  LIST_ENTRY                            *NextLink0;
  LIST_ENTRY                            *Link1;
  LIST_ENTRY                            *NextLink1;
  AQ_GOTO                               *NodeOfGoto;
  //
  // free mGotoTmpList
  //
  BASE_LIST_FOR_EACH_SAFE(Link0, NextLink0, &mGotoTmpList) {
    NodeOfGoto = CR(Link0, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
    RemoveEntryList(&NodeOfGoto->Link);
    FreePool(NodeOfGoto);
  }
  //
  // free gAttributeQuestionList
  //
  BASE_LIST_FOR_EACH_SAFE(Link0, NextLink0, &gAttributeQuestionList) {
    AttributeQuestion = CR(Link0, AQ_NODE, Link, AQ_NODE_SIGNATURE);
    BASE_LIST_FOR_EACH_SAFE(Link1, NextLink1, &AttributeQuestion->GotoList) {
      NodeOfGoto = CR(Link1, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
      RemoveEntryList(&NodeOfGoto->Link);
      FreePool(NodeOfGoto);
    }
    RemoveEntryList(&AttributeQuestion->Link);
    FreePool(AttributeQuestion);
  }
}
