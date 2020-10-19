/*******************************************************************************
  PIC32MZW Driver Powersave Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_ps.h

  Summary:
    PIC32MZW wireless driver power save header file.

  Description:
    This interface provides control APIs for the WiFi power save modes.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc. All rights reserved.

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

#include "wdrv_pic32mzw_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*  WiFi powersave/low power modes

  Summary:
    Defines the power save modes supported by WiFi driver/FW.

  Description:
    This enumeration defines the various WiFi power save modes supported by WiFi.

  Remarks:
    None.
*/
typedef enum
{
    /* Run mode : No powersave. Both Tx and Rx are active */
    WDRV_PIC32MZW_POWERSAVE_RUN_MODE,

	/* WSM mode : Wireless sleep mode. Tx,Rx is stopped, clocks will be running.
	STA will be in powersave mode keeping the connection active with AP */
    WDRV_PIC32MZW_POWERSAVE_WSM_MODE,

	/* WDS mode : Wireless Deep sleep mode. Tx,Rx is stopped. clocks will be cutoff.
	STA will be in powersave mode keeping the connection active with AP */
    WDRV_PIC32MZW_POWERSAVE_WDS_MODE,

	/* WXDS mode : Wireless Extreme Deep sleep mode. Everything shutsdown. No Tx,Rx. Reinitialize WiFi driver/context to recover
	STA will disconnect from AP. Restore the connection with the saved context */
    WDRV_PIC32MZW_POWERSAVE_WXDS_MODE,

	/* WOFF mode : Wireless OFF mode. Turns off wifi LDO(WLDO). No Tx,Rx Reinitialize WiFi driver/context to recover.
	STA will disconnect from AP. Restore the connection with the saved context */
    WDRV_PIC32MZW_POWERSAVE_WOFF_MODE
}WDRV_PIC32MZW_POWERSAVE_MODE;


//*******************************************************************************
/*  WiFi and PIC powersave/sleep modes correlation

  Summary:
    Defines the correlation between WiFi and PIC sleep modes.

  Description:
    This enumeration defines the correlation between WiFi and PIC sleep modes.

	WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE -
	PIC sleep entry forces WiFi into Sleep, PIC wakeup (non WIFI) can be independent of the WIFI Sleep modes
	WiFi sleep entry can be independent of the PIC sleep mode entry, WIFI wakeup to RUN mode will force PIC into RUN mode
	ASYNC mode is not applicable for WXDS and WOFF powersave modes.

	WDRV_PIC32MZW_POWERSAVE_PIC_SYNC_MODE -
	PIC sleep entry forces the WiFi into sleep mode and vice-versa. PIC wakeup forces the WiFi sleep exit(Run) and vice-versa

  Remarks:
    None.
*/
typedef enum
{
    /* Asynchronous correlation. Trigger of sleep mode entry/exit is done through software */
    WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE,

	/* Synchronous correlation. Trigger of sleep mode entry/exit is done through hardware */
    WDRV_PIC32MZW_POWERSAVE_PIC_SYNC_MODE
} WDRV_PIC32MZW_POWERSAVE_CORRELATION;


//*******************************************************************************
/*  WiFi powersave/sleep mode configuration

  Summary:
    WiFi powersave configuration structure containing information about the WiFi supported powersave modes, configuration.

  Description:
    The configuration structure contains information related to powersave mode, correlation with the pic state,
    listen interval, configuring DTIM monitoring etc.

  Remarks:
    None.
*/
typedef struct
{
    /* Set the power save mode(Run, WSM, WDS, WOFF, WXDS) */
   	WDRV_PIC32MZW_POWERSAVE_MODE powerSaveMode;

	/* Specify the correlation between PIC powersave state and WiFi powersave modes */
	WDRV_PIC32MZW_POWERSAVE_CORRELATION picCorrelation;

    /* Specify whether to save WiFi context data to WCM(WiFi Context Memory)
     (Applicable for WOFF and WXDS mode only) For WOFF and WXDS set to TRUE else FALSE */
	bool saveContextEnabled;

	/* Broadcast reception enable:
	If TRUE, STA will awake each DTIM beacon for recieving broadcast traffic.
	If FALSE, STA will not wakeup at the DTIM beacon,but it will wakeup depends only on the
	configured Listen Interval(DRV_PIC32MZW_PS_LISTEN_INTERVAL)
	(Applicable for WSM and WDS power save modes only) */
	bool BcastTrafficEnabled;
} WDRV_PIC32MZW_POWERSAVE_CONFIG;


// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Power save Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveModeSet
    (
        DRV_HANDLE handle,
        const WDRV_PIC32MZW_POWERSAVE_CONFIG *const pPowerSaveCfg
    )

  Summary:
    Set the power save/sleep mode for WiFi.

  Description:
    Requests that the current powersavemode is changed to the one specified according to config parameters.

	In case of WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE,
    - Call to WDRV_PIC32MZW_PowerSaveModeSet with powerSaveMode set to WSM/WDS/WOFF/WXDS will put WiFi into respective powersave/sleep mode.
    - Call to WDRV_PIC32MZW_PowerSaveModeSet with powerSaveMode set to RUN to exit from powersave/sleep mode.

    In case of WDRV_PIC32MZW_POWERSAVE_PIC_SYNC_MODE,
    - Call to WDRV_PIC32MZW_PowerSaveModeSet with powerSaveMode set to WSM/WDS/WOFF/WXDS will just prepare WiFi to enter into respective powersave/sleep mode.
      On issue of cpu_wait command from PMF(PIC), HW sends hw_wifi_sleep_mode_req signal to WiFi which automatically puts the WiFi into respective powersave/sleep mode.
    - Call to WDRV_PIC32MZW_PowerSaveModeSet with powerSaveMode set to RUN to exit from powersave/sleep mode.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
    WDRV_PIC32MZW_BSSConnect should have been called and there should be an active connection established with an AP.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    pPowerSaveCfg    - Pointer to power save configuration.

  Returns:
    WDRV_PIC32MZW_STATUS_OK              - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR   - The request to the PIC32MZW was rejected.
    WDRV_PIC32MZW_STATUS_NOT_CONNECTED	 - Not currently connected to an AP
	WDRV_PIC32MZW_STATUS_OPERATION_NOT_SUPPORTED -Request operation is not supported.

  Remarks:
    None.

*/
WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_PowerSaveModeSet
 (
	 DRV_HANDLE handle,
	 const WDRV_PIC32MZW_POWERSAVE_CONFIG *const pPowerSaveCfg
 );


#endif /* _WDRV_PIC32MZW_PS_H */
