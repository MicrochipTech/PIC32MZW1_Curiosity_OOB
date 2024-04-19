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
        uint16_t listenInt
    );

  Summary:
    Set the WiFi listen interval for power-save operation(in beacon period count).

  Description:
    Set the WiFi listen interval value for power-save operation.It is given in
    units of Beacon period.

    Periodically after the listen interval fires, the WiFi wakes up and listen
    to the beacon and check for any buffered frames for it from the AP.

    A default value of 10 is used by the WiFi stack for listen interval. The user
    can override that value via this API.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle    - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    listenInt - Listen interval in units of beacon period.

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
    uint16_t listenInt
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveSleepInactLimitSet
    (
        DRV_HANDLE handle,
        uint16_t sleepInactLimit
    );

  Summary:
    Sets the sleep inactivity(assoc-timeout) threshold/limit for power-save operation
    (in beacon periods).

  Description:
    Set the sleep inactivity threshold/limit value for power-save operation.
    It is given in units of beacon period.

    During power-save if there is no activity in the BSS for the number of beacons
    specified by sleepInactLimit, a NULL frame will be sent to the AP.
    This is done to avoid the AP de-authenticating the STA during an inactivity period.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle          - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    sleepInactLimit - Inactivity threshold in units of beacon period.

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
    uint16_t sleepInactLimit
);

#endif /* _WDRV_PIC32MZW_PS_H */
