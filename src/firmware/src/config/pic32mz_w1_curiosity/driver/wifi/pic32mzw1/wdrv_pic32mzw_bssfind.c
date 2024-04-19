/*******************************************************************************
  PIC32MZW Wireless Driver

  File Name:
    wdrv_pic32mzw_bssfind.c

  Summary:
    PIC32MZW wireless driver.

  Description:
    PIC32MZW wireless driver.
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
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>
#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_bssfind.h"
#include "wdrv_pic32mzw_cfg.h"

#define WDRV_PIC32MZW_SCAN_RESULT_CACHE_NUM_ENTRIES     100

typedef struct
{
    int                         numDescrs;
    DRV_PIC32MZW_SCAN_RESULTS   bssDescr[WDRV_PIC32MZW_SCAN_RESULT_CACHE_NUM_ENTRIES];
} WDRV_PIC32MZW_SCAN_RESULT_CACHE;

static WDRV_PIC32MZW_SCAN_RESULT_CACHE scanResultCache;

static const WDRV_PIC32MZW_SEC_MASK map11iToSecMask[] = {
    0,                                  //DRV_PIC32MZW_PRIVACY
    0,                                  //DRV_PIC32MZW_SKEY
    WDRV_PIC32MZW_SEC_BIT_WEP,          //DRV_PIC32MZW_11I_WEP
    WDRV_PIC32MZW_SEC_BIT_WEP,          //DRV_PIC32MZW_11I_WEP104
    0,                                  //DRV_PIC32MZW_11I_WPAIE
    0,                                  //DRV_PIC32MZW_11I_RSNE
    WDRV_PIC32MZW_SEC_BIT_WPA2OR3,      //DRV_PIC32MZW_11I_CCMP128
    WDRV_PIC32MZW_SEC_BIT_WPA,          //DRV_PIC32MZW_11I_TKIP
    WDRV_PIC32MZW_SEC_BIT_MFP_CAPABLE,  //DRV_PIC32MZW_11I_BIPCMAC128
    WDRV_PIC32MZW_SEC_BIT_MFP_REQUIRED, //DRV_PIC32MZW_11I_MFP_REQUIRED
    WDRV_PIC32MZW_SEC_BIT_ENTERPRISE,   //DRV_PIC32MZW_11I_1X
    WDRV_PIC32MZW_SEC_BIT_PSK,          //DRV_PIC32MZW_11I_PSK
    WDRV_PIC32MZW_SEC_BIT_SAE           //DRV_PIC32MZW_11I_SAE
};

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver BSS Find Implementations
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindFirst
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_CHANNEL_ID channel,
        bool active,
        const WDRV_PIC32MZW_SSID_LIST *const pSSIDList,
        const WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Request a BSS scan is performed by the PIC32MZW.

  Description:
    A scan is requested on the specified channels. An optional callback can
      be provided to receive notification of the first BSS discovered.

  Remarks:
    See wdrv_pic32mzw_bssfind.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindFirst
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_CHANNEL_ID channel,
    bool active,
    const WDRV_PIC32MZW_SSID_LIST *const pSSIDList,
    const WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    DRV_PIC32MZW_WIDCTX wids;
    uint8_t filter = 0;
    uint8_t ssidList[(DRV_PIC32MZW_MAX_HIDDEN_SITES * (WDRV_PIC32MZW_MAX_SSID_LEN +1)) + 1] = {0};
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure request channel is valid. */
    if (channel > WDRV_PIC32MZW_CID_2_4G_CH13)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure SSID list is only provided for active scans. */
    if ((false == active) && (NULL != pSSIDList))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Ensure RF and MAC is configured */
    if (WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG != (pDcpt->pCtrl->rfMacConfigStatus & WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG))
    {
        return WDRV_PIC32MZW_STATUS_RF_MAC_CONFIG_NOT_VALID;
    }

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

    if (NULL != pSSIDList)
    {
        const WDRV_PIC32MZW_SSID_LIST *pSSIDListEle;
        uint8_t *pSSIDListArrIdx;

        pSSIDListEle = pSSIDList;

        ssidList[0] = 0;
        pSSIDListArrIdx = &ssidList[1];

        /* Construct packed SSID list. */
        while ((NULL != pSSIDListEle) && (ssidList[0] < DRV_PIC32MZW_MAX_HIDDEN_SITES))
        {
            if (pSSIDListEle->ssid.length > (WDRV_PIC32MZW_MAX_SSID_LEN-1))
            {
                return WDRV_PIC32MZW_STATUS_INVALID_ARG;
            }

            *pSSIDListArrIdx++ = pSSIDListEle->ssid.length;
            memcpy(pSSIDListArrIdx, pSSIDListEle->ssid.name, pSSIDListEle->ssid.length);
            pSSIDListArrIdx += pSSIDListEle->ssid.length;

            ssidList[0]++;

            pSSIDListEle = pSSIDListEle->pNext;
        }
    }

    scanResultCache.numDescrs = 0;

    DRV_PIC32MZW_MultiWIDInit(&wids, 512);

    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SCAN_FILTER, filter);
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_USER_SCAN_CHANNEL, channel);

    /* Check if the scan parameters have been updated from
       the defaults. */
    if (false == pDcpt->pCtrl->scanParamDefault)
    {
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SCAN_NUM_SLOTS, pDcpt->pCtrl->scanNumSlots);
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SCAN_NUM_PROBES, pDcpt->pCtrl->scanNumProbes);
    }

    if (true == active)
    {
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_ACTIVE_SCAN_TIME, pDcpt->pCtrl->scanActiveScanTime);
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SCAN_TYPE, 1);

        if (NULL != pSSIDList)
        {
            DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_SCAN_SSID_LIST, (uint8_t*)ssidList, sizeof(ssidList));
        }
    }
    else
    {
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_PASSIVE_SCAN_TIME, pDcpt->pCtrl->scanPassiveListenTime);
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SCAN_TYPE, 0);
    }

    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SCAN_CH_BITMAP_2GHZ, pDcpt->pCtrl->scanChannelMask24);
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_BCAST_SSID, 0);
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_START_SCAN_REQ, 1);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    if (false == DRV_PIC32MZW_MultiWIDWrite(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    pDcpt->pCtrl->scanInProgress    = true;
    pDcpt->pCtrl->scanIndex         = 0;
    pDcpt->pCtrl->pfBSSFindNotifyCB = pfNotifyCallback;

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindNext
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Request the next scan results be provided.

  Description:
    The information structure of the next BSS is requested from the PIC32MZW.

  Remarks:
    See wdrv_pic32mzw_bssfind.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindNext
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (NULL == pDcpt->pCtrl) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Ensure RF and MAC is configured */
    if (WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG != (pDcpt->pCtrl->rfMacConfigStatus & WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG))
    {
        return WDRV_PIC32MZW_STATUS_RF_MAC_CONFIG_NOT_VALID;
    }

    /* Cannot request results while a scan is in progress. */
    if (true == pDcpt->pCtrl->scanInProgress)
    {
        return WDRV_PIC32MZW_STATUS_SCAN_IN_PROGRESS;
    }

    /* Check if the request would exceed the number of results
       available, signal find operation end if so. */
    pDcpt->pCtrl->scanIndex++;

    if (pDcpt->pCtrl->scanIndex >= scanResultCache.numDescrs)
    {
        pDcpt->pCtrl->scanIndex--;

        return WDRV_PIC32MZW_STATUS_BSS_FIND_END;
    }

    /* Store callback supplied. */
    pDcpt->pCtrl->pfBSSFindNotifyCB = pfNotifyCallback;

    if (NULL != pDcpt->pCtrl->pfBSSFindNotifyCB)
    {
        WDRV_PIC32MZW_BSS_INFO bssInfo;
        bool findResult;

        while (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSFindGetInfo(handle, &bssInfo))
        {
            /* Notify the user application of the scan results. */
            findResult = pDcpt->pCtrl->pfBSSFindNotifyCB(handle, pDcpt->pCtrl->scanIndex+1, scanResultCache.numDescrs, &bssInfo);

            /* Check if the callback requested the next set of results. */
            if (true == findResult)
            {
                /* Request the next BSS results, or end operation if no
                   more are available. */
                pDcpt->pCtrl->scanIndex++;

                if (pDcpt->pCtrl->scanIndex >= scanResultCache.numDescrs)
                {
                    pDcpt->pCtrl->pfBSSFindNotifyCB = NULL;
                    break;
                }
            }
            else
            {
                pDcpt->pCtrl->pfBSSFindNotifyCB = NULL;
                break;
            }
        }
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindReset
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Request the first scan results again

  Description:
    The information structure of the first BSS is requested from the PIC32MZW.

  Remarks:
    See wdrv_pic32mzw_bssfind.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindReset
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Cannot reset the find operation while a scan is in progress. */
    if (true == pDcpt->pCtrl->scanInProgress)
    {
        return WDRV_PIC32MZW_STATUS_SCAN_IN_PROGRESS;
    }

    /* Reuse find next function by pre-decrementing scan index. */
    pDcpt->pCtrl->scanIndex = -1;

    return WDRV_PIC32MZW_BSSFindNext(handle, pfNotifyCallback);
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindGetInfo
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_BSS_INFO *const pBSSInfo
    )

  Summary:
    Requests the information structure of the current BSS scan result.

  Description:
    After each call to either WDRV_PIC32MZW_BSSFindFirst or WDRV_PIC32MZW_BSSFindNext
      the driver receives a single BSS information structure which it stores.
      This function retrieves that structure.

  Remarks:
    See wdrv_pic32mzw_bssfind.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindGetInfo
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_BSS_INFO *const pBSSInfo
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    DRV_PIC32MZW_SCAN_RESULTS *pLastBSSScanInfo;
    DRV_PIC32MZW_11I_MASK dot11iInfo;

    /* Ensure the driver handle and user pointer is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl) || (NULL == pBSSInfo))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    if (pDcpt->pCtrl->scanIndex >= scanResultCache.numDescrs)
    {
        return WDRV_PIC32MZW_STATUS_NO_BSS_INFO;
    }

    pLastBSSScanInfo = &scanResultCache.bssDescr[pDcpt->pCtrl->scanIndex];

    if (0 == pLastBSSScanInfo->ofTotal)
    {
        return WDRV_PIC32MZW_STATUS_NO_BSS_INFO;
    }

    /* Copy BSS scan cache to user supplied buffer. */
    pBSSInfo->ctx.channel       = pLastBSSScanInfo->channel;
    pBSSInfo->rssi              = pLastBSSScanInfo->rssi;
    pBSSInfo->ctx.ssid.length   = pLastBSSScanInfo->ssid.length;
    pBSSInfo->ctx.bssid.valid   = true;
    pBSSInfo->ctx.cloaked       = false;

    memcpy(pBSSInfo->ctx.bssid.addr, pLastBSSScanInfo->bssid, 6);

    memset(pBSSInfo->ctx.ssid.name, 0, WDRV_PIC32MZW_MAX_SSID_LEN);
    memcpy(pBSSInfo->ctx.ssid.name, pLastBSSScanInfo->ssid.name, pBSSInfo->ctx.ssid.length);

    /* Derive security capabilities from dot11iInfo field. */
    dot11iInfo = (DRV_PIC32MZW_11I_MASK)(pLastBSSScanInfo->dot11iInfo);
    pBSSInfo->secCapabilities = 0;
    if (DRV_PIC32MZW_PRIVACY == dot11iInfo)
    {
        /* Privacy bit and no 11i elements means WEP. */
        pBSSInfo->secCapabilities |= WDRV_PIC32MZW_SEC_BIT_WEP;
    }
    else
    {
        /* Convert dot11iInfo bits into WDRV_PIC32MZW_SEC_MASK value. */
        int count = sizeof(map11iToSecMask)/sizeof(map11iToSecMask[0]);
        while (count--)
        {
            if (dot11iInfo & (1<<count))
            {
                pBSSInfo->secCapabilities |= map11iToSecMask[count];
            }
        }
        /* The above mapping sets the WPA2/3 bit based on the setting of
         * DRV_PIC32MZW_11I_CCMP128. We should require DRV_PIC32MZW_11I_RSNE
         * to be set also. */
        if (!(dot11iInfo & DRV_PIC32MZW_11I_RSNE))
        {
            pBSSInfo->secCapabilities &= ~WDRV_PIC32MZW_SEC_BIT_WPA2OR3;
        }
    }

    /* Derive recommended auth type for connection. Start with default (invalid). */
    pBSSInfo->authTypeRecommended = WDRV_PIC32MZW_AUTH_TYPE_DEFAULT;

    /* If no security is offered then we must use Open. */
    if (0 == dot11iInfo)
    {
        pBSSInfo->authTypeRecommended = WDRV_PIC32MZW_AUTH_TYPE_OPEN;
    }
    /* If WEP is offered then we must use WEP. */
    else if (
            (DRV_PIC32MZW_PRIVACY == dot11iInfo)
        ||  (dot11iInfo & DRV_PIC32MZW_11I_WEP)
    )
    {
        pBSSInfo->authTypeRecommended = WDRV_PIC32MZW_AUTH_TYPE_WEP;
    }
    /* Otherwise if TKIP is offered, then we must use WPA2 compatibility mode. */
    else if (
            (dot11iInfo & DRV_PIC32MZW_11I_TKIP)
        ||  !(dot11iInfo & DRV_PIC32MZW_11I_RSNE)
    )
    {
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
        if (dot11iInfo & DRV_PIC32MZW_11I_1X)
        {
            pBSSInfo->authTypeRecommended = WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_ENTERPRISE;
        }
        else
#endif
        if (dot11iInfo & DRV_PIC32MZW_11I_PSK)
        {
            pBSSInfo->authTypeRecommended = WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL;
        }
    }
    /* Otherwise check CCMP-128 is offered and recommend WPA2 or WPA3. */
    else if (dot11iInfo & DRV_PIC32MZW_11I_CCMP128)
    {
        /* WPA3-Personal if available. */
#ifdef WDRV_PIC32MZW_WPA3_PERSONAL_SUPPORT
        if (
                (dot11iInfo & DRV_PIC32MZW_11I_SAE)
            &&  (dot11iInfo & DRV_PIC32MZW_11I_BIPCMAC128)
        )
        {
            pBSSInfo->authTypeRecommended = WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL;
        }
        else
#endif
        /* Otherwise Enterprise if available. */
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
        if (dot11iInfo & DRV_PIC32MZW_11I_1X)
        {
            /* If AP _requires_ MFP then we can use WPA3-only.               */
            /* Note that MFP _capability_ is not sufficient - that does not  */
            /* guarantee support for AKM suite 5.                            */
            if (dot11iInfo & DRV_PIC32MZW_11I_MFP_REQUIRED)
            {
                pBSSInfo->authTypeRecommended = WDRV_PIC32MZW_AUTH_TYPE_WPA3_ENTERPRISE;
            }
            /* Otherwise WPA3-only might not work, so use WPA3 transition. */
            else
            {
                pBSSInfo->authTypeRecommended = WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE;
            }
        }
        else
#endif
        /* Otherwise WPA2-Personal. */
        if (dot11iInfo & DRV_PIC32MZW_11I_PSK)
        {
            pBSSInfo->authTypeRecommended = WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL;
        }
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetScanParameters
    (
        DRV_HANDLE handle,
        uint8_t numSlots,
        uint16_t activeSlotTime,
        uint16_t passiveSlotTime,
        uint8_t numProbes
    )

  Summary:
    Configures the scan operation.

  Description:
    Configures the time periods of active and passive scanning operations.

  Remarks:
    See wdrv_pic32mzw_bssfind.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetScanParameters
(
    DRV_HANDLE handle,
    uint8_t numSlots,
    uint16_t activeSlotTime,
    uint16_t passiveSlotTime,
    uint8_t numProbes
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Check for update to Active Scan Time. */
    if ((0 != activeSlotTime) && (pDcpt->pCtrl->scanActiveScanTime != activeSlotTime))
    {
        if ((activeSlotTime > DRV_PIC32MZW_MAX_SCAN_TIME) || (activeSlotTime < DRV_PIC32MZW_MIN_SCAN_TIME))
        {
            return WDRV_PIC32MZW_STATUS_INVALID_ARG;
        }
        pDcpt->pCtrl->scanActiveScanTime = activeSlotTime;
        pDcpt->pCtrl->scanParamDefault = false;
    }

    /* Check for update to Passive Scan Time. */
    if ((0 != passiveSlotTime) && (pDcpt->pCtrl->scanPassiveListenTime != passiveSlotTime))
    {
        if ((passiveSlotTime > DRV_PIC32MZW_MAX_SCAN_TIME) || (passiveSlotTime < DRV_PIC32MZW_MIN_SCAN_TIME))
        {
            return WDRV_PIC32MZW_STATUS_INVALID_ARG;
        }
        pDcpt->pCtrl->scanPassiveListenTime = passiveSlotTime;
        pDcpt->pCtrl->scanParamDefault = false;
    }

    /* Check for update to Number of Slots. */
    if ((0 != numSlots) && (pDcpt->pCtrl->scanNumSlots != numSlots))
    {
        pDcpt->pCtrl->scanNumSlots = numSlots;
        pDcpt->pCtrl->scanParamDefault = false;
    }

    /* Check for update to Number of Probes. */
    if ((0 != numProbes) && (pDcpt->pCtrl->scanNumProbes != numProbes))
    {
        if ((numProbes > DRV_PIC32MZW_SCAN_MAX_NUM_PROBE) || (numProbes < DRV_PIC32MZW_SCAN_MIN_NUM_PROBE))
        {
            return WDRV_PIC32MZW_STATUS_INVALID_ARG;
        }
        pDcpt->pCtrl->scanNumProbes = numProbes;
        pDcpt->pCtrl->scanParamDefault = false;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetEnabledChannels24
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_CHANNEL24_MASK channelMask24
    )

  Summary:
    Set the enabled channels list for 2.4GHz.

  Description:
    To comply with regulatory domains certain channels must not be scanned.
      This function configures which channels are enabled to be used.

  Remarks:
    See wdrv_pic32mzw_bssfind.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetEnabledChannels24
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_CHANNEL24_MASK channelMask24
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    if (0 != (channelMask24 & ~WDRV_PIC32MZW_CM_2_4G_ALL))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    pDcpt->pCtrl->scanChannelMask24 = channelMask24;

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetScanMatchMode
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_SCAN_MATCH_MODE matchMode
    )

  Summary:
    Configures the scan matching mode.

  Description:
    This function configures the matching mode, either stop on first or
      match all, used when scanning for SSIDs.

  Remarks:
    See wdrv_pic32mzw_bssfind.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_BSSFindSetScanMatchMode
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_SCAN_MATCH_MODE matchMode
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    uint8_t stopScanOption;
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

    /* Validate the match mode. */
    if (WDRV_PIC32MZW_SCAN_MATCH_MODE_STOP_ON_FIRST == matchMode)
    {
        stopScanOption = 1;
    }
    else if (WDRV_PIC32MZW_SCAN_MATCH_MODE_FIND_ALL == matchMode)
    {
        stopScanOption = 0;
    }
    else
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    DRV_PIC32MZW_MultiWIDInit(&wids, 8);
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SCAN_STOP_ON_FIRST, stopScanOption);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    if (false == DRV_PIC32MZW_MultiWIDWrite(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    uint8_t WDRV_PIC32MZW_BSSFindGetNumBSSResults(DRV_HANDLE handle)

  Summary:
    Returns the number of BSS scan results found.

  Description:
    Returns the number of BSS scan results found.

  Remarks:
    See wdrv_pic32mzw_bssfind.h for usage information.

*/

uint8_t WDRV_PIC32MZW_BSSFindGetNumBSSResults(DRV_HANDLE handle)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid and the instance is open. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return 0;
    }

    if ((false == pDcpt->isOpen) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return 0;
    }

    /* Return the number of BSSs found. */
    return scanResultCache.numDescrs;
}

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_BSSFindInProgress(DRV_HANDLE handle)

  Summary:
    Indicates if a BSS scan is in progress.

  Description:
    Returns a flag indicating if a BSS scan operation is currently running.

  Remarks:
    See wdrv_pic32mzw_bssfind.h for usage information.

*/

bool WDRV_PIC32MZW_BSSFindInProgress(DRV_HANDLE handle)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;

    /* Ensure the driver handle is valid and the instance is open. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return false;
    }

    if ((false == pDcpt->isOpen) || (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle))
    {
        return false;
    }

    return pDcpt->pCtrl->scanInProgress;
}

//*******************************************************************************
/*
  Function:
    bool DRV_PIC32MZW_StoreBSSScanResult(const DRV_PIC32MZW_SCAN_RESULTS *const pScanResult)

  Summary:
    Store BSS scan results in cache.

  Description:
    Store the indexed BSS results in the cache, checking for possible restart.

  Precondition:
    None.

  Parameters:
    pScanResult - Pointer to single BSS scan resuls.

  Returns:
    true or false indicate result of store operation.

*/

bool DRV_PIC32MZW_StoreBSSScanResult(const DRV_PIC32MZW_SCAN_RESULTS *const pScanResult)
{
    if (NULL == pScanResult)
    {
        return false;
    }

    if (pScanResult->index >= (WDRV_PIC32MZW_SCAN_RESULT_CACHE_NUM_ENTRIES-1))
    {
        return false;
    }

    /* If the scan has restarted or the total number of descriptors has changed
     clear the cache. */
    if ((0 == pScanResult->index) || (scanResultCache.numDescrs != pScanResult->ofTotal))
    {
        memset(&scanResultCache, 0, sizeof(WDRV_PIC32MZW_SCAN_RESULT_CACHE));
        scanResultCache.numDescrs = pScanResult->ofTotal;
    }

    memcpy(&scanResultCache.bssDescr[pScanResult->index], pScanResult, sizeof(DRV_PIC32MZW_SCAN_RESULTS));

    return true;
}
