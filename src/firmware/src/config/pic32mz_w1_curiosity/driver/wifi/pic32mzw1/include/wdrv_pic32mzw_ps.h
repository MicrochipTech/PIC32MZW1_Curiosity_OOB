/*******************************************************************************
  PIC32MZW Driver Power-Save Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_ps.h

  Summary:
    PIC32MZW wireless driver power-save header file.

  Description:
    This interface provides control APIs for the WiFi power-save modes.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2021 released Microchip Technology Inc. All rights reserved.

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

#ifndef _WDRV_PIC32MZW_PS_H
#define _WDRV_PIC32MZW_PS_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

/*  WiFi power-save sleep entry and powersave-cycle exit Notification Callback

  Summary:
    Callback to signal sleep entry of SMC(WSM/WDS) and exit of powersave cycle.

  Description:
    After WiFi power-save mode is set by the user, the driver will use this 
    callback to provide notification on each sleep entry of power-save 
    sleep-wakeup-sleep cycle and exit notification of power-save cycle on error 
    OR on user trigger to Run mode.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    psMode      - Current power-save mode.
    bSleepEntry - TRUE on sleep entry. FALSE on power-save cycle exit.
    u32SleepDurationMs - Duration of sleep configured for SMC(WSM/WDS).

  Remarks:
    The user can take necessary action on sleep entry 
    For ex, configure RTCC and put PIC to sleep/idle.
*/

typedef void (*WDRV_PIC32MZW_PS_NOTIFY_CALLBACK)
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_POWERSAVE_MODE psMode,
    bool bSleepEntry,
    uint32_t u32SleepDurationMs
);

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Power-Save Routines
// *****************************************************************************
// *****************************************************************************

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

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
    WiFi must not currently be configured as a Soft-AP

  Parameters:
    handle          - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    powerSaveMode   - Power-save mode for WiFi.
    picCorrelation  - PIC-WiFi power-save correlation mode.
    pfNotifyCallback - Callback function to receive sleep entry notification.

  Returns:
    WDRV_PIC32MZW_STATUS_OK                      - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN                - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG             - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR           - The request to the PIC32MZW was rejected.
    WDRV_PIC32MZW_STATUS_OPERATION_NOT_SUPPORTED - The requested operation is not supported.

  Remarks:
    Async Correlation Mode:
        powerSaveMode is applied immediately. WSM/WDS mode will apply even though
        the PIC is currently in the RUN state.

    Sync Correlation Mode:
        powerSaveMode is applied to the WiFi when the PIC next enters sleep mode.

    In either mode, powerSaveMode of RUN will cause the WiFi to leave power-save
    mode and enter RUN mode.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveModeSet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_POWERSAVE_MODE powerSaveMode,
    WDRV_PIC32MZW_POWERSAVE_PIC_CORRELATION picCorrelation,
    WDRV_PIC32MZW_PS_NOTIFY_CALLBACK pfNotifyCallback
);

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

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle          - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    dtimTracking    - Flag indicating if the WiFi should wake at DTIM intervals.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR  - The request to the PIC32MZW was rejected.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveBroadcastTrackingSet
(
    DRV_HANDLE handle,
    bool dtimTracking
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveListenIntervalSet
    (
        DRV_HANDLE handle,
        uint16_t u16ListenInt
    );

  Summary:
    Set the Wi-Fi listen interval for power save operation(in beacon period count).

  Description:
    Set the Wi-Fi listen interval value for power save operation.It is given in 
    units of Beacon period.
  
    Periodically after the listen interval fires, the WiFi wakes up and listen 
    to the beacon and check for any buffered frames for it from the AP.
    
    A default value of 10 is used by the WiFi stack for listen interval. The user 
    can override that value via this API.  

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
 
  Parameters:
    handle          - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    u16ListenInt    - Listen interval in units of beacon period.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR  - The request to the PIC32MZW was rejected.

  Remarks:
    
    WDRV_PIC32MZW_PowerSaveListenIntervalSet should be called before WDRV_PIC32MZW_BSSConnect.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveListenIntervalSet
(
    DRV_HANDLE handle,
    uint16_t u16ListenInt
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveSleepInactLimitSet
    (
        DRV_HANDLE handle,
        uint16_t u16SleepInactLimit
    );

  Summary:
    Set the sleep inactivity(assoc-timeout) threshold/limit for power save operation
   (in beacon period count).

  Description:
    Set the sleep inactivity threshold/limit value for power save operation.It is 
    given in units of Beacon period.
    
    During power-save if there is no activity in the BSS for the number of beacons 
    specified by u16SleepInactLimit, a NULL frame will be sent to the AP.
    This is done to avoid the AP de-authenticating the STA during an inactivity period.
    
    A default value of 10(ie, 10 ms) is used by the WiFi stack as Inactivity timeout limit. 
    The user can override that value via this API

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
 
  Parameters:
    handle          - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    u16SleepInactLimit - Inactivity threshold in units of Beacon period.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR  - The request to the PIC32MZW was rejected.

  Remarks:
    WDRV_PIC32MZW_PowerSaveSleepInactLimitSet should be called before WDRV_PIC32MZW_BSSConnect.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveSleepInactLimitSet
(
    DRV_HANDLE handle,
    uint16_t u16SleepInactLimit
);

#endif /* _WDRV_PIC32MZW_PS_H */
