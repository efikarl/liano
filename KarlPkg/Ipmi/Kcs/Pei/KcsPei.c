/** @file
  IPMI KCS PPI Implement.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "IpmiCmd.h"

IPMI_KSC_INSTANCE mInstance;

EFI_STATUS
GetSelfTestResults(
  IPMI_KSC_INSTANCE *Instance
  )
{
  EFI_STATUS  Status;
  UINTN       Times;

  Instance->Request.NetFn     = IPMI_NETFN_APP;
  Instance->Request.Cmd       = IPMI_APP_GET_SELFTEST_RESULTS;
  Instance->Request.DataSize  = 0;
  Instance->Response.DataSize = 2;
  for (Times = 3; Times > 0; Times--) {
    Status = IpmiKcsCmd (
               &mInstance.Ppi,
               &Instance->Request,
               &Instance->Response
               );
    if (Status == EFI_SUCCESS) break;
  }
  if (Times != 0 && Instance->Response.Data[0] == 0x55) {
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
}

EFI_STATUS
BmcInitEntryPoint(
  IN EFI_PEI_FILE_HANDLE          FileHandle,
  IN CONST EFI_PEI_SERVICES       **PeiServices
)
{
  EFI_STATUS        Status;

  mInstance.Signature             = IPMI_KCS_SIGNATURE;
  mInstance.Ppi.Revision          = IPMI_SPEC_REVISION;
  mInstance.Ppi.IpmiKcsCmd        = IpmiKcsCmd;
  mInstance.PpiDsc.Flags          = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  mInstance.PpiDsc.Guid           = &gEfiIpmiKcsPpiGuid;
  mInstance.PpiDsc.Ppi            = &mInstance.Ppi;

  Status = (**PeiServices).InstallPpi (PeiServices, &mInstance.PpiDsc);
  ASSERT (Status);

  Status = GetSelfTestResults (&mInstance);
  return Status;
}