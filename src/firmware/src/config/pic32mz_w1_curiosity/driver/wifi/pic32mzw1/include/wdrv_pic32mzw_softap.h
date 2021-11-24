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
/*******************************************************************************
Copyright (C) 2020-21 released Microchip Technology Inc. All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
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
    None.

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
    None.

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

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APSetCustIE
    (
        DRV_HANDLE handle,
        const WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx
    )

  Summary:
    Configures the custom IE.

  Description:
    Soft-AP beacons may contain an application provided custom IE. This function
    associates a custom IE store context with the Soft-AP instance.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle     - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pCustIECtx - Pointer to custom IE store context.

  Returns:
    WDRV_PIC32MZW_STATUS_OK            - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR - The request to the PIC32MZW was rejected.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APSetCustIE
(
    DRV_HANDLE handle,
    const WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx
);

#endif /* _WDRV_PIC32MZW_SOFTAP_H */
