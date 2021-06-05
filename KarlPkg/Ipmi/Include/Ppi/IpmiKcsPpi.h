/** @file
  IPMI KCS PPI.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef _IPMI_KCS_PPI_H_
#define _IPMI_KCS_PPI_H_

#include <Spec/Ipmi.h>

typedef struct _IPMI_KCS_PPI IPMI_KCS_PPI;

#define IPMI_KCS_PPI_GUID \
  { 0xefcbb460, 0x1749, 0x44c5, { 0xa9, 0xff, 0x99, 0x8e, 0xa4, 0x9b, 0x01, 0x47 } }


//
//  IPMI Function Prototypes
//
typedef
EFI_STATUS
(EFIAPI *IPMI_KCS_INTERFACE) (
  IN  IPMI_KCS_PPI        *This,
  IN  KSC_RQ              *KcsRq,
  OUT KSC_RS              *KcsRs
  );

//
// IPMI TRANSPORT PPI
//
struct _IPMI_KCS_PPI {
  UINT64                Revision;
  IPMI_KCS_INTERFACE    IpmiKcsCmd;
};

extern EFI_GUID gEfiIpmiKcsPpiGuid;

#endif