/** @file
  IPMI Spec Command Definition Header File.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef IPMI_NETFN_APP_H_
#define IPMI_NETFN_APP_H_
//
// NetFn
//
#define IPMI_NETFN_APP                                      0x06
//
// CMD
//
#define IPMI_APP_GET_DEVICE_ID                              0x01
#define IPMI_APP_COLD_RESET                                 0x02
#define IPMI_APP_WARM_RESET                                 0x03
#define IPMI_APP_GET_SELFTEST_RESULTS                       0x04
#define IPMI_APP_MANUFACTURING_TEST_ON                      0x05
#define IPMI_APP_SET_ACPI_POWERSTATE                        0x06
#define IPMI_APP_GET_ACPI_POWERSTATE                        0x07
#define IPMI_APP_GET_DEVICE_GUID                            0x08
#define IPMI_APP_RESET_WATCHDOG_TIMER                       0x22
#define IPMI_APP_SET_WATCHDOG_TIMER                         0x24
#define IPMI_APP_GET_WATCHDOG_TIMER                         0x25
#define IPMI_APP_SET_BMC_GLOBAL_ENABLES                     0x2E
#define IPMI_APP_GET_BMC_GLOBAL_ENABLES                     0x2F
#define IPMI_APP_CLEAR_MESSAGE_FLAGS                        0x30
#define IPMI_APP_GET_MESSAGE_FLAGS                          0x31
#define IPMI_APP_ENABLE_MESSAGE_CHANNEL_RECEIVE             0x32
#define IPMI_APP_GET_MESSAGE                                0x33
#define IPMI_APP_SEND_MESSAGE                               0x34
#define IPMI_APP_READ_EVENT_MSG_BUFFER                      0x35
#define IPMI_APP_GET_BT_INTERFACE_CAPABILITY                0x36
#define IPMI_APP_GET_SYSTEM_GUID                            0x37
#define IPMI_APP_GET_CHANNEL_AUTHENTICATION_CAPABILITIES    0x38
#define IPMI_APP_GET_SESSION_CHALLENGE                      0x39
#define IPMI_APP_ACTIVATE_SESSION                           0x3A
#define IPMI_APP_SET_SESSION_PRIVELEGE_LEVEL                0x3B
#define IPMI_APP_CLOSE_SESSION                              0x3C
#define IPMI_APP_GET_SESSION_INFO                           0x3D
#define IPMI_APP_GET_AUTHCODE                               0x3F
#define IPMI_APP_SET_CHANNEL_ACCESS                         0x40
#define IPMI_APP_GET_CHANNEL_ACCESS                         0x41
#define IPMI_APP_GET_CHANNEL_INFO                           0x42
#define IPMI_APP_GET_CHANNEL_INFO                           0x42
#define IPMI_APP_SET_USER_ACCESS                            0x43
#define IPMI_APP_GET_USER_ACCESS                            0x44
#define IPMI_APP_SET_USER_NAME                              0x45
#define IPMI_APP_GET_USER_NAME                              0x46
#define IPMI_APP_SET_USER_PASSWORD                          0x47
#define IPMI_APP_ACTIVATE_PAYLOAD                           0x48
#define IPMI_APP_DEACTIVATE_PAYLOAD                         0x49
#define IPMI_APP_GET_PAYLOAD_ACTIVATION_STATUS              0x4A
#define IPMI_APP_GET_PAYLOAD_INSTANCE_INFO                  0x4B
#define IPMI_APP_SET_USER_PAYLOAD_ACCESS                    0x4C
#define IPMI_APP_GET_USER_PAYLOAD_ACCESS                    0x4D
#define IPMI_APP_GET_CHANNEL_PAYLOAD_SUPPORT                0x4E
#define IPMI_APP_GET_CHANNEL_PAYLOAD_VERSION                0x4F
#define IPMI_APP_GET_CHANNEL_OEM_PAYLOAD_INFO               0x50
#define IPMI_APP_MASTER_WRITE_READ                          0x52
#define IPMI_APP_GET_CHANNEL_CIPHER_SUITES                  0x54
#define IPMI_APP_SUSPEND_RESUME_PAYLOAD_ENCRYPTION          0x55
#define IPMI_APP_SET_CHANNEL_SECURITY_KEYS                  0x56
#define IPMI_APP_GET_SYSTEM_INTERFACE_CAPABILITIES          0x57

#endif