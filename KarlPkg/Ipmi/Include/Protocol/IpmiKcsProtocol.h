/** @file
  IPMI KCS Protocol.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef _IPMI_KCS_PROTOCOL_H_
#define _IPMI_KCS_PROTOCOL_H_

#include <Spec/Ipmi.h>

typedef struct _IPMI_KCS_PROTOCOL IPMI_KCS_PROTOCOL;

#define IPMI_KCS_PROTOCOL_GUID \
  { 0x8e35a95d, 0x98e5, 0x46b3, { 0x99, 0x12, 0x2d, 0x32, 0xa8, 0x3b, 0xad, 0xf0 } }


//
//  IPMI Function Prototypes
//
typedef
EFI_STATUS
(EFIAPI *IPMI_KCS_INTERFACE) (
  IN  IPMI_KCS_PROTOCOL       *This,
  IN  KSC_RQ                  *KcsRq,
  OUT KSC_RS                  *KcsRs
  );

//
// IPMI TRANSPORT PROTOCOL
//
struct _IPMI_KCS_PROTOCOL {
  UINT64                Revision;
  IPMI_KCS_INTERFACE    IpmiKcsCmd;
};

extern EFI_GUID gEfiIpmiKcsProtocolGuid;

#endif