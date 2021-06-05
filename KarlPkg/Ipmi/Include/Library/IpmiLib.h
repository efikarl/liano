/** @file
  IPMI KCS Command Library.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef _IPMI_KCS_PPI_H_
#define _IPMI_KCS_PPI_H_

#include <Spec/Ipmi.h>

EFI_STATUS
EFIAPI
IpmiKcsCommand (
  IN      IPMI_KCS_PROTOCOL   *This,
  IN      KSC_RQ              *KcsRq,
  OUT     KSC_RS              *KcsRs
);

#endif