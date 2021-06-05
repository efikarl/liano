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
  UINTN                                 Signature;
  LIST_ENTRY                            Link;
  UINT16                                FormId;
  EFI_STRING_ID                         Title;
  UINTN                                 Depth;
} AQ_GOTO;

typedef struct {
  UINTN                                 Signature;
  EFI_HII_HANDLE                        Handle;
  UINT16                                Fid;
  UINT16                                Qid;
  EFI_STRING_ID                         Title[2];     // 0 for formset, 1 for 1st form
  EFI_STRING_ID                         Prompt;
  LIST_ENTRY                            Link;
  LIST_ENTRY                            GotoList;
} AQ_NODE;

EFI_STATUS
AttributeQuestionInit (
  IN     EFI_HII_HANDLE                 HiiHandle,
  IN OUT EFI_GUID                       *FormSetGuid
);

AQ_NODE *
AttributeQuestionGetSelf (
  IN      CONST UINT16                  QuestionId
);

VOID
AttributeQuestionFree (
  VOID
);
#endif