/** @file
  IPMI KCS Protocol Implement.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef IPMI_CMD_H_
#define IPMI_CMD_H_

#include "IpmiKcs.h"
#include <Library/BaseMemoryLib.h>
#include <Protocol/IpmiKcsProtocol.h>

// Ipmi instance
typedef struct {
  UINTN                   Signature;
  EFI_HANDLE              Handle;
  IPMI_KCS_PROTOCOL       Protocol;
  KSC_RQ                  Request;
  KSC_RS                  Response;
} IPMI_KSC_INSTANCE;

#define IPMI_KCS_SIGNATURE SIGNATURE_32 ('i', 'p', 'm', 'i')

#define INSTANCE_FROM_IPMI_KCS_PROTOCOL_THIS(a)  CR(a, IPMI_KSC_INSTANCE, Protocol, IPMI_KCS_SIGNATURE)

//
// Ipmi message interface
//

/*++

Routine Description:

  IPMI KCS protocol interface instance

Arguments:

  This                  - Pointer to IPMI protocol instance
  KcsRq                 - Pointer to request message buffer
  KcsRs                 - Pointer to response message buffer

Returns:

  EFI_SUCCESS           - Ipmi message completes successfully
  EFI_INVALID_PARAMETER - One of the input parameters is invalid
  EFI_BUFFER_TOO_SMALL  - Response buffer is too small
  EFI_TIME_OUT          - Timeout when waiting for a BMC status
  EFI_DEVICE_ERROR      - Ipmi inferce error during message

--*/
EFI_STATUS
EFIAPI
IpmiKcsCmd (
  IN      IPMI_KCS_PROTOCOL   *This,
  IN      KSC_RQ              *KcsRq,
  OUT     KSC_RS              *KcsRs
  );

#endif