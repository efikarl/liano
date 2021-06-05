/** @file
  IPMI Spec Command Definition Header File.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef IPMI_NETFN_CHASSIS_H_
#define IPMI_NETFN_CHASSIS_H_
//
// NetFn
//
#define IPMI_NETFN_CHASSIS                                  0x00
//
// CMD
//
#define IPMI_CHASSIS_GET_CAPABILITIES                       0x00
#define IPMI_CHASSIS_GET_STATUS                             0x01
#define IPMI_CHASSIS_CONTROL                                0x02
#define IPMI_CHASSIS_RESET                                  0x03
#define IPMI_CHASSIS_IDENTIFY                               0x04
#define IPMI_CHASSIS_SET_CAPABILITIES                       0x05
#define IPMI_CHASSIS_SET_POWER_RESTORE_POLICY               0x06
#define IPMI_CHASSIS_GET_SYSTEM_RESTART_CAUSE               0x07
#define IPMI_CHASSIS_SET_SYSTEM_BOOT_OPTIONS                0x08
#define IPMI_CHASSIS_GET_SYSTEM_BOOT_OPTIONS                0x09
#define IPMI_CHASSIS_SET_FP_BUTTON_ENABLES                  0x0A
#define IPMI_CHASSIS_SET_POWER_CYCLE_INTERVALS              0x0B
#define IPMI_CHASSIS_GET_POH_COUNTER                        0x0F

#endif