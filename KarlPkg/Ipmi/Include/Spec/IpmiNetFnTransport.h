/** @file
  IPMI Spec Command Definition Header File.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef IPMI_NETFN_TRANSPORT_H_
#define IPMI_NETFN_TRANSPORT_H_
//
// NetFn
//
#define IPMI_NETFN_TRANSPORT                                0x0C
//
// CMD
//
#define IPMI_TRANSPORT_SET_LAN_CONFIG_PARAMETERS            0x01
#define IPMI_TRANSPORT_GET_LAN_CONFIG_PARAMETERS            0x02
#define IPMI_TRANSPORT_SUSPEND_BMC_ARPS                     0x03
#define IPMI_TRANSPORT_GET_PACKET_STATISTICS                0x04
#define IPMI_TRANSPORT_SET_SERIAL_CONFIGURATION             0x10
#define IPMI_TRANSPORT_GET_SERIAL_CONFIGURATION             0x11
#define IPMI_TRANSPORT_SET_SERIAL_MUX                       0x12
#define IPMI_TRANSPORT_GET_TAP_RESPONSE_CODE                0x13
#define IPMI_TRANSPORT_SET_PPP_UDP_PROXY_TXDATA             0x14
#define IPMI_TRANSPORT_GET_PPP_UDP_PROXY_TXDATA             0x15
#define IPMI_TRANSPORT_SEND_PPP_UDP_PROXY_PACKET            0x16
#define IPMI_TRANSPORT_GET_PPP_UDP_PROXY_RX                 0x17
#define IPMI_TRANSPORT_SERIAL_CONNECTION_ACTIVE             0x18
#define IPMI_TRANSPORT_CALLBACK                             0x19
#define IPMI_TRANSPORT_SET_USER_CALLBACK_OPTIONS            0x1A
#define IPMI_TRANSPORT_GET_USER_CALLBACK_OPTIONS            0x1B
#define IPMI_TRANSPORT_SOL_ACTIVATING                       0x20
#define IPMI_TRANSPORT_SET_SOL_CONFIG_PARAM                 0x21
#define IPMI_TRANSPORT_GET_SOL_CONFIG_PARAM                 0x22

#endif