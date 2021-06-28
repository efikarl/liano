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

//
// assume that:
// 1. one package one hii handle, if not, get the first one
// 2. one form just be goto once, if not, get the short one
//
#define QUESTION_URL_LENGTH             256
#define QUESTION_URL_SLASH              L"/"

#define AQ_NODE_SIGNATURE               SIGNATURE_32 ('A', 'Q', 'N', 'D')

typedef enum {
  AQ_QUESTION,
  AQ_REF_FORM,
  AQ_REF_FSET,
} AQ_TYPE;

typedef struct {
  EFI_HII_HANDLE                        HiiHandle;
  EFI_GUID                             *FormSetId;
  EFI_STRING_ID                         FormSetTitle;
  UINT16                                FormId;
  EFI_STRING_ID                         FormTitle;
} AQ_PAGE;

typedef struct {
  UINT32                                Signature;
  AQ_PAGE                               ThisPage;
  AQ_PAGE                              *PrevPage;
  AQ_TYPE                               Type;
  UINT16                                QuestionId;
  EFI_STRING_ID                         Prompt;
  LIST_ENTRY                            Link;
} AQ_NODE;

EFI_STATUS
AttributeQuestionInit (
  IN     EFI_HII_HANDLE                 HiiHandle,
  IN OUT EFI_GUID                       *FormSetId
);

CHAR16 *
AttributeQuestionGetUrl (
  IN      CONST EFI_GUID                *FormSetId,
  IN      CONST UINT16                  QuestionId
);

VOID
AttributeQuestionFree (
  VOID
);
#endif