/** @file
  IPMI System Interface Spec.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef IPMI_KCS_INTERFACE_H_
#define IPMI_KCS_INTERFACE_H_

//
// KCS Interface message
//
// IPMI Version2.0 Revesion 1.0 
#define IPMI_SPEC_REVISION              0x2010

// KSC data size
#define KCS_REQUEST_HEADER_SIZE         2
#define KCS_RESPONSE_HEADER_SIZE        3
#define KCS_REQUEST_DATA_SIZE           0xFF
#define KCS_RESPONSE_DATA_SIZE          0xFF

// LUN 00b is typically used for all messages to the BMC through the KCS Interface.
#define IPMI_KCS_LUN                    0

typedef struct _KSC_RQ {
  UINT8 Lun   : 2;
  UINT8 NetFn : 6;
  UINT8 Cmd;
  UINT8 RqData[KCS_REQUEST_DATA_SIZE];
  UINT8 *Data;
  UINT8 DataSize;
} KSC_RQ;

typedef struct _KSC_RS {
  UINT8 Lun   : 2;
  UINT8 NetFn : 6;
  UINT8 Cmd;
  UINT8 CompletionCode;
  UINT8 RsData[KCS_RESPONSE_DATA_SIZE];
  UINT8 *Data;
  UINT8 DataSize;
} KSC_RS;

//
// KCS Interface Registers
//
// The default system base address for an I/O mapped KCS SMS Interface is CA2h.
#define IPMI_KSC_BASE_ADDRESS           0xCA2
#define IPMI_R_KSC_DATA_IN              (IPMI_KSC_BASE_ADDRESS + 0) // wo
#define IPMI_R_KSC_DATA_OUT             (IPMI_KSC_BASE_ADDRESS + 0) // ro
#define IPMI_R_KSC_COMMAND              (IPMI_KSC_BASE_ADDRESS + 1) // wo
#define IPMI_R_KSC_STATUS               (IPMI_KSC_BASE_ADDRESS + 1) // ro

// KCS Interface Status Register Bits
typedef union _IPMI_KCS_STATUS {
  UINT8 RawData;
  struct {
  UINT8 Obf     : 1;
  UINT8 Ibf     : 1;
  UINT8 SmsAtn  : 1;
  UINT8 Cd      : 1;
  UINT8 Oem1    : 1;
  UINT8 Oem2    : 1;
  UINT8 State   : 2;
  } Bit;
} IPMI_KCS_STATUS;

// KCS Interface State Bits
typedef enum _IPMI_KCS_STATE {
  KSC_STATE_IDLE,
  KSC_STATE_READ,
  KSC_STATE_WRITE,
  KSC_STATE_ERROR
} IPMI_KCS_STATE;

// KCS Control Codes
#define KSC_CRTL_ABORT                  0x60
#define KSC_CRTL_WRITE_START            0x61
#define KSC_CRTL_WRITE_END              0x62
#define KSC_CRTL_READ                   0x68

// KCS Interface Status Codes
#define KSC_STATUS_NO_ERROR             0x00
#define KSC_STATUS_ABORTED_BY_COMMAND   0x01
#define KSC_STATUS_ILLEGAL_CONTROL_CODE 0x02
#define KSC_STATUS_LENGTH_ERROR         0x06
#define KSC_STATUS_UNSPECIFIED_ERROR    0xFF

#endif