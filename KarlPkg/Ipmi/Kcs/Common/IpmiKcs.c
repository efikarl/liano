/** @file
  IPMI System Interface: KCS.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "IpmiKcs.h"

EFI_STATUS
WaitForIbfFalse (
  OUT IPMI_KCS_STATUS *KcsStatus
)
{
  UINT64          TimeOut;

  TimeOut = 0;
  do {
    MicroSecondDelay(KCS_DELAY_UNIT);
    KcsStatus->RawData = IoRead8 (IPMI_R_KSC_STATUS);
    if (KcsStatus->RawData == KSC_STATUS_UNSPECIFIED_ERROR) {
      return EFI_DEVICE_ERROR;
    }
    if (TimeOut++ >= KCS_TIME_OUT_LIMIT) {
      return EFI_TIMEOUT;
    }
  } while (KcsStatus->Bit.Ibf);

  return EFI_SUCCESS;
}

EFI_STATUS
WaitForObfTrue (
  OUT IPMI_KCS_STATUS *KcsStatus
)
{
  UINT64          TimeOut;

  TimeOut = 0;
  do {
    MicroSecondDelay(KCS_DELAY_UNIT);
    KcsStatus->RawData = IoRead8 (IPMI_R_KSC_STATUS);
    if (KcsStatus->RawData == KSC_STATUS_UNSPECIFIED_ERROR) {
      return EFI_DEVICE_ERROR;
    }
    if (TimeOut++ >= KCS_TIME_OUT_LIMIT) {
      return EFI_TIMEOUT;
    }
  } while (!KcsStatus->Bit.Obf);

  return EFI_SUCCESS;
}

EFI_STATUS
KcsErrorExit()
{
  EFI_STATUS      Status;
  IPMI_KCS_STATUS KcsStatus;
  UINT8           KcsPhase;
  UINT8           RetryCount;

  RetryCount = 0;
  while (RetryCount < KCS_RETRY_LIMIT) {
    WaitForIbfFalse(&KcsStatus);

    KcsPhase = KSC_CRTL_ABORT;
    IoWrite8 (IPMI_R_KSC_COMMAND, KcsPhase);

    Status = WaitForIbfFalse(&KcsStatus);
    if (Status != EFI_SUCCESS) return Status;

    if (KcsStatus.Bit.Obf == 1) {
      IoRead8 (IPMI_R_KSC_DATA_OUT);
    }

    KcsPhase = 0x00;
    IoWrite8 (IPMI_R_KSC_DATA_IN, KcsPhase);

    Status = WaitForIbfFalse(&KcsStatus);
    if (Status != EFI_SUCCESS) return Status;

    if (KcsStatus.Bit.State == KSC_STATE_READ) {
      Status = WaitForObfTrue(&KcsStatus);
      if (Status != EFI_SUCCESS) return Status;

      IoRead8 (IPMI_R_KSC_DATA_OUT);

      KcsPhase = KSC_CRTL_READ;
      IoWrite8 (IPMI_R_KSC_DATA_IN, KcsPhase);

      Status = WaitForIbfFalse(&KcsStatus);
      if (Status != EFI_SUCCESS) return Status;
    }  else {
      if (++RetryCount == KCS_RETRY_LIMIT) {
        return EFI_DEVICE_ERROR;
      } else {
        continue;
      }
    }

    if (KcsStatus.Bit.State == KSC_STATE_IDLE) {
      Status = WaitForObfTrue(&KcsStatus);
      if (Status != EFI_SUCCESS) return Status;

      IoRead8 (IPMI_R_KSC_DATA_OUT);

      break;
    } else {
      if (++RetryCount == KCS_RETRY_LIMIT) {
        return EFI_DEVICE_ERROR;
      } else {
        continue;
      }
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
KcsCheckStatus (
  OUT IPMI_KCS_STATUS *KcsStatus,
  IN  IPMI_KCS_STATE  KcsStateForCheck
)
{
  EFI_STATUS Status;

  Status = WaitForIbfFalse(KcsStatus);
  if (Status != EFI_SUCCESS) return Status;

  switch (KcsStateForCheck) {
  case KSC_STATE_WRITE:
    if (KcsStatus->Bit.State == KSC_STATE_WRITE) {
      IoRead8 (IPMI_R_KSC_DATA_OUT);
      return EFI_SUCCESS;
    } else {
      return KcsErrorExit ();
    }
  break;
  case KSC_STATE_READ:
    if (KcsStatus->Bit.State == KSC_STATE_READ || KcsStatus->Bit.State == KSC_STATE_IDLE) {
      Status = WaitForObfTrue(KcsStatus);
      if (Status != EFI_SUCCESS) return Status;
    } else {
      return KcsErrorExit ();
    }
    return EFI_SUCCESS;
  break;
  case KSC_STATE_IDLE:
    return EFI_SUCCESS;
  break;
  default:
    return EFI_INVALID_PARAMETER;
  break;
  }
}

EFI_STATUS
KcsWriteDataToBmc (
  IN UINT8 *Data,
  IN UINT8 DataSize
  )
{
  EFI_STATUS      Status;
  IPMI_KCS_STATUS KcsStatus;
  UINT8           KcsPhase;
  UINT8           i;

  Status = WaitForIbfFalse(&KcsStatus);
  if (Status != EFI_SUCCESS) return Status;

  IoRead8 (IPMI_R_KSC_DATA_OUT);

  KcsPhase = KSC_CRTL_WRITE_START;
  IoWrite8 (IPMI_R_KSC_STATUS, KcsPhase);
  for (i = 0; i < DataSize; i++) {
    Status = KcsCheckStatus (&KcsStatus, KSC_STATE_WRITE);
    if (Status != EFI_SUCCESS) return Status;

    IoWrite8 (IPMI_R_KSC_DATA_IN, Data[i]);
  }
  Status = KcsCheckStatus (&KcsStatus, KSC_STATE_WRITE);
  if (Status != EFI_SUCCESS) return Status;
  KcsPhase = KSC_CRTL_WRITE_END;
  IoWrite8 (IPMI_R_KSC_COMMAND, KcsPhase);

  return EFI_SUCCESS;
}

EFI_STATUS
KcsReadDataFromBmc (
  OUT UINT8 *Data,
  IN  UINT8 DataSize
  )
{
  EFI_STATUS      Status;
  IPMI_KCS_STATUS KcsStatus;
  UINT8           KcsPhase;
  UINT8           i;

  Status = WaitForIbfFalse(&KcsStatus);
  if (Status != EFI_SUCCESS) return Status;

  for (i = 0; i < DataSize; i++) {
    KcsPhase = KSC_CRTL_READ;
    IoWrite8 (IPMI_R_KSC_DATA_IN, KcsPhase);

    Status = KcsCheckStatus (&KcsStatus, KSC_STATE_READ);
    if (Status != EFI_SUCCESS) return Status;

    if (KcsStatus.Bit.State == KSC_STATE_IDLE) {
      break;
    }
    Data[i++] = IoRead8 (IPMI_R_KSC_DATA_OUT);
  }

  return EFI_SUCCESS;
}
