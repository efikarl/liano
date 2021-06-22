/** @file
  Copyright ©2021 Liu Yi, efikarl@yeah.net

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
--*/

#ifndef __EFI_HII_SAMPLE_H__
#define __EFI_HII_SAMPLE_H__

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HiiAttributeQuestionLib.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiConfigRouting.h>

#include "HiiConfigAccess.h"
#include "Formset1.vfr.h"
#include "Formset2.vfr.h"
//
// This is the auto-gen IFR bin data for each formset in VFR.
// This data is ready to be as input of HiiAddPackages() to create a packagelist.
//
extern UINT8  Formset1Bin[];
extern UINT8  Formset2Bin[];
//
// This is the auto-gen String package data for all .UNI files.
// This data is ready to be as input of HiiAddPackages() to create a packagelist.
//
extern UINT8  HiiSampleStrings[];
///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH              VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        End;
} HII_DEVICE_PATH;

typedef struct {
  FORMSET1_SETUP_TYPE_VARSTORE_EFI      Formset1SetupVarstoreEfi;
  FORMSET1_SETUP_TYPE_VARSTORE          Formset1SetupVarstore;
} HII_SAMPLE_SETUP;


#define HII_SAMPLE_PRIVATE_SIGNATURE SIGNATURE_32 ('H', 'S', 'P', 'D')
typedef struct {
  EFI_HII_HANDLE                        Handle[2];
  //
  // Consumed Protocols
  //
  EFI_HII_DATABASE_PROTOCOL             *Database;
  EFI_HII_CONFIG_ROUTING_PROTOCOL       *ConfigRouting;
  //
  // Produced Protocols
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL        ConfigAccess;
} HII_HANDLE_AND_PROTOCOL;

typedef struct {
  UINTN                                 Signature;
  EFI_HANDLE                            Handle[2];

  HII_HANDLE_AND_PROTOCOL               Hii;
  HII_SAMPLE_SETUP                      Setup;
} HII_SAMPLE_PRIVATE_DATA;

#define HII_SAMPLE_PRIVATE_DATA_FROM_HCAP(Hcap) \
  CR (BASE_CR (Hcap, HII_HANDLE_AND_PROTOCOL, ConfigAccess), HII_SAMPLE_PRIVATE_DATA, Hii, HII_SAMPLE_PRIVATE_SIGNATURE)

#endif
