/*******************************************************************************
  PIC32MZW Driver Power-Save Implementation

  File Name:
    wdrv_pic32mzw_ps.c

  Summary:
    PIC32MZW wireless power-save implementation.

  Description:
    This interface controls the WiFi power-save functionality
 *******************************************************************************/

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
 *******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_ps.h"
#include "wdrv_pic32mzw_cfg.h"

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveModeSet
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_POWERSAVE_MODE powerSaveMode,
        WDRV_PIC32MZW_POWERSAVE_CORRELATION picCorrelation
    )

  Summary:
    Set the power-save/sleep mode for WiFi.

  Description:
    Requests that the current power-save mode is changed to the one specified
    according to the parameters.

  Remarks:
    See wdrv_pic32mzw_ps.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveModeSet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_POWERSAVE_MODE powerSaveMode,
    WDRV_PIC32MZW_POWERSAVE_PIC_CORRELATION picCorrelation
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    DRV_PIC32MZW_WIDCTX wids;
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Ensure operation mode is really STA. */
    if (true == pDcpt->pCtrl->isAP)
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    /* Ensure driver handle is valid */
    if (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Currently only ASYNC mode is supported */
    if (WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE != picCorrelation)
    {
        return WDRV_PIC32MZW_STATUS_OPERATION_NOT_SUPPORTED;
    }

    if (powerSaveMode <= WDRV_PIC32MZW_POWERSAVE_WDS_MODE)
    {
        /* Allocate memory for the WIDs. */
        DRV_PIC32MZW_MultiWIDInit(&wids, 64);

        /* Set the correlation */
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_PS_CORRELATION, picCorrelation);

        /* Set the power-save mode */
        DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_POWER_MANAGEMENT, powerSaveMode);

        critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

        /* Write the wids. */
        if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
        {
            OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);
            return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
        }

        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);
    }
    else
    {
        return WDRV_PIC32MZW_STATUS_OPERATION_NOT_SUPPORTED;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveBroadcastTrackingSet
    (
        DRV_HANDLE handle,
        bool dtimTracking
    );

  Summary:
    Configures the WiFi broadcast traffic wake up behaviour.

  Description:
    Configures if the WiFi should wake for DTIM broadcast traffic or not.

  Remarks:
    See wdrv_pic32mzw_ps.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveBroadcastTrackingSet
(
    DRV_HANDLE handle,
    bool dtimTracking
)
{
    WDRV_PIC32MZW_DCPT *const pDcpt = (WDRV_PIC32MZW_DCPT *const)handle;
    DRV_PIC32MZW_WIDCTX wids;
    OSAL_CRITSECT_DATA_TYPE critSect;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if ((false == pDcpt->isOpen) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_PIC32MZW_STATUS_NOT_OPEN;
    }

    /* Ensure operation mode is really STA. */
    if (true == pDcpt->pCtrl->isAP)
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    /* Ensure driver handle is valid */
    if (DRV_HANDLE_INVALID == pDcpt->pCtrl->handle)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 64);

    /* Enable/Disable broadcast traffic reception during power-save */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_PS_BCAST_ENABLE, dtimTracking ? 1 : 0);

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
