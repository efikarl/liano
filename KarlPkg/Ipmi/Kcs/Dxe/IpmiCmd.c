/** @file
  IPMI KCS Protocol Implement.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "IpmiCmd.h"

EFI_STATUS
IpmiKcsCmd (
  IN      IPMI_KCS_PROTOCOL   *This,
  IN      KSC_RQ              *KcsRq,
  OUT     KSC_RS              *KcsRs
  )
{
  EFI_STATUS                  Status;
  UINT8                       DataSize;
  IPMI_KSC_INSTANCE           *Instance;

  Instance = INSTANCE_FROM_IPMI_KCS_PROTOCOL_THIS(This);
  Instance->Request.NetFn     = KcsRq->NetFn;
  Instance->Request.Lun       = IPMI_KCS_LUN;
  Instance->Request.Cmd       = KcsRq->Cmd;

  if (KcsRq->DataSize > 0) {
    if (KcsRq->Data == NULL) return EFI_INVALID_PARAMETER;
    CopyMem (Instance->Request.RqData, KcsRq->Data, KcsRq->DataSize);
  }

  DataSize = KcsRq->DataSize + KCS_REQUEST_HEADER_SIZE;
  Status = KcsWriteDataToBmc  ((UINT8 *)&Instance->Request,  DataSize);
  if (Status != EFI_SUCCESS) return Status;

  DataSize = KcsRs->DataSize + KCS_RESPONSE_HEADER_SIZE;
  Status = KcsReadDataFromBmc ((UINT8 *)&Instance->Response, DataSize);
  if (Status != EFI_SUCCESS) return Status;

  DEBUG ((EFI_D_INFO, "IPMI CompletionCode = 0x%2x.\n", Instance->Response.CompletionCode));

  if ((KcsRs->DataSize > 0) && (Instance->Response.CompletionCode == 0)) {
    if (KcsRs->Data == NULL) return EFI_INVALID_PARAMETER;
    CopyMem (KcsRs->RsData, Instance->Response.RsData, KcsRs->DataSize);
    KcsRs->Data = KcsRs->RsData;
  }

  return EFI_SUCCESS;
}
