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
  EFI_STATUS                            Status;
  EFI_HII_PACKAGE_LIST_HEADER           *PackageListHeader;

  Status = EFI_SUCCESS;
  //
  // mPrivateDate init
  //
  mPrivateData.Signature = HII_SAMPLE_PRIVATE_SIGNATURE;

  mPrivateData.Hii.ConfigAccess.ExtractConfig = HiiSampleHiiConfigAccessExtractConfig;
  mPrivateData.Hii.ConfigAccess.RouteConfig   = HiiSampleHiiConfigAccessRouteConfig;
  mPrivateData.Hii.ConfigAccess.Callback      = HiiSampleHiiConfigAccessCallback;
  //
  // Publish our Hii Protocols
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mPrivateData.Handle,
                  &gEfiDevicePathProtocolGuid,
                  &gFormset1HiiDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData.Hii.ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  //
  // Retrieve HII Package List Header on mPrivateData.Handle
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageListHeader,
                  mPrivateData.Handle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Retrieve the pointer to the UEFI HII Database Protocol 
    //
    Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **)&mPrivateData.Hii.Database);
    if (!EFI_ERROR (Status)) {
      //
      // Register list of HII packages in the HII Database
      //
      Status = mPrivateData.Hii.Database->NewPackageList (
                              mPrivateData.Hii.Database, 
                              PackageListHeader,
                              mPrivateData.Handle, 
                              &mPrivateData.Hii.Handle
                              );
      ASSERT_EFI_ERROR (Status);
      //
      //  Retrieve the pointer to the UEFI HII ConfigRouting Protocol 
      //
      Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **)&mPrivateData.Hii.ConfigRouting);
      ASSERT_EFI_ERROR (Status);
    }
  } else {
    DEBUG ((DEBUG_INFO, "%a(): Fail to open gEfiHiiPackageListProtocolGuid\n", __FUNCTION__));
    goto Done;
    return Status;
  }

  //
  // Update HiiSample.vfr/formid=1/text.STR_CPU_SPEED_VALUE
  //
  Status = Formset1TextWhoIAm(mPrivateData.Handle, mPrivateData.Hii.Handle);
  if (EFI_ERROR (Status)) {
    goto Done;
    return Status;
  }
  //
  // Formset Variable Init
  //
  Status = FormsetVarstoreInit(
             &mFormset1Guid,
             FORMSET1_SETUP_NAME_VARSTORE_EFI,
             &mPrivateData.Setup.Formset1SetupVarstoreEfi,
             sizeof(FORMSET1_SETUP_TYPE_VARSTORE_EFI),
             mPrivateData.Handle
             );
  if (EFI_ERROR (Status)) {
    HiiSampleUnload(mPrivateData.Handle);
    return Status;
  }
  Status = FormsetVarstoreInit(
             &mFormset1Guid,
             FORMSET1_SETUP_NAME_VARSTORE,
             &mPrivateData.Setup.Formset1SetupVarstore,
             sizeof(FORMSET1_SETUP_TYPE_VARSTORE),
             mPrivateData.Handle
             );
  if (EFI_ERROR (Status)) {
    HiiSampleUnload(mPrivateData.Handle);
    return Status;
  }

  AttributeQuestionInit(mPrivateData.Hii.Handle, &mFormset1Guid);

Done:
  HiiSampleUnload(mPrivateData.Handle);
  return Status;
}
