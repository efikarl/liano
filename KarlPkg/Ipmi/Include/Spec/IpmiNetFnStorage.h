/** @file
  IPMI Spec Command Definition Header File.

  Copyright (c) 2016, efikarl.

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#ifndef IPMI_NETFN_STORAG_H_
#define IPMI_NETFN_STORAG_H_
//
// NetFn
//
#define IPMI_NETFN_STORAGE                                  0x0A
//
// CMD
//
#define IPMI_STORAGE_GET_FRU_INVENTORY_AREAINFO             0x10
#define IPMI_STORAGE_READ_FRU_DATA                          0x11
#define IPMI_STORAGE_WRITE_FRU_DATA                         0x12
#define IPMI_STORAGE_GET_SDR_REPOSITORY_INFO                0x20
#define IPMI_STORAGE_GET_SDR_REPOSITORY_ALLOCATION_INFO     0x21
#define IPMI_STORAGE_RESERVE_SDR_REPOSITORY                 0x22
#define IPMI_STORAGE_GET_SDR                                0x23
#define IPMI_STORAGE_ADD_SDR                                0x24
#define IPMI_STORAGE_PARTIAL_ADD_SDR                        0x25
#define IPMI_STORAGE_DELETE_SDR                             0x26
#define IPMI_STORAGE_CLEAR_SDR                              0x27
#define IPMI_STORAGE_GET_SDR_REPOSITORY_TIME                0x28
#define IPMI_STORAGE_SET_SDR_REPOSITORY_TIME                0x29
#define IPMI_STORAGE_ENTER_SDR_UPDATE_MODE                  0x2A
#define IPMI_STORAGE_EXIT_SDR_UPDATE_MODE                   0x2B
#define IPMI_STORAGE_RUN_INIT_AGENT                         0x2C
#define IPMI_STORAGE_GET_SEL_INFO                           0x40
#define IPMI_STORAGE_GET_SEL_ALLOCATION_INFO                0x41
#define IPMI_STORAGE_RESERVE_SEL                            0x42
#define IPMI_STORAGE_GET_SEL_ENTRY                          0x43
#define IPMI_STORAGE_ADD_SEL_ENTRY                          0x44
#define IPMI_STORAGE_PARTIAL_ADD_SEL_ENTRY                  0x45
#define IPMI_STORAGE_DELETE_SEL_ENTRY                       0x46
#define IPMI_STORAGE_CLEAR_SEL                              0x47
#define IPMI_STORAGE_GET_SEL_TIME                           0x48
#define IPMI_STORAGE_SET_SEL_TIME                           0x49
#define IPMI_STORAGE_GET_AUXILLARY_LOG_STATUS               0x5A
#define IPMI_STORAGE_SET_AUXILLARY_LOG_STATUS               0x5B

#endif