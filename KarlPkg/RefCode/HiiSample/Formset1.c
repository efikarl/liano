/** @file
  Copyright ©2021 Liu Yi, efikarl@yeah.net

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
--*/
#include <Uefi.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include "Formset1.h"

GLOBAL_REMOVE_IF_UNREFERENCED
HII_VENDOR_DEVICE_PATH gFormset1HiiDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8)  (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    FORMSET1_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8)  (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

EFI_STATUS
Formset1TextWhoIAm (
  EFI_HANDLE                  DeviceHandle,
  EFI_HII_HANDLE              HiiHandle
) {
  CHAR16 *I = L"efikarl";

  if (HiiSetString (HiiHandle, STRING_TOKEN (STR_I_AM_WHO), I, NULL) == 0) {
    return EFI_OUT_OF_RESOURCES;
  }

  HiiSetString (HiiHandle, 0, I, NULL);

  return EFI_SUCCESS;
}
