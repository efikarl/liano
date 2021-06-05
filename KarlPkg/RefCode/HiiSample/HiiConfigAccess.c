/** @file
  Copyright ©2021 Liu Yi, efikarl@yeah.net

  This program is just made available under the terms and conditions of the
  MIT license: http://www.efikarl.com/mit-license.html

  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
--*/

#include "HiiSample.h"
#include "Formset1.h"

VOID
DebugRequest (
  IN CONST  EFI_STRING                      ConfigRequest,
  IN CONST  EFI_GUID                        *FormsetGuid,
  IN CONST  CHAR16                          *VariableName,
  IN CONST  VOID                            *VariableData,
  IN CONST  UINTN                            VariableSize
) {
  DEBUG ((DEBUG_INFO, "@ConfigRequest: %s\n", ConfigRequest));
  DEBUG ((DEBUG_INFO, "Variable:\n"
    "  GUID: %g\n"
    "  Name: %s\n"
    "  Data: %p\n"
    "  Size: %d\n",
    FormsetGuid, VariableName, VariableData, VariableSize
    ));
}

EFI_STATUS
VarstoreHiiConfigAccessExtractConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results,
  IN        EFI_GUID                        *FormsetGuid,
  IN        CHAR16                          *VariableName,
  IN OUT    VOID                            *VariableData,
  IN        UINTN                            VariableSize
) {
  EFI_STATUS                                Status;
  HII_SAMPLE_PRIVATE_DATA                   *PrivateData;
  UINTN                                     OnBehalfOfVariableSize;

  DEBUG ((DEBUG_INFO, "%a().+\n", __FUNCTION__));
  if (Progress == NULL || Results == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  DebugRequest(Request, FormsetGuid, VariableName, VariableData, VariableSize);
  //
  // Initialize the local variables.
  //
  PrivateData       = HII_SAMPLE_PRIVATE_DATA_FROM_HCAP (This);
  *Progress         = Request;

  if (Request == NULL) {
    //
    // Request is set to NULL, construct full request string.
    //
    DEBUG ((DEBUG_INFO, "Request == NULL\n"));
  } else {
    //
    // Check whether request for efivarstore.
    // efivarstore get data through hii database, not support in this path.
    //
    if (HiiIsConfigHdrMatch(Request, FormsetGuid, FORMSET1_SETUP_NAME_VARSTORE_EFI)) {
      DEBUG ((DEBUG_INFO, "EFI_ERROR: HiiIsConfigHdrMatch efivarstore\n"));
      Status = EFI_UNSUPPORTED;
      goto Done;
    }
  }
  //
  // Get Buffer Storage Data from EFI Variable.
  //
  OnBehalfOfVariableSize  = VariableSize;
  Status = gRT->GetVariable (VariableName, FormsetGuid, NULL, &OnBehalfOfVariableSize, VariableData);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "EFI_ERROR: gRT->GetVariable\n"));
    goto Done;
  }
  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  if (StrStr (Request, L"OFFSET") == NULL) {
    DEBUG ((DEBUG_INFO, "EFI_IFR_VARSTORE_NAME_VALUE\n"));
  } else {
    Status = PrivateData->Hii.ConfigRouting->BlockToConfig (PrivateData->Hii.ConfigRouting, Request, (UINT8 *) VariableData, VariableSize, Results, Progress);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "EFI_ERROR: BlockToConfig\n"));
    }
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

Done:
  if (*Progress) {
    DEBUG ((DEBUG_INFO, "Progress: %s\n", *Progress));
  }
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

EFI_STATUS
EFIAPI
VarstoreHiiConfigAccessRouteConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Config,
  OUT       EFI_STRING                      *Progress,
  IN        EFI_GUID                        *FormsetGuid,
  IN        CHAR16                          *VariableName,
  IN OUT    VOID                            *VariableData,
  IN        UINTN                            VariableSize
  )
{
  EFI_STATUS                                Status;
  HII_SAMPLE_PRIVATE_DATA                   *PrivateData;
  UINTN                                     OnBehalfOfVariableSize;

  DEBUG ((DEBUG_INFO, "%a().+\n", __FUNCTION__));
  if (Config == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DebugRequest(Config, FormsetGuid, VariableName, VariableData, VariableSize);
  //
  // Check whether request for efivarstore.
  // efivarstore set data through hii database, not support in this path.
  //
  if (HiiIsConfigHdrMatch(Config, FormsetGuid, FORMSET1_SETUP_NAME_VARSTORE_EFI)) {
      DEBUG ((DEBUG_INFO, "EFI_ERROR: HiiIsConfigHdrMatch efivarstore\n"));
      Status = EFI_UNSUPPORTED;
      goto Done;
  }

  PrivateData = HII_SAMPLE_PRIVATE_DATA_FROM_HCAP (This);
  //
  // Get Buffer Storage data from EFI variable
  //
  OnBehalfOfVariableSize  = VariableSize;
  Status = gRT->GetVariable (VariableName, FormsetGuid, NULL, &OnBehalfOfVariableSize, VariableData);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "EFI_ERROR: gRT->GetVariable\n"));
    goto Done;
  }
  //
  // Convert <ConfigResp> to buffer data by helper function ConfigToBlock()
  //
  *Progress               = Config;
  OnBehalfOfVariableSize  = VariableSize;
  Status = PrivateData->Hii.ConfigRouting->ConfigToBlock (PrivateData->Hii.ConfigRouting, Config, VariableData, &OnBehalfOfVariableSize, Progress);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "EFI_ERROR: ConfigToBlock\n"));
    goto Done;
  }
  //
  // Store Buffer Storage back to EFI variable
  //
  Status = gRT->SetVariable (VariableName, FormsetGuid, EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS, VariableSize, VariableData);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "EFI_ERROR: gRT->SetVariable\n"));
  }

Done:
  if (*Progress) {
    DEBUG ((DEBUG_INFO, "Progress: %s\n", *Progress));
  }
  DEBUG ((DEBUG_INFO, "%a().- Status = %r\n", __FUNCTION__, Status));
  return Status;
}

/**

  This function allows the caller to request the current
  configuration for one or more named elements. The resulting
  string is in <ConfigAltResp> format. Any and all alternative
  configuration strings shall also be appended to the end of the
  current configuration string. If they are, they must appear
  after the current configuration. They must contain the same
  routing (GUID, NAME, PATH) as the current configuration string.
  They must have an additional description indicating the type of
  alternative configuration the string represents,
  "ALTCFG=<StringToken>". That <StringToken> (when
  converted from Hex UNICODE to binary) is a reference to a
  string in the associated string pack.

  @param This       Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.

  @param Request    A null-terminated Unicode string in
                    <ConfigRequest> format. Note that this
                    includes the routing information as well as
                    the configurable name / value pairs. It is
                    invalid for this string to be in
                    <MultiConfigRequest> format.
                    If a NULL is passed in for the Request field,
                    all of the settings being abstracted by this function
                    will be returned in the Results field.  In addition,
                    if a ConfigHdr is passed in with no request elements,
                    all of the settings being abstracted for that particular
                    ConfigHdr reference will be returned in the Results Field.

  @param Progress   On return, points to a character in the
                    Request string. Points to the string's null
                    terminator if request was successful. Points
                    to the most recent "&" before the first
                    failing name / value pair (or the beginning
                    of the string if the failure is in the first
                    name / value pair) if the request was not
                    successful.

  @param Results    A null-terminated Unicode string in
                    <MultiConfigAltResp> format which has all values
                    filled in for the names in the Request string.
                    String to be allocated by the called function.

  @retval EFI_SUCCESS             The Results string is filled with the
                                  values corresponding to all requested
                                  names.

  @retval EFI_OUT_OF_RESOURCES    Not enough memory to store the
                                  parts of the results that must be
                                  stored awaiting possible future
                                  protocols.

  @retval EFI_NOT_FOUND           Routing data doesn't match any
                                  known driver. Progress set to the
                                  first character in the routing header.
                                  Note: There is no requirement that the
                                  driver validate the routing data. It
                                  must skip the <ConfigHdr> in order to
                                  process the names.

  @retval EFI_INVALID_PARAMETER   Illegal syntax. Progress set
                                  to most recent "&" before the
                                  error or the beginning of the
                                  string.

  @retval EFI_INVALID_PARAMETER   Unknown name. Progress points
                                  to the & before the name in
                                  question.

**/
EFI_STATUS
EFIAPI
HiiSampleHiiConfigAccessExtractConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results
  )
{
  EFI_STATUS                                Status;
  HII_SAMPLE_PRIVATE_DATA                   *PrivateData;

  PrivateData = HII_SAMPLE_PRIVATE_DATA_FROM_HCAP (This);

  Status = VarstoreHiiConfigAccessExtractConfig (
             This, Request, Progress, Results,
             &mFormset1Guid, FORMSET1_SETUP_NAME_VARSTORE, &PrivateData->Setup.Formset1SetupVarstore, sizeof(FORMSET1_SETUP_TYPE_VARSTORE)
             );
  if (!EFI_ERROR(Status)) {
    return Status;
  }

  return Status;
}

/**

  This function applies changes in a driver's configuration.
  Input is a Configuration, which has the routing data for this
  driver followed by name / value configuration pairs. The driver
  must apply those pairs to its configurable storage. If the
  driver's configuration is stored in a linear block of data
  and the driver's name / value pairs are in <BlockConfig>
  format, it may use the ConfigToBlock helper function (above) to
  simplify the job.

  @param This           Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.

  @param Configuration  A null-terminated Unicode string in
                        <ConfigString> format.

  @param Progress       A pointer to a string filled in with the
                        offset of the most recent '&' before the
                        first failing name / value pair (or the
                        beginn ing of the string if the failure
                        is in the first name / value pair) or
                        the terminating NULL if all was
                        successful.

  @retval EFI_SUCCESS             The results have been distributed or are
                                  awaiting distribution.

  @retval EFI_OUT_OF_RESOURCES    Not enough memory to store the
                                  parts of the results that must be
                                  stored awaiting possible future
                                  protocols.

  @retval EFI_INVALID_PARAMETERS  Passing in a NULL for the
                                  Results parameter would result
                                  in this type of error.

  @retval EFI_NOT_FOUND           Target for the specified routing data
                                  was not found

**/
EFI_STATUS
EFIAPI
HiiSampleHiiConfigAccessRouteConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Configuration,
  OUT       EFI_STRING                      *Progress
  )
{
  EFI_STATUS                                Status;
  HII_SAMPLE_PRIVATE_DATA                   *PrivateData;

  PrivateData = HII_SAMPLE_PRIVATE_DATA_FROM_HCAP (This);

  Status = VarstoreHiiConfigAccessRouteConfig (
             This, Configuration, Progress,
             &mFormset1Guid, FORMSET1_SETUP_NAME_VARSTORE, &PrivateData->Setup.Formset1SetupVarstore, sizeof(FORMSET1_SETUP_TYPE_VARSTORE)
             );
  if (!EFI_ERROR(Status)) {
    return Status;
  }

  return Status;
}

/**

  This function is called to provide results data to the driver.
  This data consists of a unique key that is used to identify
  which data is either being passed back or being asked for.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Action                 Specifies the type of action taken by the browser.
  @param  QuestionId             A unique value which is sent to the original
                                 exporting driver so that it can identify the type
                                 of data to expect. The format of the data tends to
                                 vary based on the opcode that generated the callback.
  @param  Type                   The type of value for the question.
  @param  Value                  A pointer to the data being sent to the original
                                 exporting driver.
  @param  ActionRequest          On return, points to the action requested by the
                                 callback function.

  @retval EFI_SUCCESS            The callback successfully handled the action.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the
                                 variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be saved.
  @retval EFI_UNSUPPORTED        The specified Action is not supported by the
                                 callback.
**/
EFI_STATUS
EFIAPI
HiiSampleHiiConfigAccessCallback (
  IN     CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                     Action,
  IN     EFI_QUESTION_ID                        QuestionId,
  IN     UINT8                                  Type,
  IN OUT EFI_IFR_TYPE_VALUE                     *Value,
  OUT    EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                                    Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "%a().+\n", __FUNCTION__));

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE))
    ||(ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG((DEBUG_INFO, "Action = %d, QuestionId = %04x\n", Action, QuestionId));
  switch (Action) {
  case EFI_BROWSER_ACTION_CHANGING:
    break;
  case EFI_BROWSER_ACTION_CHANGED:
    switch (QuestionId) {
    case FORM2_KEY_STR_EXIT:
      DEBUG((DEBUG_INFO, "----------> QuestionId = %04x\n", QuestionId));
      //
      // User press KEY_STR_EXIT, request Browser to exit
      //
      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
      break;
    case FORM2_KEY_STR_SAVE:
      DEBUG((DEBUG_INFO, "----------> QuestionId = %04x\n", QuestionId));
      //
      // User press KEY_STR_SAVE, request Browser to save
      //
      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_SUBMIT;
      break;
    case FORM2_KEY_STR_FORM_ASIS_AND_EXIT:
      DEBUG((DEBUG_INFO, "----------> QuestionId = %04x\n", QuestionId));
      //
      // User press STR_FORM_SAVE_AND_EXIT, request Browser to discard current form and exit.
      //
      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD_EXIT;
      break;
    case FORM2_KEY_STR_FORM_SAVE_AND_EXIT:
      DEBUG((DEBUG_INFO, "----------> QuestionId = %04x\n", QuestionId));
      //
      // User press STR_FORM_ASIS_AND_EXIT, request Browser to  submit current form and exit
      //
      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_SUBMIT_EXIT;
      break;
    default:
      Status = EFI_UNSUPPORTED;
      break;
    }
    break;
  case EFI_BROWSER_ACTION_RETRIEVE:
    break;
  case EFI_BROWSER_ACTION_FORM_OPEN:
    break;
  case EFI_BROWSER_ACTION_FORM_CLOSE:
    break;
  case EFI_BROWSER_ACTION_SUBMITTED:
    break;
  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  DEBUG ((DEBUG_INFO, "%a().-\n", __FUNCTION__));

  return Status;
}
