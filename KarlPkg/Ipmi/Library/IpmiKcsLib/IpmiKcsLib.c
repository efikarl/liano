
#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Protocol/IpmiKcsProtocol.h>

STATIC IPMI_KCS_PROTOCOL  *mKcsProtocol = NULL;

EFI_STATUS
EFIAPI
IpmiKcsCommand (
  IN      IPMI_KCS_PROTOCOL   *This,
  IN      KSC_RQ              *KcsRq,
  OUT     KSC_RS              *KcsRs
)
/*++

Routine Description:

  Wapper of kcs interface for  EASY TO USE

Arguments:

  This              - Pointer to IPMI protocol instance
  NetFn             - Net Function of request message
  Cmd               - Command of request message
  RequestData       - Pointer to request data buffer
  RequestSize       - Size of request data buffer
  ResponseData      - Pointer to response data buffer
  ResponseSize      - Size of response data buffer

Returns:

  EFI_SUCCESS           - Ipmi message completes successfully
  EFI_INVALID_PARAMETER - One of the input parameters is invalid
  EFI_BUFFER_TOO_SMALL  - Response buffer is too small
  EFI_TIME_OUT          - Timeout when waiting for a BMC status
  EFI_DEVICE_ERROR      - Ipmi inferce error during message

--*/
{
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (
                  &gEfiIpmiKcsProtocolGuid,
                  NULL,
                  (VOID **) &mKcsProtocol
                  );
  ASSERT (Status);

  Status = mKcsProtocol->IpmiCommand(
                           This,
                           NetFn,
                           Cmd,
                           RequestData,
                           RequestSize,
                           ResponseData,
                           ResponseSize
                           );

  return Status;
}

