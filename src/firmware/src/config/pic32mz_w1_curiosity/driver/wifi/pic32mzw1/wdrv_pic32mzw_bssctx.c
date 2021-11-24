/*******************************************************************************
  PIC32MZW Driver BSS Context Implementation

  File Name:
    wdrv_pic32mzw_bssctx.c

  Summary:
    PIC32MZW wireless driver BSS context implementation.

  Description:
    This interface manages the BSS contexts which 'wrap' the state
      associated with BSSs.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020-21 released Microchip Technology Inc.  All rights reserved.

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
//DOM-IGNORE-END

#include <stdint.h>
#include <string.h>

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_bssctx.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver BSS Context Implementation
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
    See wdrv_pic32mzw_bssctx.h for usage information.

*/

bool WDRV_PIC32MZW_BSSCtxIsValid
(
    const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    bool ssidValid
)
{
    /* Ensure BSS context is valid. */
    if (NULL == pBSSCtx)
    {
        return false;
    }

    /* Ensure the channels are valid. */
    if (pBSSCtx->channel > WDRV_PIC32MZW_CID_2_4G_CH13)
    {
        return false;
    }

    /* Ensure the SSID length is valid. */
    if (pBSSCtx->ssid.length > WDRV_PIC32MZW_MAX_SSID_LEN)
    {
        return false;
    }

    /* Ensure the BSSID is non-zero, if valid. */
    if (true == pBSSCtx->bssid.valid)
    {
        int i;
        uint8_t macAddrChk;

        macAddrChk = 0;
        for (i=0; i<WDRV_PIC32MZW_MAC_ADDR_LEN; i++)
        {
            macAddrChk |= pBSSCtx->bssid.addr[i];
        }

        if (0 == macAddrChk)
        {
            return false;
        }
    }

    if (true == ssidValid)
    {
        /* If requested, check the SSID is present. */
        if (0 == pBSSCtx->ssid.length)
        {
            return false;
        }
    }

    return true;
}

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

  Remarks:
    See wdrv_pic32mzw_bssctx.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetDefaults
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx
)
{
    /* Ensure BSS context is valid. */
    if (NULL == pBSSCtx)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    memset(pBSSCtx->bssid.addr, 0, WDRV_PIC32MZW_MAC_ADDR_LEN);
    pBSSCtx->bssid.valid = false;

    /* Set context to have no SSID, all channels and not cloaked. */
    pBSSCtx->ssid.length = 0;
    pBSSCtx->channel     = WDRV_PIC32MZW_CID_ANY;
    pBSSCtx->cloaked     = false;

    return WDRV_PIC32MZW_STATUS_OK;
}

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
    The SSID string and length provided the SSID is copied into the BSS
      context.

  Remarks:
    See wdrv_pic32mzw_bssctx.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetSSID
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    uint8_t *const pSSID,
    uint8_t ssidLength
)
{
    /* Ensure BSS context and SSID buffer and length are valid. */
    if ((NULL == pBSSCtx) || (NULL == pSSID) || (ssidLength > WDRV_PIC32MZW_MAX_SSID_LEN))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Copy the SSID ensure unused space is zeroed. */
    memset(&pBSSCtx->ssid.name, 0, WDRV_PIC32MZW_MAX_SSID_LEN);
    memcpy(&pBSSCtx->ssid.name, pSSID, ssidLength);
    pBSSCtx->ssid.length = ssidLength;

    /* Validate context. */
    if (false == WDRV_PIC32MZW_BSSCtxIsValid(pBSSCtx, false))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_CONTEXT;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

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

  Remarks:
    See wdrv_pic32mzw_bssctx.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetBSSID
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    uint8_t *const pBSSID
)
{
    /* Ensure BSS context is valid. */
    if (NULL == pBSSCtx)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    if (NULL != pBSSID)
    {
        /* Copy the BSSID. */
        memcpy(&pBSSCtx->bssid, pBSSID, WDRV_PIC32MZW_MAC_ADDR_LEN);
        pBSSCtx->bssid.valid = true;
    }
    else
    {
        memset(&pBSSCtx->bssid, 0, WDRV_PIC32MZW_MAC_ADDR_LEN);
        pBSSCtx->bssid.valid = false;
    }

    /* Validate context. */
    if (false == WDRV_PIC32MZW_BSSCtxIsValid(pBSSCtx, false))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_CONTEXT;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

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

  Remarks:
    See wdrv_pic32mzw_bssctx.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetChannel
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    WDRV_PIC32MZW_CHANNEL_ID channel
)
{
    /* Ensure BSS context and channels are valid. */
    if ((NULL == pBSSCtx) || (channel > WDRV_PIC32MZW_CID_2_4G_CH13))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Copy channel. */
    pBSSCtx->channel = channel;

    /* Validate context. */
    if (false == WDRV_PIC32MZW_BSSCtxIsValid(pBSSCtx, false))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_CONTEXT;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

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

  Remarks:
    See wdrv_pic32mzw_bssctx.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSCtxSetSSIDVisibility
(
    WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    bool visible
)
{
    /* Ensure BSS context is valid. */
    if (NULL == pBSSCtx)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Set cloaked state. */
    pBSSCtx->cloaked = visible ? false : true;

    /* Validate context. */
    if (false == WDRV_PIC32MZW_BSSCtxIsValid(pBSSCtx, false))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_CONTEXT;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}
