/** @file
  IPMI Spec Command Definition Header File.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef IPMI_NETFN_BRIDGE_H_
#define IPMI_NETFN_BRIDGE_H_
//
// NetFn
//
#define IPMI_NETFN_BRIDGE                                   0x02
//
// CMD
//
#define IPMI_BRIDGE_GET_STATE                               0x00
#define IPMI_BRIDGE_SET_STATE                               0x01
#define IPMI_BRIDGE_GET_ICMB_ADDRESS                        0x02
#define IPMI_BRIDGE_SET_ICMB_ADDRESS                        0x03
#define IPMI_BRIDGE_SET_PROXY_ADDRESS                       0x04
#define IPMI_BRIDGE_GET_BRIDGE_STATISTICS                   0x05
#define IPMI_BRIDGE_GET_ICMB_CAPABILITIES                   0x06
#define IPMI_BRIDGE_CLEAR_STATISTICS                        0x08
#define IPMI_BRIDGE_GET_PROXY_ADDRESS                       0x09
#define IPMI_BRIDGE_GET_ICMB_CONNECTOR_INFO                 0x0A
#define IPMI_BRIDGE_GET_ICMB_CONNECTION_ID                  0x0B
#define IPMI_BRIDGE_SEND_ICMB_CONNECTION_ID                 0x0C
#define IPMI_BRIDGE_PREPARE_FOR_DISCOVERY                   0x10
#define IPMI_BRIDGE_GET_ADDRESSES                           0x11
#define IPMI_BRIDGE_SET_DISCOVERED                          0x12
#define IPMI_BRIDGE_GET_CHASSIS_DEVICEID                    0x13
#define IPMI_BRIDGE_SET_CHASSIS_DEVICEID                    0x14
#define IPMI_BRIDGE_REQUEST                                 0x20
#define IPMI_BRIDGE_MESSAGE                                 0x21
#define IPMI_BRIDGE_GET_EVENT_COUNT                         0x30
#define IPMI_BRIDGE_SET_EVENT_DESTINATION                   0x31
#define IPMI_BRIDGE_SET_EVENT_RECEPTION_STATE               0x32
#define IPMI_BRIDGE_SET_EVENT_RECEPTION_STATE               0x32
#define IPMI_BRIDGE_SEND_ICMB_EVENT_MESSAGE                 0x33

#endif