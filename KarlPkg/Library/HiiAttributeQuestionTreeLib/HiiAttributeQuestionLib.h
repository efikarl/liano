/** @file
  Copyright ©2021 Liu Yi, efikarl@yeah.net

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
--*/
#ifndef _HII_ATTRIBUTE_QUESTION_H_
#define _HII_ATTRIBUTE_QUESTION_H_
#include <Uefi.h>

#define DEBUG_THIS                      DEBUG_VERBOSE

#define FORMSET_NODE_SIGNATURE          SIGNATURE_32 ('A', 'Q', 'F', 'S')
typedef struct {
  UINTN                                 Signature;
  EFI_HII_HANDLE                        HiiHandle;
  EFI_IFR_FORM_SET                      *FormSet;
  UINTN                                 FormSetSize;
  LIST_ENTRY                            Link;
  LIST_ENTRY                            FormList;
} FORMSET_NODE;

#define FORM_NODE_SIGNATURE             SIGNATURE_32 ('A', 'Q', 'F', 'N')
typedef struct {
  UINTN                                 Signature;
  LIST_ENTRY                            Link;
  EFI_IFR_FORM                          *Form;
  UINTN                                 FormSize;
} FORM_NODE;

#define FORMREF_NODE_SIGNATURE          SIGNATURE_32 ('A', 'Q', 'F', 'R')
typedef struct {
  UINTN                                 Signature;
  LIST_ENTRY                            Link;
  EFI_GUID                             *FormSetId;
  UINT16                                FormId;
} FORMREF_NODE;

#define URL_NODE_SIGNATURE              SIGNATURE_32 ('A', 'Q', 'U', 'N')
typedef struct {
  UINTN                                 Signature;
  LIST_ENTRY                            Link;
  EFI_HII_HANDLE                        HiiHandle;
  UINT16                                Name;
} URL_NODE;


EFI_STATUS
AttributeQuestionFormSetRegisterDepex (
  IN            EFI_GUID               *FormSetId,
  IN            EFI_FORM_ID             FormId,
  IN            BOOLEAN                 ResetGoto
);

EFI_STATUS
AttributeQuestionFormSetRegisterDepex (
  IN            EFI_GUID               *FormSetId,
  IN            EFI_FORM_ID             FormId,
  IN            BOOLEAN                 ResetGoto
);

VOID
AttributeQuestionFormSetFree (
  VOID
);

FORMSET_NODE *
AttributeQuestionGetFormSet (
  IN      CONST EFI_GUID               *FormSetId
);

EFI_HII_HANDLE
AttributeQuestionGetHiiHandle (
  IN      CONST EFI_GUID               *FormSetId
);

EFI_STATUS
AttributeQuestionFormListInit (
  IN      CONST EFI_IFR_FORM_SET       *FormSet,
  IN            UINTN                   FormSetSize,
  IN            LIST_ENTRY             *FormList
);

FORM_NODE *
AttributeQuestionGetForm (
  IN      CONST EFI_GUID               *FormSetId,
  IN      CONST UINT16                  FormId
);

VOID
AttributeQuestionPrint (
  IN      CONST EFI_GUID               *FormSetId,
  IN      CONST UINT16                  QuestionId
);

VOID
DebugAttributeQuestion (
  AQ_NODE                              *AttributeQuestion
);

#endif