/*******************************************************************************
  PIC32MZW Driver BSS Context Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_bssctx.h

  Summary:
    PIC32MZW wireless driver BSS context header file.

  Description:
    This interface manages the BSS contexts which 'wrap' the state
      associated with BSSs.
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

#ifndef _WDRV_PIC32MZW_BSSCTX_H
#define _WDRV_PIC32MZW_BSSCTX_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver BSS Context Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  BSS Context

  Summary:
    Context structure containing information about BSSs.

  Description:
    The context contains the SSID, channel and visibility of a BSS.

  Remarks:
    None.
*/

typedef struct
{
    /* SSID of BSS. */
    WDRV_PIC32MZW_SSID ssid;

    /* BSSID. */
    WDRV_PIC32MZW_MAC_ADDR bssid;

    /* Current channel of BSS. */
    WDRV_PIC32MZW_CHANNEL_ID channel;

    /* Flag indicating if SSID is cloaked. */
    bool cloaked;
} WDRV_PIC32MZW_BSS_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver BSS Context Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_BSSCtxIsValid
    (
        const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
        bool ssidValid
    )

  Summary:
    Tests if a BSS context is valid.

  Description:
    Tests the elements of the BSS context to judge if their values are legal.

  Precondition:
    None.

  Parameters:
    pBSSCtx   - Pointer to a BSS context.
    ssidValid - Flag indicating if the SSID within the context must be valid.

  Returns:
    true or false indicating if context is valid.

  Remarks:
    A valid SSID is one which has a non-zero length. The check is optional as
      it is legal for the SSID field to be zero length.

*/

bool WDRV_PIC32MZW_BSSCtxIsValid
(
    const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    bool ssidValid
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetDefaults
    (
        WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx
    )

  Summary:
    Configures a BSS context into a default legal state.

  Description:
    Ensures that each element of the structure is configured into a legal state.

  Precondition:
    None.

  Parameters:
    pBSSCtx - Pointer to a BSS context.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetDefaults
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetSSID
    (
        WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
        uint8_t *const pSSID,
        uint8_t ssidLength
    )

  Summary:
    Configures the SSID of the BSS context.

  Description:
    The SSID string and length provided are copied into the BSS context.

  Precondition:
    None.

  Parameters:
    pBSSCtx    - Pointer to a BSS context.
    pSSID      - Pointer to buffer containing the new SSID.
    ssidLength - The length of the SSID held in the pSSID buffer.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_INVALID_CONTEXT - The BSS context is not valid.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetSSID
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    uint8_t *const pSSID,
    uint8_t ssidLength
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetBSSID
    (
        WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
        uint8_t *const pBSSID
    )

  Summary:
    Configures the BSSID of the BSS context.

  Description:
    The BSSID string is copied into the BSS context.

  Precondition:
    None.

  Parameters:
    pBSSCtx    - Pointer to a BSS context.
    pBSSID     - Pointer to buffer containing the new BSSID.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_INVALID_CONTEXT - The BSS context is not valid.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetBSSID
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    uint8_t *const pBSSID
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetChannel
    (
        WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
        WDRV_PIC32MZW_CHANNEL_ID channel
    )

  Summary:
    Configures the channel of the BSS context.

  Description:
    The supplied channel value is copied into the BSS context.

  Precondition:
    None.

  Parameters:
    pBSSCtx - Pointer to a BSS context.
    channel - Channel number.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_INVALID_CONTEXT - The BSS context is not valid.

  Remarks:
    channel may be WDRV_PIC32MZW_CID_ANY to represent no fixed channel.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetChannel
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    WDRV_PIC32MZW_CHANNEL_ID channel
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetSSIDVisibility
    (
        WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
        bool visible
    )

  Summary:
    Configures the visibility of the BSS context.

  Description:
    Specific to Soft-AP mode this flag defines if the BSS context will create a
      visible presence on air.

  Precondition:
    None.

  Parameters:
    pBSSCtx - Pointer to a BSS context.
    visible - Flag indicating if this BSS will be visible or not.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_INVALID_CONTEXT - The BSS context is not valid.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetSSIDVisibility
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    bool visible
);

#endif /* _WDRV_PIC32MZW_BSSCTX_H */
