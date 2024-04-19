/*******************************************************************************
  PIC32MZW Driver STA Implementation

  File Name:
    wdrv_pic32mzw_sta.c

  Summary:
    PIC32MZW wireless driver STA implementation.

  Description:
    PIC32MZW wireless driver STA implementation.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
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
#include "wdrv_pic32mzw_sta.h"
#include "wdrv_pic32mzw_cfg.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver STA Implementation
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

  Remarks:
    See wdrv_pic32mzw_sta.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSConnect
(
    DRV_HANDLE handle,
    const WDRV_PIC32MZW_BSS_CONTEXT *const pBSSCtx,
    const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    const WDRV_PIC32MZW_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
)
{
    WDRV_PIC32MZW_DCPT *pDcpt = (WDRV_PIC32MZW_DCPT *)handle;
    DRV_PIC32MZW_WIDCTX wids;
    uint8_t channel;
    DRV_PIC32MZW_11I_MASK dot11iInfo = 0;
    uint8_t filter = 0;
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* Ensure the driver handle and user pointer is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    if ((NULL == pBSSCtx) && (NULL == pAuthCtx))
    {
        /* Allow callback to be set/changed, but only if not trying to change
         BSS/Auth settings. */
        pDcpt->pCtrl->pfConnectNotifyCB = pfNotifyCallback;
        return WDRV_PIC32MZW_STATUS_OK;
    }

    /* Ensure RF and MAC is configured */
    if (WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG != (pDcpt->pCtrl->rfMacConfigStatus & WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG))
    {
        return WDRV_PIC32MZW_STATUS_RF_MAC_CONFIG_NOT_VALID;
    }

    /* Ensure the BSS context is valid. */
    if (false == WDRV_PIC32MZW_BSSCtxIsValid(pBSSCtx, false))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_CONTEXT;
    }

    /* NULL authentication context is OK - no encryption. */
    if (NULL != pAuthCtx)
    {
        /* Ensure the authentication context is valid. */
        if (false == WDRV_PIC32MZW_AuthCtxIsValid(pAuthCtx))
        {
            return WDRV_PIC32MZW_STATUS_INVALID_CONTEXT;
        }

        /* Convert authentication type to an 11i bitmap. */
        dot11iInfo = DRV_PIC32MZW_Get11iMask(
                pAuthCtx->authType,
                pAuthCtx->authMod & ~WDRV_PIC32MZW_AUTH_MOD_AP_TD);
    }

    channel = pBSSCtx->channel;

    /* Convert public API representation of all channels to
       internal representation. */
    if (WDRV_PIC32MZW_CID_ANY == channel)
    {
        channel = 0xff;
    }
    else
    {
        if(!((1<<(channel-1)) & pDcpt->pCtrl->scanChannelMask24))
        {
            return WDRV_PIC32MZW_STATUS_INVALID_ARG;
        }

        filter |= 0x10;
    }

    /* Ensure PIC32MZW is not configured for Soft-AP. */
    if (false != pDcpt->pCtrl->isAP)
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    /* Ensure PIC32MZW is not connected or attempting to connect. */
    if (WDRV_PIC32MZW_CONN_STATE_DISCONNECTED != pDcpt->pCtrl->connectedState)
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 512);

    /* Switch to STA mode (0). */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SWITCH_MODE, 0);

    /* Set Transmit Rate to Autorate */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_CURRENT_TX_RATE, 0);

    /* Set BSSID if provided. */
    if (true == pBSSCtx->bssid.valid)
    {
        DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_BSSID, pBSSCtx->bssid.addr, 6);
    }

    /* Set SSID. */
    DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_SSID, pBSSCtx->ssid.name, pBSSCtx->ssid.length);

    /* Set scan filter */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SCAN_FILTER, filter);

    /* Set channel to be use for scan & connect */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_USER_SCAN_CHANNEL, channel);

    /* Set 11i info as derived above. */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_11I_SETTINGS, (uint32_t)dot11iInfo);

    /* Set credentials for whichever authentication types are enabled. */
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
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
    if (dot11iInfo & DRV_PIC32MZW_11I_1X)
    {
        /* Initialize TLS stack with the WOLFSSL_CTX handle passed */
        pDcpt->pCtrl->tlsHandle = DRV_PIC32MZW1_TLS_Init(pAuthCtx->authInfo.enterprise.phase1.tlsCtxHandle,
                pAuthCtx->authInfo.enterprise.phase1.serverDomainName);
        if (DRV_PIC32MZW1_TLS_HANDLE_INVALID == pDcpt->pCtrl->tlsHandle)
        {
            /* Failed to initialize TLS module */
            DRV_PIC32MZW_MultiWIDDestroy(&wids);

            return WDRV_PIC32MZW_STATUS_CONNECT_FAIL;
        }

        /* set the EAP domainUsername */
        DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_SUPP_DOMAIN_USERNAME,
            (uint8_t *) pAuthCtx->authInfo.enterprise.phase1.identity,
            (uint16_t) strlen(pAuthCtx->authInfo.enterprise.phase1.identity));

        /* Set the 802.1x EAP method */
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SUPP_1X_AUTH_METHOD, (uint16_t) pAuthCtx->authInfo.enterprise.auth1xMethod);

        if (WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTTLSv0_MSCHAPv2 == pAuthCtx->authInfo.enterprise.auth1xMethod)
        {
            /* Set the MSCHAPv2 username and password */
            DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_SUPP_USERNAME,
                (uint8_t *) pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.username,
                (uint16_t) strlen(pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.username));

            DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_SUPP_PASSWORD,
                (uint8_t *) pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.password,
                (uint16_t) strlen(pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.password));
        }
    }
#endif

    /* Set 11g compatibility mode 1 (2). */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_11G_OPERATING_MODE, 2);

    /* Set Ack policy: Normal Ack (0). */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_ACK_POLICY, 0);

    /* Set 11n enabled (1). */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_11N_ENABLE, 1);

    /* Set short preamble to auto selection mode */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_PREAMBLE, 2);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the WIDs. */
    if (false == DRV_PIC32MZW_MultiWIDWrite(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        return WDRV_PIC32MZW_STATUS_CONNECT_FAIL;
    }

    pDcpt->pCtrl->pfConnectNotifyCB = pfNotifyCallback;
    pDcpt->pCtrl->connectedState    = WDRV_PIC32MZW_CONN_STATE_CONNECTING;

    pDcpt->pCtrl->assocInfoSTA.handle = DRV_HANDLE_INVALID;
    pDcpt->pCtrl->assocInfoSTA.rssi   = 0;
    pDcpt->pCtrl->assocInfoSTA.authType = pAuthCtx->authType;
    pDcpt->pCtrl->assocInfoSTA.transitionDisable = false;
    pDcpt->pCtrl->assocInfoSTA.peerAddress.valid = false;
    pDcpt->pCtrl->assocInfoSTA.assocID = 1;

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSDisconnect(DRV_HANDLE handle)

  Summary:
    Disconnects from a BSS.

  Description:
    Disconnects from an existing BSS.

  Remarks:
    See wdrv_pic32mzw_sta.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSDisconnect(DRV_HANDLE handle)
{
    WDRV_PIC32MZW_DCPT *pDcpt = (WDRV_PIC32MZW_DCPT *)handle;
    DRV_PIC32MZW_WIDCTX wids;
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Ensure PIC32MZW is connected or attempting to connect. */
    if (WDRV_PIC32MZW_CONN_STATE_DISCONNECTED == pDcpt->pCtrl->connectedState)
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    /* Disconnect PIC32MZW. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 16);
    DRV_PIC32MZW_MultiWIDAddString(&wids, DRV_WIFI_WID_SSID, "\0");
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_DISCONNECT, 1);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    if (false == DRV_PIC32MZW_MultiWIDWrite(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        return WDRV_PIC32MZW_STATUS_DISCONNECT_FAIL;
    }

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}
