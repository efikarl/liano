/** @file
  IPMI System Interface: KCS.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef IPMI_KCS_H_
#define IPMI_KCS_H_

#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Spec/IpmiKcsIntf.h>

#define KCS_RETRY_LIMIT       3       // [Times]
// 5s_KCS_TIME_OUT = KCS_DELAY_UNIT * KCS_TIME_OUT_LIMIT
#define KCS_DELAY_UNIT        50      // [MicroSeconds]
#define KCS_TIME_OUT_LIMIT    100000  // [Times]

//
// KCS IO Fucntion List
//

/**
  Read data from BMC.

  @param Data                 - Pointer to the data to be written
  @param DataSize             - Size of the data

  @retval EFI_SUCCESS         - Write the data successfully
  @retval EFI_TIMEOUT         - Get KCS status timeout
  @retval EFI_DEVICE_ERROR    - Bmc device error during data writting
**/
EFI_STATUS
EFIAPI
KcsWriteDataToBmc (
  IN UINT8 *Data,
  IN UINT8 DataSize
  );

/**
  Read data from BMC.

  @param Data                 - Pointer to the data to be reading
  @param DataSize             - Size of the data

  @retval EFI_SUCCESS         - Read the data successfully
  @retval EFI_TIMEOUT         - Get KCS status timeout
  @retval EFI_DEVICE_ERROR    - Bmc device error during data read
**/
EFI_STATUS
EFIAPI
KcsReadDataFromBmc (
  OUT UINT8 *Data,
  OUT UINT8 DataSize
  );

#endif