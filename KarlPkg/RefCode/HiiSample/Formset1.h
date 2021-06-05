/** @file
  Copyright ©2021 Liu Yi, efikarl@yeah.net

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
--*/
#ifndef __FORMSET1_H__
#define __FORMSET1_H__

#include <Uefi.h>
#include "Formset1.vfr.h"

EFI_STATUS
Formset1TextWhoIAm (
  EFI_HANDLE                  DeviceHandle,
  EFI_HII_HANDLE              HiiHandle
);

///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH              VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        End;
} HII_VENDOR_DEVICE_PATH;

extern HII_VENDOR_DEVICE_PATH     gFormset1HiiDevicePath;

#endif