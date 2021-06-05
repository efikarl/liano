/** @file
  Copyright ©2021 Liu Yi, efikarl@yeah.net

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
--*/
#ifndef __FORMSET1_VFR_H__
#define __FORMSET1_VFR_H__
#include <Guid/HiiPlatformSetupFormset.h>
#include <Uefi/UefiMultiPhase.h>

#define FORMSET1_GUID { 0x6011d16f, 0xc329, 0x2543, { 0xaa, 0xf6, 0x07, 0x22, 0xc3, 0xbf, 0xfa, 0x56 } }

#define FORMSET1_VARID                                1

#define FORM1                                         1

#define FORM2                                         2
#define FORM2_KEY_STR_EXIT                            0xF200
#define FORM2_KEY_STR_SAVE                            0xF201
#define FORM2_KEY_STR_FORM_ASIS_AND_EXIT              0xF202
#define FORM2_KEY_STR_FORM_SAVE_AND_EXIT              0xF203

#define FORMSET1_VARID_TYPE_VARSTORE                  1
#define FORMSET1_VARID_TYPE_VARSTORE_EFI              0xEF10

#define FORMSET1_SETUP_NAME_VARSTORE                  L"Formset1SetupVarstore"
typedef struct {
  UINT8   CheckBox;
  UINT8   Numeric;
  UINT8   OneOf;
  UINT8   OrderedList;
  CHAR16  String[64];
} FORMSET1_SETUP_TYPE_VARSTORE;

#define FORMSET1_SETUP_NAME_VARSTORE_EFI              L"Formset1SetupVarstoreEfi"
typedef struct {
  UINT8 Bit0: 1;
  UINT8 Bit1: 1;
  UINT8 Bit2: 1;
  UINT8     : 1;
  UINT8 Bit4: 1;
  UINT8 Bit5: 1;
  UINT8 Bit6: 1;
  UINT8 Bit7: 1;
} FORMSET1_SETUP_TYPE_VARSTORE_EFI;

#endif