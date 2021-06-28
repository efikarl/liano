/** @file
  Copyright ©2021 Liu Yi, efikarl@yeah.net

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
--*/

#include "HiiSample.h"

HII_SAMPLE_PRIVATE_DATA mPrivateData;
EFI_GUID                mFormset1Guid   = FORMSET1_GUID;
EFI_GUID                mFormset2Guid   = FORMSET2_GUID;
HII_DEVICE_PATH mFormset1HiiDevicePath  = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8)  (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    FORMSET1_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8)  (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};
HII_DEVICE_PATH mFormset2HiiDevicePath  = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8)  (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    FORMSET2_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8)  (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};


EFI_STATUS
FormsetVarstoreInit (
  IN  EFI_GUID                         *FormsetGuid,
  IN  CHAR16                           *VariableName,
  IN  VOID                             *VariableData,
  IN  UINTN                             VariableSize,
  IN  EFI_HANDLE                        DriverHandle
) {
  EFI_STATUS                            Status = EFI_SUCCESS;
  UINTN                                 OnBehalfOfVariableSize;
  BOOLEAN                               ActionFlag;
  EFI_STRING                            ConfigRequestHdr;

  DEBUG ((DEBUG_INFO, "%a().+\n", __FUNCTION__));
  //
  // Initialize efivarstore configuration data
  //
  ZeroMem (VariableData, VariableSize);

  ConfigRequestHdr = HiiConstructConfigHdr (FormsetGuid, VariableName, DriverHandle);
  ASSERT (ConfigRequestHdr != NULL);

  OnBehalfOfVariableSize  = VariableSize;
  Status = gRT->GetVariable (VariableName, FormsetGuid, NULL, &OnBehalfOfVariableSize, VariableData);
  DEBUG ((DEBUG_INFO, "gRT->GetVariable Status = %r\n", Status));
  if (EFI_ERROR (Status)) {
    //
    // Store stdandard default data to EFI Variable Storage.
    //
    Status = gRT->SetVariable(
                    VariableName,
                    FormsetGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                    VariableSize,
                    VariableData
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "gRT->SetVariable Status = %r\n", Status));
      goto Done;
    }
    ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
    if (!ActionFlag) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  } else {
    //
    // EFI variable does exist and Validate Current Setting
    //
    ActionFlag = HiiValidateSettings (ConfigRequestHdr);
    if (!ActionFlag) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  FreePool (ConfigRequestHdr);

Done:
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

EFI_STATUS
Formset1Init(
  VOID
) {
  EFI_STATUS  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "%a().+\n", __FUNCTION__));
  //
  // Publish our Hii Protocols
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mPrivateData.Handle[0],
                  &gEfiDevicePathProtocolGuid,
                  &mFormset1HiiDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData.Hii.ConfigAccess,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Add packagelist of HII packages in the HII Database
  //
  mPrivateData.Hii.Handle[0] = HiiAddPackages (
                                 &mFormset1Guid,
                                 mPrivateData.Handle[0],
                                 Formset1Bin,
                                 HiiSampleStrings,
                                 NULL
                                 );
  if (mPrivateData.Hii.Handle[0] == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // Update Formset1.vfr/formid=1/text.STR_WHO_I_AM
  //
  if (HiiSetString (mPrivateData.Hii.Handle[0], STRING_TOKEN(STR_I_AM_WHO), L"LiuYi", NULL) == 0) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // Formset Variable Init
  //
  Status = FormsetVarstoreInit(
             &mFormset1Guid,
             FORMSET1_SETUP_NAME_VARSTORE_EFI,
             &mPrivateData.Setup.Formset1SetupVarstoreEfi,
             sizeof(FORMSET1_SETUP_TYPE_VARSTORE_EFI),
             mPrivateData.Handle[0]
             );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  Status = FormsetVarstoreInit(
             &mFormset1Guid,
             FORMSET1_SETUP_NAME_VARSTORE,
             &mPrivateData.Setup.Formset1SetupVarstore,
             sizeof(FORMSET1_SETUP_TYPE_VARSTORE),
             mPrivateData.Handle[0]
             );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

Done:
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

EFI_STATUS
Formset2Init(
  VOID
) {
  EFI_STATUS  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "%a().+\n", __FUNCTION__));
  //
  // Publish our Hii Protocols
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mPrivateData.Handle[1],
                  &gEfiDevicePathProtocolGuid,
                  &mFormset2HiiDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData.Hii.ConfigAccess,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Add packagelist of HII packages in the HII Database
  //
  mPrivateData.Hii.Handle[1] = HiiAddPackages (
                                 &mFormset2Guid,
                                 mPrivateData.Handle[1],
                                 Formset2Bin,
                                 HiiSampleStrings,
                                 NULL
                                 );
  if (mPrivateData.Hii.Handle[1] == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // Update Formset2.vfr/formid=1/text.STR_WHO_I_AM
  //
  if (HiiSetString (mPrivateData.Hii.Handle[1], STRING_TOKEN(STR_I_AM_WHO), L"efika", NULL) == 0) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

Done:
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

/**
  Unloads an image.

  @param  ImageHandle           Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
  @retval EFI_INVALID_PARAMETER ImageHandle is not a valid image handle.

**/
EFI_STATUS
EFIAPI
HiiSampleUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
  //
  // Do any additional cleanup that is required for this driver
  //

  return Status;
}

/**
  This is the declaration of an EFI image entry point. This entry point is
  the same for UEFI Applications, UEFI OS Loaders, and UEFI Drivers including
  both device drivers and bus drivers.

  @param  ImageHandle           The firmware allocated handle for the UEFI image.
  @param  SystemTable           A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval Others                An unexpected error occurred.
**/
EFI_STATUS
EFIAPI
HiiSampleDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "%a().+\n", __FUNCTION__));
  //
  // Retrieve the pointer to the UEFI HII Database Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&mPrivateData.Hii.Database
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  //  Retrieve the pointer to the UEFI HII ConfigRouting Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **)&mPrivateData.Hii.ConfigRouting
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // mPrivateDate init
  //
  mPrivateData.Signature = HII_SAMPLE_PRIVATE_SIGNATURE;

  mPrivateData.Hii.ConfigAccess.ExtractConfig = HiiSampleHiiConfigAccessExtractConfig;
  mPrivateData.Hii.ConfigAccess.RouteConfig   = HiiSampleHiiConfigAccessRouteConfig;
  mPrivateData.Hii.ConfigAccess.Callback      = HiiSampleHiiConfigAccessCallback;

  Status = Formset1Init();
  if (EFI_ERROR (Status)) {
    HiiSampleUnload(mPrivateData.Handle[0]);
    goto Done;
  }
  Status = Formset2Init();
  if (EFI_ERROR (Status)) {
    HiiSampleUnload(mPrivateData.Handle[1]);
    goto Done;
  }

  AttributeQuestionInit(mPrivateData.Hii.Handle[0], &mFormset1Guid);
  AttributeQuestionInit(mPrivateData.Hii.Handle[1], &mFormset2Guid);

Done:
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}
