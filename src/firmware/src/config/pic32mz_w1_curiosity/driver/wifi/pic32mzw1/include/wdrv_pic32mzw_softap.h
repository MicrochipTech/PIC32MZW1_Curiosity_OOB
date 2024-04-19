/*******************************************************************************
  PIC32MZW Driver Soft-AP Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_softap.h

  Summary:
    PIC32MZW wireless driver Soft-AP header file.

  Description:
    Provides an interface to create and manage a Soft-AP.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*
Copyright (C) 2020-2023, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
// DOM-IGNORE-END

#ifndef _WDRV_PIC32MZW_SOFTAP_H
#define _WDRV_PIC32MZW_SOFTAP_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_bssctx.h"
#include "wdrv_pic32mzw_authctx.h"
#include "wdrv_pic32mzw_custie.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Soft-AP Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APStart
    (
        DRV_HANDLE handle,
        const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
        const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
        const WDRV_PIC32MZW_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Starts an instance of Soft-AP.

  Description:
    Using the defined BSS and authentication contexts with an optional HTTP
      provisioning context (socket mode only) this function creates and starts
      a Soft-AP instance.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
    A BSS context must have been created and initialized.
    An authentication context must have been created and initialized.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pBSSCtx          - Pointer to BSS context.
    pAuthCtx         - Pointer to authentication context.
    pfNotifyCallback - Pointer to notification callback function.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR   - The request to the PIC32MZW was rejected.
    WDRV_PIC32MZW_STATUS_INVALID_CONTEXT - The BSS context is not valid.

  Remarks:
    If pBSSCtx and pAuthCtx are both NULL then no AP will be established, however the
    pfNotifyCallback callback will still be accepted, even if an AP is active.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APStart
(
    DRV_HANDLE handle,
    const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    const WDRV_PIC32MZW_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APStop(DRV_HANDLE handle)

  Summary:
    Stops an instance of Soft-AP.

  Description:
    Stops an instance of Soft-AP.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_PIC32MZW_Open.

  Returns:
    WDRV_PIC32MZW_STATUS_OK            - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR - The request to the PIC32MZW was rejected.

  Remarks:
    The AP stopping will be confirmed via the notification callback registered
    by WDRV_PIC32MZW_APStart. The callback will receive the association handle
    WDRV_PIC32MZW_ASSOC_HANDLE_ALL to signify that all STA associations have been
    disconnected.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APStop(DRV_HANDLE handle);

//*******************************************************************************
/*
  Function:
   WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APRekeyIntervalSet
   (
       DRV_HANDLE handle,
       const uint32_t interval
   )

  Summary:
    Configures the group re-key interval used when operating in Soft-AP mode

  Description:
    The re-key interval specifies how much time must elapse before a group re-key
    is initiated with connected stations.
    The timer is restarted after each group re-key.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    interval - The time in seconds that must pass before each re-key attempt.
               The minimum time value is 60 seconds.
               Defaults to 86400.

  Returns:
    WDRV_PIC32MZW_STATUS_OK            - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR - The request to the PIC32MZW was rejected.

  Remarks:
    Takes effect after the next re-key - if an interval other than the default is
    desired then it is recommended to call this API before calling
    WDRV_PIC32MZW_APStart.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APRekeyIntervalSet(
    DRV_HANDLE handle,
    const uint32_t interval
);

#endif /* _WDRV_PIC32MZW_SOFTAP_H */
