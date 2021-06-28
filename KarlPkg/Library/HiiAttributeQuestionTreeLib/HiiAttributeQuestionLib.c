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
LIST_ENTRY    mFormRefList            = INITIALIZE_LIST_HEAD_VARIABLE(mFormRefList);
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
  IN     EFI_GUID                      *FormSetId,
  IN     BOOLEAN                        ResetGoto
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

  DEBUG ((DEBUG_THIS, "%a().+\n", __FUNCTION__));
  Package     = NULL;
  OpCodeData  = NULL;
  //
  // get HII PackageList
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
  //
  // update formset depex
  //
  Status = AttributeQuestionFormSetRegisterDepex(&NodeOfFormSet->FormSet->Guid, 0, ResetGoto);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = EFI_SUCCESS;
Done:
  DEBUG ((DEBUG_THIS, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
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
    Status = AttributeQuestionFormSetRegister(HiiHandles[Index], NULL, TRUE);
    DEBUG ((DEBUG_THIS, "%a(): Index = %02d, Status = %r\n", __FUNCTION__, Index, Status));
  }

  if (EFI_ERROR(Status)) {
    AttributeQuestionFormSetFree();
  }
  return Status;
}

EFI_STATUS
AttributeQuestionFormSetRegisterDepex (
  IN            EFI_GUID               *FormSetId,
  IN            EFI_FORM_ID             FormId,
  IN            BOOLEAN                 ResetGoto
) {
  EFI_STATUS                            Status;
  FORMSET_NODE                          *NodeOfFormSet;
  FORM_NODE                             *NodeOfForm;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  EFI_HII_HANDLE                        *HiiHandles;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;
  FORMREF_NODE                          *NodeOfGoto;

  DEBUG ((DEBUG_THIS, "%a().+\n", __FUNCTION__));
  if (ResetGoto) {
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormRefList) {
      NodeOfGoto = CR(Link, FORMREF_NODE, Link, FORMREF_NODE_SIGNATURE);
      RemoveEntryList(&NodeOfGoto->Link);
      FreePool(NodeOfGoto);
    }
  }

  NodeOfFormSet = AttributeQuestionGetFormSet(FormSetId);
  if (!NodeOfFormSet) {
    Status = EFI_NOT_READY;
    goto Done;
  }
  if (!FormSetId) {
    FormSetId = &NodeOfFormSet->FormSet->Guid;
  }
  NodeOfForm = AttributeQuestionGetForm(FormSetId, FormId);
  if (!NodeOfForm) {
    Status = EFI_NOT_READY;
    goto Done;
  }
  if (!FormId) {
    FormId = NodeOfForm->Form->FormId;
  }
  //
  // in case of goto each other, skip done one
  //
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormRefList) {
    NodeOfGoto = CR(Link, FORMREF_NODE, Link, FORMREF_NODE_SIGNATURE);
    if (CompareGuid(FormSetId, NodeOfGoto->FormSetId) && (FormId == NodeOfGoto->FormId) ) {
      Status = EFI_SUCCESS;
      goto Done;
    }
  }
  //
  // always insert this to goto list
  //
  NodeOfGoto = AllocateZeroPool(sizeof(FORMREF_NODE));
  if (!NodeOfGoto) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  InsertTailList(&mFormRefList, &NodeOfGoto->Link);
  NodeOfGoto->Signature = FORMREF_NODE_SIGNATURE;
  NodeOfGoto->FormSetId = FormSetId;
  NodeOfGoto->FormId    = FormId;
  //
  // for each attribute question in this form
  //
  OpCodeData = (EFI_IFR_OP_HEADER *) NodeOfForm->Form;
  while ((UINTN) (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) OpCodeData + OpCodeData->Length)) < (UINTN) NodeOfForm->Form + NodeOfForm->FormSize) {
    if (OpCodeData->OpCode == EFI_IFR_REF_OP) {
      if (OpCodeData->Length >= sizeof(EFI_IFR_REF3)) {
        HiiHandles = HiiGetHiiHandles(&((EFI_IFR_REF3*) OpCodeData)->FormSetId);
        if (!HiiHandles) {
          Status = EFI_NOT_FOUND;
          goto Done;
        }
        Status = AttributeQuestionFormSetRegister(HiiHandles[0], &((EFI_IFR_REF3*) OpCodeData)->FormSetId, FALSE);
      }
      if (EFI_ERROR(Status)) {
        goto Done;
      }
    }
  }

  Status = EFI_SUCCESS;
Done:
  if (Status == EFI_OUT_OF_RESOURCES) {
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormSetList) {
      NodeOfFormSet = CR(Link, FORMSET_NODE, Link, FORMSET_NODE_SIGNATURE);
      RemoveEntryList(&NodeOfFormSet->Link);
      FreePool(NodeOfFormSet);
    }
  }
  DEBUG ((DEBUG_THIS, "%a().- Status = %r\n", __FUNCTION__, Status));
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

FORMSET_NODE *
AttributeQuestionGetFormSet (
  IN      CONST EFI_GUID                *FormSetId
) {
  FORMSET_NODE                          *NodeOfFormSet = NULL;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  if (!FormSetId) {
    if (IsListEmpty(&mFormSetList)) {
      return NULL;
    } else {
      return CR(mFormSetList.ForwardLink, FORMSET_NODE, Link, FORMSET_NODE_SIGNATURE);
    }
  }

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormSetList) {
    NodeOfFormSet = CR(Link, FORMSET_NODE, Link, FORMSET_NODE_SIGNATURE);
    if (CompareGuid(FormSetId, &NodeOfFormSet->FormSet->Guid)) {
      break;
    }
  }
  return NodeOfFormSet;
}

EFI_HII_HANDLE
AttributeQuestionGetHiiHandle (
  IN      CONST EFI_GUID                *FormSetId
) {
  EFI_HII_HANDLE                        HiiHandle      = NULL;
  FORMSET_NODE                          *NodeOfFormSet = NULL;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormSetList) {
    NodeOfFormSet = CR(Link, FORMSET_NODE, Link, FORMSET_NODE_SIGNATURE);
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
  DEBUG ((DEBUG_THIS, "%a().+\n", __FUNCTION__));
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
  DEBUG ((DEBUG_THIS, "%a().- Status = %r\n", __FUNCTION__, Status));
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

  NodeOfFormSet = AttributeQuestionGetFormSet(FormSetId);
  if (!NodeOfFormSet) {
    goto Done;
  }

  if (!FormId) {
    if (IsListEmpty(&NodeOfFormSet->FormList)) {
      return NULL;
    } else {
      return CR(NodeOfFormSet->FormList.ForwardLink, FORM_NODE, Link, FORM_NODE_SIGNATURE);
    }
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

EFI_STATUS
AttributeQuestionListInit (
  IN            EFI_GUID               *FormSetId,
  IN            EFI_FORM_ID             FormId,
  IN            AQ_PAGE                *PrevAttributeQuestionPage,
  IN            BOOLEAN                 ResetGoto
) {
  EFI_STATUS                            Status;
  FORMSET_NODE                          *NodeOfFormSet;
  FORM_NODE                             *NodeOfForm;
  EFI_IFR_QUESTION_HEADER               *Question;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  AQ_NODE                               *AttributeQuestion;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;
  FORMREF_NODE                          *NodeOfGoto;

  DEBUG ((DEBUG_THIS, "%a().+\n", __FUNCTION__));
  if (ResetGoto) {
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormRefList) {
      NodeOfGoto = CR(Link, FORMREF_NODE, Link, FORMREF_NODE_SIGNATURE);
      RemoveEntryList(&NodeOfGoto->Link);
      FreePool(NodeOfGoto);
    }
  }

  NodeOfFormSet = AttributeQuestionGetFormSet(FormSetId);
  if (!NodeOfFormSet) {
    Status = EFI_NOT_READY;
    goto Done;
  }
  if (!FormSetId) {
    FormSetId = &NodeOfFormSet->FormSet->Guid;
  }
  NodeOfForm = AttributeQuestionGetForm(FormSetId, FormId);
  if (!NodeOfForm) {
    Status = EFI_NOT_READY;
    goto Done;
  }
  if (!FormId) {
    FormId = NodeOfForm->Form->FormId;
  }
  // <<
  DEBUG((DEBUG_THIS,
    "ThisPage\n"
    "  .FormSetId     : %g\n"
    "  .FormId        : %04x\n"
    "  .FormSetTitle  : %s\n"
    "  .FormTitle     : %s\n",
    FormSetId, FormId,
    HiiGetString(NodeOfFormSet->HiiHandle, NodeOfFormSet->FormSet->FormSetTitle, NULL),
    HiiGetString(NodeOfFormSet->HiiHandle, NodeOfForm->Form->FormTitle, NULL)
    ));
  if (PrevAttributeQuestionPage) {
  DEBUG((DEBUG_THIS,
    "PrevPage\n"
    "  .FormSetId     : %g\n"
    "  .FormId        : %04x\n"
    "  .FormSetTitle  : %s\n"
    "  .FormTitle     : %s\n",
    PrevAttributeQuestionPage->FormSetId, PrevAttributeQuestionPage->FormId,
    HiiGetString(PrevAttributeQuestionPage->HiiHandle, PrevAttributeQuestionPage->FormSetTitle, NULL),
    HiiGetString(PrevAttributeQuestionPage->HiiHandle, PrevAttributeQuestionPage->FormTitle   , NULL)
    ));
  }
  // >>

  //
  // always insert this to goto list
  //
  NodeOfGoto = AllocateZeroPool(sizeof(FORMREF_NODE));
  if (!NodeOfGoto) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  InsertTailList(&mFormRefList, &NodeOfGoto->Link);
  NodeOfGoto->Signature = FORMREF_NODE_SIGNATURE;
  NodeOfGoto->FormSetId = FormSetId;
  NodeOfGoto->FormId    = FormId;
  //
  // in case of goto each other, skip gone one
  //
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormRefList) {
    NodeOfGoto = CR(Link, FORMREF_NODE, Link, FORMREF_NODE_SIGNATURE);
    if (!IsNodeAtEnd(&mFormRefList, &NodeOfGoto->Link)) {
      if (CompareGuid(FormSetId, NodeOfGoto->FormSetId) && (FormId == NodeOfGoto->FormId)) {
        Status = EFI_SUCCESS;
        goto Done;
      }
    }
  }
  //
  // for each attribute question in this form
  //
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
      AttributeQuestion->Signature              = AQ_NODE_SIGNATURE;
      AttributeQuestion->ThisPage.HiiHandle     = NodeOfFormSet->HiiHandle;
      AttributeQuestion->ThisPage.FormSetId     = FormSetId;
      AttributeQuestion->ThisPage.FormSetTitle  = NodeOfFormSet->FormSet->FormSetTitle;
      AttributeQuestion->ThisPage.FormId        = NodeOfForm->Form->FormId;
      AttributeQuestion->ThisPage.FormTitle     = NodeOfForm->Form->FormTitle;
      AttributeQuestion->PrevPage               = PrevAttributeQuestionPage;
      AttributeQuestion->Type                   = AQ_QUESTION;
      AttributeQuestion->QuestionId             = Question->QuestionId;
      AttributeQuestion->Prompt                 = Question->Header.Prompt;
    } else if (OpCodeData->OpCode == EFI_IFR_REF_OP) {
      //
      // create AQ_NODE
      //
      AttributeQuestion = AllocateZeroPool(sizeof(AQ_NODE));
      if (!AttributeQuestion) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }
      InsertTailList(&gAttributeQuestionList, &AttributeQuestion->Link);
      AttributeQuestion->Signature              = AQ_NODE_SIGNATURE;
      AttributeQuestion->ThisPage.HiiHandle     = NodeOfFormSet->HiiHandle;
      AttributeQuestion->ThisPage.FormSetId     = FormSetId;
      AttributeQuestion->ThisPage.FormSetTitle  = NodeOfFormSet->FormSet->FormSetTitle;
      AttributeQuestion->ThisPage.FormId        = FormId;
      AttributeQuestion->ThisPage.FormTitle     = NodeOfForm->Form->FormTitle;
      AttributeQuestion->PrevPage               = PrevAttributeQuestionPage;
      AttributeQuestion->QuestionId             = ((EFI_IFR_REF*) OpCodeData)->Question.QuestionId;
      AttributeQuestion->Prompt                 = ((EFI_IFR_REF*) OpCodeData)->Question.Header.Prompt;

      if (OpCodeData->Length >= sizeof(EFI_IFR_REF3)) {
        AttributeQuestion->Type = AQ_REF_FSET;
        //
        // goto next formset
        //
        FormSetId = &((EFI_IFR_REF3 *) OpCodeData)->FormSetId;
        FormId    = 0;
      } else {
        AttributeQuestion->Type = AQ_REF_FORM;
        //
        // goto next form
        //
        FormId    = ((EFI_IFR_REF *) OpCodeData)->FormId;
      }
      Status = AttributeQuestionListInit(FormSetId, FormId, &AttributeQuestion->ThisPage, FALSE);
      //
      // pop last NodeOfGoto
      //
      if (Status != EFI_NOT_READY) {
        if (!IsListEmpty(&mFormRefList)) {
          NodeOfGoto = CR(mFormRefList.BackLink, FORMREF_NODE, Link, FORMREF_NODE_SIGNATURE);
          RemoveEntryList(&NodeOfGoto->Link);
          FreePool(NodeOfGoto);
        }
      }
      if (EFI_ERROR(Status)) {
        goto Done;
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
  DEBUG ((DEBUG_THIS, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

AQ_NODE *
AttributeQuestionGetSelf (
  IN      CONST EFI_GUID                *FormSetId,
  IN      CONST UINT16                  QuestionId
) {
  AQ_NODE                               *AttributeQuestion = NULL;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &gAttributeQuestionList) {
    AttributeQuestion = CR(Link, AQ_NODE, Link, AQ_NODE_SIGNATURE);
    if (CompareGuid(FormSetId, AttributeQuestion->ThisPage.FormSetId) && (QuestionId == AttributeQuestion->QuestionId)) {
      break;
    }
  }
  return AttributeQuestion;
}

AQ_NODE *
AttributeQuestionGetPrev (
  IN      CONST AQ_NODE                 *ThisAttributeQuestion
) {
  AQ_NODE                               *PrevAttributeQuestion;

  if (ThisAttributeQuestion->PrevPage) {
    PrevAttributeQuestion = CR(ThisAttributeQuestion->PrevPage, AQ_NODE, ThisPage, AQ_NODE_SIGNATURE);
  } else {
    PrevAttributeQuestion = NULL;
  }

  return PrevAttributeQuestion;
}

CHAR16 *
AttributeQuestionGetUrl (
  IN      CONST EFI_GUID                *FormSetId,
  IN      CONST UINT16                  QuestionId
) {
  EFI_STATUS                            Status;
  CHAR16                                *Result                = NULL;
  AQ_NODE                               *ThisAttributeQuestion = NULL;
  AQ_NODE                               *PrevAttributeQuestion = NULL;
  CHAR16                                Url[QUESTION_URL_LENGTH];
  LIST_ENTRY                            UrlNodeList;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;
  URL_NODE                              *NodeOfUrl;

  DEBUG ((DEBUG_THIS, "%a().+\n", __FUNCTION__));
  InitializeListHead(&UrlNodeList);

  ThisAttributeQuestion = AttributeQuestionGetSelf(FormSetId, QuestionId);
  if (!ThisAttributeQuestion) {
    Status = EFI_NOT_READY;
    goto Done;
  }
  if (ThisAttributeQuestion->Type != AQ_QUESTION) {
    goto Done;
  }

  while ((PrevAttributeQuestion = AttributeQuestionGetPrev(ThisAttributeQuestion)) != NULL) {
    NodeOfUrl = AllocateZeroPool(sizeof(URL_NODE));
    if (!NodeOfUrl) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }
    InsertHeadList(&UrlNodeList, &NodeOfUrl->Link);
    NodeOfUrl->Signature = URL_NODE_SIGNATURE;
    NodeOfUrl->HiiHandle = ThisAttributeQuestion->ThisPage.HiiHandle;
    NodeOfUrl->Name = ThisAttributeQuestion->Prompt;

    ThisAttributeQuestion = PrevAttributeQuestion;
  }
  NodeOfUrl = AllocateZeroPool(sizeof(URL_NODE));
  if (!NodeOfUrl) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  InsertHeadList(&UrlNodeList, &NodeOfUrl->Link);
  NodeOfUrl->Signature = URL_NODE_SIGNATURE;
  NodeOfUrl->HiiHandle = ThisAttributeQuestion->ThisPage.HiiHandle;
  NodeOfUrl->Name = ThisAttributeQuestion->Prompt;

  NodeOfUrl = AllocateZeroPool(sizeof(URL_NODE));
  if (!NodeOfUrl) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  InsertHeadList(&UrlNodeList, &NodeOfUrl->Link);
  NodeOfUrl->Signature = URL_NODE_SIGNATURE;
  NodeOfUrl->HiiHandle = ThisAttributeQuestion->ThisPage.HiiHandle;
  NodeOfUrl->Name = ThisAttributeQuestion->ThisPage.FormTitle;

  NodeOfUrl = AllocateZeroPool(sizeof(URL_NODE));
  if (!NodeOfUrl) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  InsertHeadList(&UrlNodeList, &NodeOfUrl->Link);
  NodeOfUrl->Signature = URL_NODE_SIGNATURE;
  NodeOfUrl->HiiHandle = ThisAttributeQuestion->ThisPage.HiiHandle;
  NodeOfUrl->Name = ThisAttributeQuestion->ThisPage.FormSetTitle;

  ZeroMem(Url, sizeof(Url));
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &UrlNodeList) {
    NodeOfUrl = CR(Link, URL_NODE, Link, URL_NODE_SIGNATURE);
    StrCatS(Url, QUESTION_URL_LENGTH, QUESTION_URL_SLASH);
    StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(NodeOfUrl->HiiHandle, NodeOfUrl->Name, NULL));
  }

  Result = AllocateCopyPool(StrnSizeS(Url, QUESTION_URL_LENGTH), Url);
  if (!Result) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  DEBUG((DEBUG_INFO, "Url: %s\n", Result));
  Status = EFI_SUCCESS;
Done:
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &UrlNodeList) {
    NodeOfUrl = CR(Link, URL_NODE, Link, URL_NODE_SIGNATURE);
    RemoveEntryList(&NodeOfUrl->Link);
    FreePool(NodeOfUrl);
  }
  DEBUG ((DEBUG_THIS, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Result;
}

VOID
DebugAttributeQuestion (
  AQ_NODE                               *AttributeQuestion
) {
  DEBUG ((DEBUG_THIS, "@AQ_NODE:\n"));
  DEBUG ((DEBUG_THIS,
    "  Type           : %u\n"
    "  QuestionId     : %04x\n"
    "  Prompt         : %s\n"
    "  PrevPage       : %p\n",
    AttributeQuestion->Type, AttributeQuestion->QuestionId,
    HiiGetString(AttributeQuestion->ThisPage.HiiHandle, AttributeQuestion->Prompt, NULL),
    AttributeQuestion->PrevPage
    ));

  DEBUG ((DEBUG_THIS,
    "ThisPage\n"
    "  .FormSetId     : %g\n"
    "  .FormId        : %04x\n"
    "  .FormSetTitle  : %s\n"
    "  .FormTitle     : %s\n",
    AttributeQuestion->ThisPage.FormSetId, AttributeQuestion->ThisPage.FormId,
    HiiGetString(AttributeQuestion->ThisPage.HiiHandle, AttributeQuestion->ThisPage.FormSetTitle  , NULL),
    HiiGetString(AttributeQuestion->ThisPage.HiiHandle, AttributeQuestion->ThisPage.FormTitle     , NULL)
    ));

  if (AttributeQuestion->PrevPage) {
  DEBUG ((DEBUG_THIS,
    "PrevPage\n"
    "  .FormSetId     : %g\n"
    "  .FormId        : %04x\n"
    "  .FormSetTitle  : %s\n"
    "  .FormTitle     : %s\n",
    AttributeQuestion->PrevPage->FormSetId, AttributeQuestion->PrevPage->FormId,
    HiiGetString(AttributeQuestion->PrevPage->HiiHandle, AttributeQuestion->PrevPage->FormSetTitle, NULL),
    HiiGetString(AttributeQuestion->PrevPage->HiiHandle, AttributeQuestion->PrevPage->FormTitle   , NULL)
    ));
  }
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
  AQ_NODE                               *AttributeQuestion;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  DEBUG ((DEBUG_INFO, "%a().+\n", __FUNCTION__));
  //
  // free before next work
  //
  AttributeQuestionFree();
  //
  // init formset database
  //
  Status = AttributeQuestionFormSetRegister(HiiHandle, FormSetId, TRUE);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // attribute question list init
  //
  Status = AttributeQuestionListInit(FormSetId, 0, NULL, TRUE);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &gAttributeQuestionList) {
    AttributeQuestion = CR(Link, AQ_NODE, Link, AQ_NODE_SIGNATURE);
    DebugAttributeQuestion(AttributeQuestion);
    EFI_STRING Url;
    Url = AttributeQuestionGetUrl(AttributeQuestion->ThisPage.FormSetId, AttributeQuestion->QuestionId);
    if (Url) {
      FreePool(Url);
    }
  }

  Status = EFI_SUCCESS;
Done:
  if (EFI_ERROR (Status)) {
    AttributeQuestionFree();
  }
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

VOID
AttributeQuestionFree (
  VOID
) {
  AQ_NODE                               *AttributeQuestion;
  FORMREF_NODE                          *NodeOfGoto;
  LIST_ENTRY                            *Link0;
  LIST_ENTRY                            *NextLink0;
  //
  // free mFormRefList
  //
  BASE_LIST_FOR_EACH_SAFE(Link0, NextLink0, &mFormRefList) {
    NodeOfGoto = CR(Link0, FORMREF_NODE, Link, FORMREF_NODE_SIGNATURE);
    RemoveEntryList(&NodeOfGoto->Link);
    FreePool(NodeOfGoto);
  }
  //
  // free gAttributeQuestionList
  //
  BASE_LIST_FOR_EACH_SAFE(Link0, NextLink0, &gAttributeQuestionList) {
    AttributeQuestion = CR(Link0, AQ_NODE, Link, AQ_NODE_SIGNATURE);
    RemoveEntryList(&AttributeQuestion->Link);
    FreePool(AttributeQuestion);
  }
}
