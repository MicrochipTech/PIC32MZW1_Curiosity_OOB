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
        WDRV_PIC32MZW_POWERSAVE_CORRELATION picCorrelation
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
    WDRV_PIC32MZW_POWERSAVE_PIC_CORRELATION picCorrelation
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

#endif /* _WDRV_PIC32MZW_PS_H */
