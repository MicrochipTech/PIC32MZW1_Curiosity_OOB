/*******************************************************************************
  PIC32MZW Driver Association Implementation

  File Name:
    wdrv_pic32mzw_assoc.c

  Summary:
    PIC32MZW wireless driver association implementation.

  Description:
    This interface provides information about the current association with a
    peer device.
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
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>

#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_assoc.h"
#include "wdrv_pic32mzw_cfg.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Association Internal Implementation
// *****************************************************************************
// *****************************************************************************

static bool _WDRV_PIC32MZW_AssocHandleIsValid
(
    const WDRV_PIC32MZW_CTRLDCPT *const pCtrl,
    const WDRV_PIC32MZW_ASSOC_INFO *const pAssocInfo
)
{
    int i;

    if ((NULL == pCtrl) || (NULL == pAssocInfo))
    {
        return false;
    }

    for (i=0; i<WDRV_PIC32MZW_NUM_ASSOCS; i++)
    {
        if (pAssocInfo == &pCtrl->assocInfoAP[i])
        {
            return true;
        }
    }

    if (pAssocInfo == &pCtrl->assocInfoSTA)
    {
        return true;
    }

    return false;
}

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Association Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocPeerAddressGet
    (
        WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
        WDRV_PIC32MZW_MAC_ADDR *const pPeerAddress
    )

  Summary:
    Retrieve the current association peer device network address.

  Description:
    Attempts to retrieve the network address of the peer device in the
      current association.

  Remarks:
    See wdrv_pic32mzw_assoc.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocPeerAddressGet
(
    WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
    WDRV_PIC32MZW_MAC_ADDR *const pPeerAddress
)
{
    WDRV_PIC32MZW_CTRLDCPT *pCtrl;
    WDRV_PIC32MZW_ASSOC_INFO *const pAssocInfo = (WDRV_PIC32MZW_ASSOC_INFO *const)assocHandle;

    if ((WDRV_PIC32MZW_ASSOC_HANDLE_INVALID == assocHandle) || (NULL == pAssocInfo) || (NULL == pPeerAddress))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    pCtrl = (WDRV_PIC32MZW_CTRLDCPT*)pAssocInfo->handle;

    if ((DRV_HANDLE_INVALID == pAssocInfo->handle) || (NULL == pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_NOT_CONNECTED;
    }

    /* Ensure the association handle is valid. */
    if (false == _WDRV_PIC32MZW_AssocHandleIsValid(pCtrl, pAssocInfo))
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    if (true == pAssocInfo->peerAddress.valid)
    {
        /* If association information stored in driver and user application
           supplied a buffer, copy the peer address to the buffer. */

         memcpy(pPeerAddress, &pAssocInfo->peerAddress, sizeof(WDRV_PIC32MZW_MAC_ADDR));

        return WDRV_PIC32MZW_STATUS_OK;
    }
    else if (WDRV_PIC32MZW_CONN_STATE_CONNECTED == pCtrl->connectedState)
    {
        return WDRV_PIC32MZW_STATUS_RETRY_REQUEST;
    }

    return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocRSSIGet
    (
        WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
        int8_t *const pRSSI,
        WDRV_PIC32MZW_ASSOC_RSSI_CALLBACK const pfAssociationRSSICB
    )

  Summary:
    Retrieve the current association RSSI.

  Description:
    Attempts to retrieve the RSSI of the current association.

  Remarks:
    See wdrv_pic32mzw_assoc.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocRSSIGet
(
    WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
    int8_t *const pRSSI,
    WDRV_PIC32MZW_ASSOC_RSSI_CALLBACK const pfAssociationRSSICB
)
{
    WDRV_PIC32MZW_CTRLDCPT *pCtrl;
    WDRV_PIC32MZW_ASSOC_INFO *const pAssocInfo = (WDRV_PIC32MZW_ASSOC_INFO *const)assocHandle;

    if ((WDRV_PIC32MZW_ASSOC_HANDLE_INVALID == assocHandle) || (NULL == pAssocInfo))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    pCtrl = (WDRV_PIC32MZW_CTRLDCPT*)pAssocInfo->handle;

    if ((DRV_HANDLE_INVALID == pAssocInfo->handle) || (NULL == pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_NOT_CONNECTED;
    }

    /* Ensure the association handle is valid. */
    if (false == _WDRV_PIC32MZW_AssocHandleIsValid(pCtrl, pAssocInfo))
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    /* Store the callback for use later. */
    pCtrl->pfAssociationRSSICB = pfAssociationRSSICB;

    if (WDRV_PIC32MZW_CONN_STATE_CONNECTED == pCtrl->connectedState)
    {
        /* PIC32MZW is currently connected. */

        if (NULL == pfAssociationRSSICB)
        {
            /* No callback has been provided. */

            if ((0 == pAssocInfo->rssi) && (NULL == pRSSI))
            {
                /* No previous RSSI information and no callback or
                   user application buffer to receive the information. */

                return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
            }
            else if (NULL != pRSSI)
            {
                /* A current RSSI value exists and the user application provided
                   a buffer to receive it, copy the information. */

                *pRSSI = pAssocInfo->rssi;

                return WDRV_PIC32MZW_STATUS_OK;
            }
            else
            {
                /* No user application buffer and no callback. */
            }
        }
        else
        {
            DRV_PIC32MZW_WIDCTX wids;

            /* A callback has been provided, request the current RSSI from the
               PIC32MZW device. */

            DRV_PIC32MZW_MultiWIDInit(&wids, 16);

            if (false == pCtrl->isAP)
            {
                DRV_PIC32MZW_MultiWIDAddQuery(&wids, DRV_WIFI_WID_RSSI);
            }
            else
            {
                /* TODO: request the RSSI for the association selected. */
            }

            if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
            {
                return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
            }

            /* Request was successful so indicate the user application needs to
               retry request, or rely on callback for information. */

            return WDRV_PIC32MZW_STATUS_RETRY_REQUEST;
        }
    }
    else
    {
        /* PIC32MZW is currently disconnected. */
    }

    return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
}
