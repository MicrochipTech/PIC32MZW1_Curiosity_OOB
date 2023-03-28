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
        WDRV_PIC32MZW_POWERSAVE_CORRELATION picCorrelation,
        WDRV_PIC32MZW_PS_NOTIFY_CALLBACK pfNotifyCallback
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
    WDRV_PIC32MZW_POWERSAVE_PIC_CORRELATION picCorrelation,
    WDRV_PIC32MZW_PS_NOTIFY_CALLBACK pfNotifyCallback    
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
    
    /* Ensure power-save mode is valid */
    if (powerSaveMode > WDRV_PIC32MZW_POWERSAVE_WDS_MODE)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }
    
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

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }
    
    /* Store the parameters, powerSaveMode will be updated when we receive 
     * DRV_WIFI_WID_POWER_MANAGEMENT_INFO event from firmware */
    pDcpt->pCtrl->pfPSNotifyCB = pfNotifyCallback;
    pDcpt->pCtrl->powerSavePICCorrelation = picCorrelation;
    pDcpt->pCtrl->powerSaveMode = powerSaveMode;

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

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
    DRV_PIC32MZW_MultiWIDInit(&wids, 32);

    /* Enable/Disable broadcast traffic reception during power-save */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_PS_BCAST_ENABLE, dtimTracking ? 1 : 0);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the wids. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
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
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveListenIntervalSet
    (
        DRV_HANDLE handle,
        uint16_t u16ListenInt
    );

  Summary:
    Set the Wi-Fi listen interval for power save operation.

  Description:
    Set the Wi-Fi listen interval for power save operation.
    Periodically after the listen interval fires, the WiFi wakes up and listen 
    to the beacon and check for any buffered frames for it from the AP.
 
  Remarks:
    See wdrv_pic32mzw_ps.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveListenIntervalSet
(
    DRV_HANDLE handle,
    uint16_t u16ListenInt
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
    
    /* Ensure the STA is not connected */
    if ((WDRV_PIC32MZW_CONN_STATE_CONNECTING == pDcpt->pCtrl->connectedState) ||
            (WDRV_PIC32MZW_CONN_STATE_CONNECTED == pDcpt->pCtrl->connectedState))
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 32);

    /* Set the listen interval value for power-save */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_LISTEN_INTERVAL, u16ListenInt);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the wids. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
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
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveSleepInactLimitSet
	(
		DRV_HANDLE handle,
		uint16_t u16SleepInactLimit
	);

  Summary:
    Set the sleep inactivity(assoc-timeout) threshold/limit for power save 
    operation(in beacon period count).

  Description:
	Set the sleep inactivity threshold/limit value for power save operation.
    It is given in units of Beacon period.
    During power-save if there is no activity in the BSS for the number of beacons 
    specified by u16SleepInactLimit, a NULL frame will be sent to the AP.
    This is done to avoid the AP de-authenticating the STA during an inactivity period.
 
  Remarks:
    See wdrv_pic32mzw_ps.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveSleepInactLimitSet
(
    DRV_HANDLE handle,
    uint16_t u16SleepInactLimit
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
    
    /* Ensure the STA is not connected */
    if ((WDRV_PIC32MZW_CONN_STATE_CONNECTING == pDcpt->pCtrl->connectedState) ||
            (WDRV_PIC32MZW_CONN_STATE_CONNECTED == pDcpt->pCtrl->connectedState))
    {
        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    /* Allocate memory for the WIDs. */
    DRV_PIC32MZW_MultiWIDInit(&wids, 32);

    /* Set the sleep inactivity threshold limit for power-save */
    DRV_PIC32MZW_MultiWIDAddValue(&wids, DRV_WIFI_WID_SLEEP_INACT_IND_THRESHOLD, u16SleepInactLimit);

    critSect = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

    /* Write the wids. */
    if (false == DRV_PIC32MZW_MultiWid_Write(&wids))
    {
        OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

        DRV_PIC32MZW_MultiWIDDestroy(&wids);

        return WDRV_PIC32MZW_STATUS_REQUEST_ERROR;
    }

    OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, critSect);

    return WDRV_PIC32MZW_STATUS_OK;
}