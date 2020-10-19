/*******************************************************************************
  PIC32MZW Driver Soft-AP Implementation

  File Name:
    wdrv_pic32mzw_softap.c

  Summary:
    PIC32MZW wireless driver Soft-AP implementation.

  Description:
    Provides an interface to create and manage a Soft-AP.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

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

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>

#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_softap.h"
#include "wdrv_pic32mzw_cfg.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Soft-AP Implementation
// *****************************************************************************
// *****************************************************************************

DRV_PIC32MZW_11I_MASK DRV_PIC32MZW_Get11iMask
(
    WDRV_PIC32MZW_AUTH_TYPE authType,
    WDRV_PIC32MZW_AUTH_MOD_MASK authMod
);

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

  Remarks:
    See wdrv_pic32mzw_softap.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APStart
(
    DRV_HANDLE handle,
    const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    const WDRV_PIC32MZW_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
)
{
    WDRV_PIC32MZW_DCPT *pDcpt = (WDRV_PIC32MZW_DCPT *)handle;
    DRV_PIC32MZW_WIDCTX wids;
    DRV_PIC32MZW_11I_MASK dot11iInfo = 0;
    int i;
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pBSSCtx))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Ensure RF and MAC is configured */
    if (WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG != (pDcpt->pCtrl->rfMacConfigStatus & WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG))
    {
        return WDRV_PIC32MZW_STATUS_RF_MAC_CONFIG_NOT_VALID;
    }

    /* Validate BSS context. */
    if (false == WDRV_PIC32MZW_BSSCtxIsValid(pBSSCtx, false))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_CONTEXT;
    }

    /* NULL auth context is ok - no encryption. */
    if (NULL != pAuthCtx)
    {
        /* Ensure the Auth context is valid. */
        if (false == WDRV_PIC32MZW_AuthCtxIsValid(pAuthCtx))
        {
            return WDRV_PIC32MZW_STATUS_INVALID_CONTEXT;
        }
        /* Convert auth type to 11i bitmap. */
        dot11iInfo = DRV_PIC32MZW_Get11iMask(pAuthCtx->authType, pAuthCtx->authMod);
    }

    /* Indicate that the dot11i settings are intended for AP mode. */
    dot11iInfo |= DRV_PIC32MZW_AP;

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 512);

    /* Switch to AP mode (1). */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SWITCH_MODE, 1);

    /* Set Transmit Rate to Autorate */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_CURRENT_TX_RATE, 0);

    /* Copy SSID and channel. */
    DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_SSID, pBSSCtx->ssid.name, pBSSCtx->ssid.length);

    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_USER_PREF_CHANNEL, pBSSCtx->channel);

    /* Set 11i info as derived above. */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_11I_SETTINGS, (int)dot11iInfo);

    /* Set credentials for whichever auth types are enabled. */
    if (
            (dot11iInfo & DRV_PIC32MZW_PRIVACY)
        &&  !(dot11iInfo & DRV_PIC32MZW_RSNA_MASK)
    )
    {
        /* Set WEP credentials. */
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_KEY_ID, pAuthCtx->authInfo.WEP.idx-1);
        DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_WEP_KEY_VALUE, pAuthCtx->authInfo.WEP.key, pAuthCtx->authInfo.WEP.size);
    }
    if (dot11iInfo & DRV_PIC32MZW_11I_PSK)
    {
        /* Set PSK credentials. */
        DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_11I_PSK,
                pAuthCtx->authInfo.personal.password,
                pAuthCtx->authInfo.personal.size);
    }
    if (dot11iInfo & DRV_PIC32MZW_11I_SAE)
    {
        /* Set SAE credentials. */
        DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_RSNA_PASSWORD,
                pAuthCtx->authInfo.personal.password,
                pAuthCtx->authInfo.personal.size);
    }

    /* Set 11g compatibility mode 1 (2). */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_11G_OPERATING_MODE, 2);
    /* Set Ack policy: Normal Ack (0). */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_ACK_POLICY, 0);
    /* Set 11n enabled (1). */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_11N_ENABLE, 1);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the wids. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);
        return WDRV_PIC32MZW_STATUS_CONNECT_FAIL;
    }

    pDcpt->pCtrl->pfConnectNotifyCB = pfNotifyCallback;
    pDcpt->pCtrl->isAP              = true;

    for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
    {
        pDcpt->pCtrl->assocInfoAP[i].handle = DRV_HANDLE_INVALID;
        pDcpt->pCtrl->assocInfoAP[i].peerAddress.valid = false;
    }

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APStop(DRV_HANDLE handle)

  Summary:
    Stops an instance of Soft-AP.

  Description:
    Stops an instance of Soft-AP.

  Remarks:
    See wdrv_pic32mzw_softap.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_APStop(DRV_HANDLE handle)
{
    WDRV_PIC32MZW_DCPT *pDcpt = (WDRV_PIC32MZW_DCPT *)handle;
    DRV_PIC32MZW_WIDCTX wids;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Ensure operation mode is really Soft-AP. */
    if (false == pDcpt->pCtrl->isAP)
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    DRV_PIC32MZW_MultiWIDInit(&wids, 16);

    /* Switch to STA mode (0) */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SWITCH_MODE, 0);

    /* Clear the SSID value */
    DRV_PIC32MZW_MultiWIDAddString(&wids, DRV_WIFI_WID_SSID, "\0");

    /* Write the wids. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}
