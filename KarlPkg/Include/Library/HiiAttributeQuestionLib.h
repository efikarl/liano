/** @file
  Copyright ©2021 Liu Yi, efikarl@yeah.net

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
--*/
#ifndef _HII_ATTRIBUTE_QUESTION_LIB_H_
#define _HII_ATTRIBUTE_QUESTION_LIB_H_
#include <Uefi.h>

#define QUESTION_URL_LENGTH             512
#define QUESTION_URL_SLASH              L"/"
#define MAX_GOTO_LIST                   4

#define AQ_NODE_SIGNATURE               SIGNATURE_32 ('A', 'Q', 'N', 'D')
#define AQ_GOTO_SIGNATURE               SIGNATURE_32 ('A', 'Q', 'G', 'T')

typedef struct {
  UINT32                                Signature;
  LIST_ENTRY                            Link;
  EFI_GUID                             *FormSetId;
  UINT16                                FormId;
  EFI_HII_HANDLE                        HiiHandle[2];
  EFI_STRING_ID                         Title[2];
  UINTN                                 Depth;
} AQ_GOTO;

typedef struct {
  UINT32                                Signature;
  EFI_HII_HANDLE                        RootHiiHandle;
  EFI_GUID                             *RootFormSetId;
  EFI_HII_HANDLE                        ThisHiiHandle;
  EFI_GUID                             *ThisFormSetId;
  UINT16                                RootFormId;
  UINT16                                QuestionId;
  EFI_STRING_ID                         Title[2];     // 0 for root formset, 1 for root form
  EFI_STRING_ID                         Prompt;
  LIST_ENTRY                            Link;
  LIST_ENTRY                            GotoList;
} AQ_NODE;

EFI_STATUS
AttributeQuestionInit (
  IN     EFI_HII_HANDLE                 HiiHandle,
  IN OUT EFI_GUID                       *FormSetId
);

AQ_NODE *
AttributeQuestionGetSelf (
  IN      CONST EFI_GUID                *RootFormSetId,
  IN      CONST EFI_GUID                *ThisFormSetId,
  IN      CONST UINT16                  QuestionId
);

VOID
AttributeQuestionFree (
  VOID
);
#endif