/*******************************************************************************
  PIC32MZW Driver STA Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_sta.h

  Summary:
    PIC32MZW wireless driver STA header file.

  Description:
    PIC32MZW wireless driver STA header file.
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

#ifndef _WDRV_PIC32MZW_STA_H
#define _WDRV_PIC32MZW_STA_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_bssctx.h"
#include "wdrv_pic32mzw_authctx.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

#ifdef WDRV_PIC32MZW_DEVICE_BSS_ROAMING
// *****************************************************************************
/*  BSS Roaming Configuration

  Summary:
    Defines the BSS roaming configuration.

  Description:
    This enumeration defines the BSS roaming configuration.

  Remarks:
    None.
*/
typedef enum
{
    /* BSS Roaming is turned off. */
    WDRV_PIC32MZW_BSS_ROAMING_CFG_OFF,

    /* BSS Roaming is turned on, no IP renew occurs. */
    WDRV_PIC32MZW_BSS_ROAMING_CFG_ON,

    /* BSS Roaming is turned on, DHCP renew is request upon reconnection. */
    WDRV_PIC32MZW_BSS_ROAMING_CFG_ON_IP_RENEW
} WDRV_PIC32MZW_BSS_ROAMING_CFG;
#endif

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver STA Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSConnect
    (
        DRV_HANDLE handle,
        const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
        const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
        const WDRV_PIC32MZW_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Connects to a BSS in infrastructure station mode.

  Description:
    Using the defined BSS and authentication contexts this function requests
      the PIC32MZW connect to the BSS as an infrastructure station.

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
    WDRV_PIC32MZW_STATUS_CONNECT_FAIL    - The connection has failed.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSConnect
(
    DRV_HANDLE handle,
    const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    const WDRV_PIC32MZW_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSDisconnect(DRV_HANDLE handle)

  Summary:
    Disconnects from a BSS.

  Description:
    Disconnects from an existing BSS.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_PIC32MZW_Open.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_DISCONNECT_FAIL - The disconnection has failed.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR   - The request to the PIC32MZW was rejected.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSDisconnect(DRV_HANDLE handle);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSRoamingConfigure
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_BSS_ROAMING_CFG roamingCfg
    )

  Summary:
    Configures BSS roaming support.

  Description:
    Enables or disables BSS roaming support. If enabled the PIC32MZW can perform
      a DHCP renew of the current IP address if configured to do so, otherwise
      it will assume the existing IP address is still valid.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle     - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    roamingCfg - Roaming configuration, see WDRV_PIC32MZW_BSS_ROAMING_CFG.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_CONNECT_FAIL    - The disconnection has failed.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR   - The request to the PIC32MZW was rejected.

  Remarks:
    None.

*/

#ifdef WDRV_PIC32MZW_DEVICE_BSS_ROAMING
WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSRoamingConfigure
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_BSS_ROAMING_CFG roamingCfg
);
#endif

#endif /* _WDRV_PIC32MZW_STA_H */
