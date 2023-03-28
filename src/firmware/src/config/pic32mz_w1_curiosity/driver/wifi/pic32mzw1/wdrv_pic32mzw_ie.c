/*******************************************************************************
  PIC32MZW Driver IE Implementation

  File Name:
    wdrv_pic32mzw_ie.c

  Summary:
    PIC32MZW wireless driver vendor specific IE implementation.

  Description:
    This file provides an interface for manipulating the vendor specific
    information element. Custom IE's can be added to management frames and also
    vendor specific IEs can be received from management frames.
 ******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2021 released Microchip Technology Inc.  All rights reserved.

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
 ******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_custie.h"
#include "wdrv_pic32mzw_cfg.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver IE Implementation
// *****************************************************************************
// *****************************************************************************

//******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_IECustTxDataSet
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_IE_FRAME_TYPE_MASK frameMask,
        const WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx
    )

  Summary:
    Configures the custom IE.

  Description:
    Management frames like, beacons, probe request and probe response etc. may
    contain an application provided custom IE. This function associates a custom
    IE store context with the STA/soft-AP instance.

  Remarks:
    See wdrv_pic32mzw_ie.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_IECustTxDataSet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_IE_FRAME_TYPE_MASK frameMask,
    const WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx
)
{
    WDRV_PIC32MZW_DCPT *pDcpt = (WDRV_PIC32MZW_DCPT *)handle;
    DRV_PIC32MZW_WIDCTX wids;
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* We are concerned about 3 bits in the mask*/
    frameMask &= 0x0007;
    
    /* Ensure the driver handle and user pointer is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl) || ((0 != frameMask) && (NULL == pCustIECtx)))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }
    
    /* Update the frame filter mask */
    pDcpt->pCtrl->vendorIEMask &= 0xF0;
    pDcpt->pCtrl->vendorIEMask |= frameMask;
    
    if (0 != frameMask)
    {    
        DRV_PIC32MZW_MultiWIDInit(&wids, (32 + pCustIECtx->curLength));

        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_VSIE_FRAME, pDcpt->pCtrl->vendorIEMask);

        DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_VSIE_TX_DATA, (uint8_t*)&pCustIECtx->ieData, pCustIECtx->curLength);
    }
    else
    {
        DRV_PIC32MZW_MultiWIDInit(&wids, 32 );

        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_VSIE_FRAME, pDcpt->pCtrl->vendorIEMask);

        DRV_PIC32MZW_MultiWIDAddData(&wids, DRV_WIFI_WID_VSIE_TX_DATA, NULL, 0);
    }

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the wids. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }
    
    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}

//******************************************************************************
/*
WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_IERxDataGet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_IE_FRAME_TYPE_MASK frameMask,
    const uint32_t vendorOUI,
    const WDRV_PIC32MZW_IE_RX_CALLBACK pfVendorIERxCB
)

  Summary:
    Requests for received vendor IE.

  Description:
    This API places a request and registers a callback to receive vendor
    specific IE data based on the OUI and frame filter mask provided by the
    application. The received vendor specific IE data is passed to the
    application using the callback pfVendorIERxCB.

  Remarks:
    See wdrv_pic32mzw_ie.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_IERxDataGet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_IE_FRAME_TYPE_MASK frameMask,
    const uint32_t vendorOUI,
    const WDRV_PIC32MZW_IE_RX_CALLBACK pfVendorIERxCB
)
{
    WDRV_PIC32MZW_DCPT *pDcpt = (WDRV_PIC32MZW_DCPT *)handle;
    DRV_PIC32MZW_WIDCTX wids;
    OSAL_CRITSECT_DATA_TYPE critSect;
    
    /* We are concerned about 3 bits in the mask*/
    frameMask &= 0x0007;

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
    
    /* Update the frame filter mask */
    pDcpt->pCtrl->vendorIEMask &= 0x0F;
    pDcpt->pCtrl->vendorIEMask |= (frameMask << 4);
    
    DRV_PIC32MZW_MultiWIDInit(&wids, 64);
    
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_VSIE_FRAME, pDcpt->pCtrl->vendorIEMask);
    
    if (0 != frameMask)
    {
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_VSIE_RX_OUI, vendorOUI);

        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_VSIE_INFO_ENABLE, 1);
    }
    else
    {
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_VSIE_RX_OUI, 0);

        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_VSIE_INFO_ENABLE, 0);
    }
    
    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the wids. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }
    
    pDcpt->pCtrl->pfVendorIERxCB = pfVendorIERxCB;
    
    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}