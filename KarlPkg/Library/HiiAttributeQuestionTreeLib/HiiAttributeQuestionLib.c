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

LIST_ENTRY    gAttributeQuestionList  = INITIALIZE_LIST_HEAD_VARIABLE(gAttributeQuestionList);
LIST_ENTRY    mFormList               = INITIALIZE_LIST_HEAD_VARIABLE(mFormList);
LIST_ENTRY    mGotoList               = INITIALIZE_LIST_HEAD_VARIABLE(mGotoList);

EFI_IFR_FORM_SET    *mFormSet         = NULL;
UINTN                mFormSetSize     = 0;

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
AttributeQuestionLoadFormSet (
  IN     EFI_HII_HANDLE                 Handle,
  IN OUT EFI_GUID                       *FormSetGuid
) {
  EFI_STATUS                            Status;
  EFI_HII_PACKAGE_LIST_HEADER           *PackageList;
  UINTN                                 PackageListSize;
  EFI_HII_PACKAGE_HEADER                *Package;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  UINT8                                 Index;
  EFI_GUID                              *ClassGuid;
  UINT8                                 ClassGuidCount;
  BOOLEAN                               ClassGuidMatch;
  BOOLEAN                               FormSetFound;

  DEBUG ((DEBUG_VERBOSE, "%a().+\n", __FUNCTION__));
  Package     = NULL;
  OpCodeData  = NULL;
  //
  // Get HII PackageList
  //
  PackageList     = NULL;
  PackageListSize = 0;
  Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, Handle, &PackageListSize, PackageList);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    PackageList = AllocateZeroPool (PackageListSize);
    ASSERT (PackageList != NULL);
    Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, Handle, &PackageListSize, PackageList);
  }
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  ASSERT (PackageList != NULL);
  //
  // get form package from this hii package list
  //
  while ((UINTN) (Package = (EFI_HII_PACKAGE_HEADER *) ((UINTN) PackageList + sizeof (EFI_HII_PACKAGE_LIST_HEADER))) < (UINTN) PackageList + PackageListSize) {
    if (Package->Type != EFI_HII_PACKAGE_FORMS) {
      continue;
    }
    FormSetFound = FALSE;
    //
    // search formset in this form package
    //
    while ((UINTN) (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) Package + sizeof (EFI_HII_PACKAGE_HEADER))) < (UINTN) Package + Package->Length) {
      if (OpCodeData->OpCode != EFI_IFR_FORM_SET_OP) {
        continue;
      }
      if (FormSetGuid == NULL) {
        FormSetFound = TRUE;
        break;
      } else if (CompareGuid (FormSetGuid, &((EFI_IFR_FORM_SET *) OpCodeData)->Guid)) {
        FormSetFound = TRUE;
        break;
      } else if (((EFI_IFR_OP_HEADER *) OpCodeData)->Length > sizeof (EFI_IFR_FORM_SET)) {
        ClassGuid       = (EFI_GUID *) (OpCodeData + sizeof (EFI_IFR_FORM_SET));
        ClassGuidCount  = (UINT8) (((EFI_IFR_FORM_SET *) OpCodeData)->Flags & 0x3);
        ClassGuidMatch  = FALSE;
        for (Index = 0; Index < ClassGuidCount; Index++) {
          if (CompareGuid (FormSetGuid, ClassGuid + Index)) {
            ClassGuidMatch = TRUE;
            break;
          }
        }
        if (ClassGuidMatch) {
          FormSetFound = TRUE;
          break;
        }
      } else if (FormSetGuid == &gEfiHiiPlatformSetupFormsetGuid) {
        FormSetFound = TRUE;
        break;
      }
    }
    if (FormSetFound) {
      break;
    }
  }
  FreePool (PackageList);

  if (FormSetGuid != NULL) {
    CopyMem (FormSetGuid, &((EFI_IFR_FORM_SET *) OpCodeData)->Guid, sizeof (EFI_GUID));
  }
  mFormSetSize = Package->Length - sizeof(EFI_HII_PACKAGE_HEADER);
  mFormSet     = AllocateCopyPool (mFormSetSize, OpCodeData);
  if (mFormSet == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
  } else {
    Status = EFI_SUCCESS;
  }

Done:
  DEBUG ((DEBUG_VERBOSE, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

EFI_STATUS
AttributeQuestionFormListInit (
  VOID
) {
  //
  // for one formset
  //
  EFI_STATUS                            Status;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  EFI_IFR_FORM                          *Form;
  UINTN                                 InTheForm;
  FORM_NODE                             *NodeOfForm;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;
  AQ_GOTO                               *NodeOfGoto;

  DEBUG ((DEBUG_VERBOSE, "%a().+\n", __FUNCTION__));
  //
  // reset mFormList for new form list
  //
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormList) {
    NodeOfForm = CR(Link, FORM_NODE, Link, FORM_NODE_SIGNATURE);
    RemoveEntryList(&NodeOfForm->Link);
    FreePool(NodeOfForm);
  }
  //
  // reset mGotoList for new form list
  //
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mGotoList) {
    NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
    RemoveEntryList(&NodeOfGoto->Link);
    FreePool(NodeOfGoto);
  }
  //
  // for each formset
  //
  OpCodeData = (EFI_IFR_OP_HEADER *) mFormSet;
  while ((UINTN) (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) OpCodeData + OpCodeData->Length)) < (UINTN) mFormSet + mFormSetSize) {
    if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode != EFI_IFR_FORM_OP) {
      continue;
    }
    Form = (EFI_IFR_FORM *) OpCodeData;
    InTheForm=1;
    while ((UINTN) (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) OpCodeData + OpCodeData->Length)) < (UINTN) mFormSet + mFormSetSize) {
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
    InsertTailList(&mFormList, &NodeOfForm->Link);
    NodeOfForm->Signature = FORM_NODE_SIGNATURE;
    NodeOfForm->Form      = Form;
    NodeOfForm->FormSize  = (UINTN) OpCodeData + OpCodeData->Length - (UINTN) Form;
  }

  Status = EFI_SUCCESS;
Done:
  if (EFI_ERROR (Status)) {
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormList) {
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
  IN      CONST UINT16                  FormId
) {
  FORM_NODE                             *NodeOfForm = NULL;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormList) {
    NodeOfForm = CR(Link, FORM_NODE, Link, FORM_NODE_SIGNATURE);
    if (NodeOfForm->Form->FormId == FormId) {
      break;
    }
  }
  return NodeOfForm;
}

BOOLEAN
AttributeQuestionIsFirstForm (
  IN      CONST UINT16                  FormId
) {
  FORM_NODE                             *NodeOfForm;

  if (IsListEmpty(&mFormList)) {
    return TRUE;
  }
  NodeOfForm = CR(mFormList.ForwardLink, FORM_NODE, Link, FORM_NODE_SIGNATURE);
  if (FormId == NodeOfForm->Form->FormId) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
AttributeQuestionListInit (
  IN      CONST EFI_HII_HANDLE          HiiHandle,
  IN OUT EFI_GUID                       *FormSetGuid
) {
  EFI_STATUS                            Status;
  FORM_NODE                             *NodeOfForm;
  EFI_IFR_QUESTION_HEADER               *Question;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  AQ_NODE                               *AttributeQuestion;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;
  BOOLEAN                               FirstFormHasBeenGot;
  EFI_IFR_FORM                          *FirstForm;

  DEBUG ((DEBUG_VERBOSE, "%a().+\n", __FUNCTION__));

  Status = AttributeQuestionLoadFormSet(HiiHandle, FormSetGuid);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  Status = AttributeQuestionFormListInit();
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // reset gAttributeQuestionList for attribute question list
  //
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &gAttributeQuestionList) {
    AttributeQuestion = CR(Link, AQ_NODE, Link, AQ_NODE_SIGNATURE);
    RemoveEntryList(&AttributeQuestion->Link);
    FreePool(AttributeQuestion);
  }
  //
  // for each attribute question in this each form
  //
  FirstFormHasBeenGot = FALSE;
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mFormList) {
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
        AttributeQuestion->Signature  = AQ_NODE_SIGNATURE;
        AttributeQuestion->Handle     = HiiHandle;
        AttributeQuestion->Fid        = FirstForm->FormId;
        AttributeQuestion->Qid        = Question->QuestionId;
        AttributeQuestion->Title[0]   = mFormSet->FormSetTitle;
        AttributeQuestion->Title[1]   = FirstForm->FormTitle;
        AttributeQuestion->Prompt     = Question->Header.Prompt;
        InitializeListHead(&AttributeQuestion->GotoList);
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
    "  Id    : %04x\n"
    "  Title : %04x\n"
    "  Depth : %u\n",
    NodeOfGoto->FormId, NodeOfGoto->Title, NodeOfGoto->Depth
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
  StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(AttributeQuestion->Handle, AttributeQuestion->Title[0], NULL));
  StrCatS(Url, QUESTION_URL_LENGTH, QUESTION_URL_SLASH);
  StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(AttributeQuestion->Handle, AttributeQuestion->Title[1], NULL));
  StrCatS(Url, QUESTION_URL_LENGTH, QUESTION_URL_SLASH);
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &AttributeQuestion->GotoList) {
    NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
    StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(AttributeQuestion->Handle, NodeOfGoto->Title, NULL));
    StrCatS(Url, QUESTION_URL_LENGTH, QUESTION_URL_SLASH);
  }
  StrCatS(Url, QUESTION_URL_LENGTH, HiiGetString(AttributeQuestion->Handle, AttributeQuestion->Prompt, NULL));
  DEBUG((DEBUG_INFO, "Url: %s\n", Url));
}

EFI_STATUS
AttributeQuestionGoThruForm (
  IN      CONST UINT16                  QuestionId,
  IN      CONST UINT16                  FormId
) {
  EFI_STATUS                            Status;
  FORM_NODE                             *NodeOfForm;
  EFI_IFR_QUESTION_HEADER               *Question;
  EFI_IFR_OP_HEADER                     *OpCodeData;
  AQ_NODE                               *AttributeQuestion;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;
  AQ_GOTO                               *NodeOfGoto;

  DEBUG ((DEBUG_INFO, "%a().+ FormId = %04x\n", __FUNCTION__, FormId));

  AttributeQuestion = AttributeQuestionGetSelf(QuestionId);
  if (!AttributeQuestion) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (!FormId) {
    if (!IsListEmpty(&mFormList)) {
      NodeOfForm = CR(mFormList.ForwardLink, FORM_NODE, Link, FORM_NODE_SIGNATURE);
    } else {
      //
      // call AttributeQuestionListInit before AttributeQuestionGoThruForm
      //
      Status = EFI_NOT_READY;
      goto Done;
    }
  } else {
    NodeOfForm = AttributeQuestionGetForm(FormId);
    //
    // goto each other as success and return
    //
    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mGotoList) {
      NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
      if (FormId == NodeOfGoto->FormId) {
        Status = EFI_SUCCESS;
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
    InsertTailList(&mGotoList, &NodeOfGoto->Link);
    NodeOfGoto->Signature = AQ_GOTO_SIGNATURE;
    NodeOfGoto->FormId    = NodeOfForm->Form->FormId;
    NodeOfGoto->Title     = NodeOfForm->Form->FormTitle;
    if (IsNodeAtEnd(&mGotoList, mGotoList.ForwardLink)) {
      NodeOfGoto->Depth = 1;
    } else {
      NodeOfGoto->Depth = (CR(NodeOfGoto->Link.BackLink, AQ_GOTO, Link, AQ_GOTO_SIGNATURE))->Depth + 1;
    }
    DebugGotoNode(NodeOfGoto);
    //
    // skip first form after create mGotoList node
    //
    if (AttributeQuestionIsFirstForm(FormId)) {
      Status = EFI_SUCCESS;
      goto Done;
    }
  }
  OpCodeData = (EFI_IFR_OP_HEADER *) NodeOfForm->Form;
  while ((UINTN) (OpCodeData = (EFI_IFR_OP_HEADER *) ((UINTN) OpCodeData + OpCodeData->Length)) < (UINTN) NodeOfForm->Form + NodeOfForm->FormSize) {
    if (AttributeQuestionIsOp(OpCodeData->OpCode)) {
      //
      // for each attribute question
      //
      Question = (EFI_IFR_QUESTION_HEADER *) &((EFI_IFR_REF5 *) OpCodeData)->Question;
      if (QuestionId == Question->QuestionId) {
        Status = EFI_SUCCESS;
        //
        // insert mGotoList to attribute question goto list
        //
        BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mGotoList) {
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
      //
      // for goto form
      //
      Status = AttributeQuestionGoThruForm(QuestionId, ((EFI_IFR_REF *) OpCodeData)->FormId);
      Link   = mGotoList.BackLink;
      if (IsNodeAtEnd(&mGotoList, Link)) {
        NodeOfGoto = CR(Link, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
        RemoveEntryList(&NodeOfGoto->Link);
        FreePool(NodeOfGoto);
      }
      if (Status == EFI_OUT_OF_RESOURCES) {
        goto Done;
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

    BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &mGotoList) {
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
    "  Handle       : %08p\n"
    "  Fid          : %04x\n"
    "  Qid          : %04x\n"
    "  Title[0]     : %s\n"
    "  Title[1]     : %s\n"
    "  Prompt       : %s\n",
    AttributeQuestion->Handle, AttributeQuestion->Fid, AttributeQuestion->Qid,
    HiiGetString(AttributeQuestion->Handle, AttributeQuestion->Title[0], NULL),
    HiiGetString(AttributeQuestion->Handle, AttributeQuestion->Title[1], NULL),
    HiiGetString(AttributeQuestion->Handle, AttributeQuestion->Prompt  , NULL)
    ));
}

EFI_STATUS
AttributeQuestionInit (
  IN     EFI_HII_HANDLE                 HiiHandle,
  IN OUT EFI_GUID                       *FormSetGuid
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
  // 1st stage init
  //
  Status = AttributeQuestionListInit(HiiHandle, FormSetGuid);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // 2nd stage init
  //
  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &gAttributeQuestionList) {
    AttributeQuestion = CR(Link, AQ_NODE, Link, AQ_NODE_SIGNATURE);
    DebugAttributeQuestion(AttributeQuestion);
    Status = AttributeQuestionGoThruForm(AttributeQuestion->Qid, 0);
    AttributeQuestionPrint(AttributeQuestion);
  }

Done:
  if (Status == EFI_OUT_OF_RESOURCES) {
    AttributeQuestionFree();
  }
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

AQ_NODE *
AttributeQuestionGetSelf (
  IN      CONST UINT16                  QuestionId
) {
  AQ_NODE                               *AttributeQuestion = NULL;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *NextLink;

  BASE_LIST_FOR_EACH_SAFE(Link, NextLink, &gAttributeQuestionList) {
    AttributeQuestion = CR(Link, AQ_NODE, Link, AQ_NODE_SIGNATURE);
    if (AttributeQuestion->Qid == QuestionId) {
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
  FORM_NODE                             *NodeOfForm;
  AQ_GOTO                               *NodeOfGoto;
  //
  // free mGotoList
  //
  BASE_LIST_FOR_EACH_SAFE(Link0, NextLink0, &mGotoList) {
    NodeOfGoto = CR(Link0, AQ_GOTO, Link, AQ_GOTO_SIGNATURE);
    RemoveEntryList(&NodeOfGoto->Link);
    FreePool(NodeOfGoto);
  }
  //
  // free mFormList
  //
  BASE_LIST_FOR_EACH_SAFE(Link0, NextLink0, &mFormList) {
    NodeOfForm = CR(Link0, FORM_NODE, Link, FORM_NODE_SIGNATURE);
    RemoveEntryList(&NodeOfForm->Link);
    FreePool(NodeOfForm);
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
  //
  // free mFormSet
  //
  if (mFormSet) {
    FreePool(mFormSet);
    mFormSet  = NULL;
    mFormSetSize = 0;
  }
}
