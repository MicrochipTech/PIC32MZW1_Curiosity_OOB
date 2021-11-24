/*******************************************************************************
  PIC32MZW Wireless Driver System Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_api.h

  Summary:
    PIC32MZW wireless driver system API header file.

  Description:
    This interface is intended to be called from the main system initialization
    and task functions.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020-21 released Microchip Technology Inc. All rights reserved.

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

#ifndef _WDRV_PIC32MZW_API_H
#define _WDRV_PIC32MZW_API_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#include "system/system_module.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver System Data Types
// *****************************************************************************
// *****************************************************************************

#define WDRV_PIC32MZW_SYS_IDX_0         0

//*******************************************************************************
/*  WiFi power-save/low power modes

  Summary:
    Defines the power save modes supported by the WiFi driver/firmware.

  Description:
    This enumeration defines the various WiFi power save modes supported by the
    WiFi driver/firmware.

*/

typedef enum
{
    /* Run mode : No power-save. Both TX and RX are active */
    WDRV_PIC32MZW_POWERSAVE_RUN_MODE = 0,

    /* WSM mode : Wireless sleep mode. TX and RX are stopped, clocks will be running.
       STA will be in power-save mode keeping the connection active with AP. */
    WDRV_PIC32MZW_POWERSAVE_WSM_MODE = 1,

    /* WDS mode : Wireless Deep sleep mode. TX and RX are stopped. clocks will be cutoff.
       STA will be in power-save mode keeping the connection active with AP. */
    WDRV_PIC32MZW_POWERSAVE_WDS_MODE = 2
} WDRV_PIC32MZW_POWERSAVE_MODE;

//*******************************************************************************
/*  WiFi/PIC Power-Save/Sleep Modes Correlation

  Summary:
    Defines the correlation between WiFi and PIC sleep modes.

  Description:
    This enumeration defines the correlation between WiFi and PIC sleep modes.

    WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE -
    PIC sleep entry forces WiFi into Sleep, PIC wakeup (non WiFi) can be independent
    of the WiFi sleep modes. WiFi sleep entry can be independent of the PIC sleep mode
    entry. WiFi wakeup to RUN mode will force PIC into RUN mode.

    WDRV_PIC32MZW_POWERSAVE_PIC_SYNC_MODE -
    PIC sleep entry forces the WiFi into sleep mode and vice-versa.
    PIC wakeup forces the WiFi sleep exit(Run) and vice-versa.

  Remarks:
    None.

*/
typedef enum
{
    /* Asynchronous correlation. Trigger of sleep mode entry/exit is done through software */
    WDRV_PIC32MZW_POWERSAVE_PIC_ASYNC_MODE = 0,

    /* Synchronous correlation. Trigger of sleep mode entry/exit is done through hardware */
    WDRV_PIC32MZW_POWERSAVE_PIC_SYNC_MODE = 1
} WDRV_PIC32MZW_POWERSAVE_PIC_CORRELATION;

/*  WiFi Control Initialization Data

  Summary:
    Data that's passed to the Control instance.

  Description:
    This structure defines the Control instance initialization.

*/

typedef struct
{
    /* Pointer to Crypt RNG context. */
    CRYPT_RNG_CTX *pCryptRngCtx;

    /* Pointer to regulatory domain string. */
    char *const pRegDomName;

    /* Power-Save Mode. */
    WDRV_PIC32MZW_POWERSAVE_MODE powerSaveMode;

    /* Correlation between PIC and WiFi power modes. */
    WDRV_PIC32MZW_POWERSAVE_PIC_CORRELATION powerSavePICCorrelation;
} WDRV_PIC32MZW_SYS_INIT;

/*  WiFi MAC Initialization Data

  Summary:
    Data that's passed to the MAC at initialization time as part of the
    TCPIP_MAC_INIT data structure.

  Description:
    This structure defines the MAC initialization data for the
    WiFi MAC/Ethernet controller.

*/

typedef struct
{
} TCPIP_MODULE_MAC_PIC32MZW1_CONFIG;

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver System Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    SYS_MODULE_OBJ WDRV_PIC32MZW_Initialize
    (
        const SYS_MODULE_INDEX index,
        const SYS_MODULE_INIT *const init
    )

  Summary:
    System interface initialization of the PIC32MZW driver.

  Description:
    This is the function that initializes the PIC32MZW driver. It is called by
    the system.

  Precondition:
    None.

  Parameters:
    index   - Index for the PIC32MZW driver instance to be initialized.
    init    - Pointer to initialization data, currently not used.

  Returns:
    Valid handle to a driver object - if successful
    SYS_MODULE_OBJ_INVALID - if initialization failed

  Remarks:
    The returned object must be passed as argument to WDRV_PIC32MZW_Reinitialize,
      WDRV_PIC32MZW_Deinitialize, WDRV_PIC32MZW_Tasks and WDRV_PIC32MZW_Status routines.

*/

SYS_MODULE_OBJ WDRV_PIC32MZW_Initialize
(
    const SYS_MODULE_INDEX index,
    const SYS_MODULE_INIT *const init
);

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_Deinitialize(SYS_MODULE_OBJ object)

  Summary:
    PIC32MZW driver deinitialization function.

  Description:
    This is the function that deinitializes the PIC32MZW.
    It is called by the system.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.

  Parameters:
    object  - Driver object handle, returned from WDRV_PIC32MZW_Initialize

  Returns:
    None.

  Remarks:
    None.

*/

void WDRV_PIC32MZW_Deinitialize(SYS_MODULE_OBJ object);

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_Reinitialize
    (
        SYS_MODULE_OBJ object,
        const SYS_MODULE_INIT *const init
    )

  Summary:
    PIC32MZW driver reinitialization function.

  Description:
    This is the function that re-initializes the PIC32MZW.
    It is called by the system.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.

  Parameters:
    object  - Driver object handle, returned from WDRV_PIC32MZW_Initialize
    init    - Pointer to initialization data, currently not used.

  Returns:
    None.

  Remarks:
    None.

*/

void WDRV_PIC32MZW_Reinitialize
(
    SYS_MODULE_OBJ object,
    const SYS_MODULE_INIT *const init
);

//*******************************************************************************
/*
  Function:
    SYS_STATUS WDRV_PIC32MZW_Status(SYS_MODULE_OBJ object)

  Summary:
    Provides the current status of the PIC32MZW driver module.

  Description:
    This function provides the current status of the PIC32MZW driver module.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called before calling this function.

  Parameters:
    object  - Driver object handle, returned from WDRV_PIC32MZW_Initialize

  Returns:
    SYS_STATUS_READY  - Indicates that any previous module operation for the
                          specified module has completed
    SYS_STATUS_BUSY   - Indicates that a previous module operation for the
                          specified module has not yet completed
    SYS_STATUS_ERROR  - Indicates that the specified module is in an error state

  Example:
    <code>
    SYS_MODULE_OBJ      object;     // Returned from WDRV_PIC32MZW_Initialize
    SYS_STATUS          status;

    status = WDRV_PIC32MZW_Status(object);
    if (SYS_STATUS_ERROR >= status)
    {
        // Handle error
    }
    </code>

  Remarks:
    Any value greater than SYS_STATUS_READY is also a normal running state
      in which the driver is ready to accept new operations.

    SYS_STATUS_BUSY - Indicates that the driver is busy with a previous
      system level operation and cannot start another

    SYS_STATUS_ERROR - Indicates that the driver is in an error state
     Any value less than SYS_STATUS_ERROR is also an error state.

    SYS_MODULE_DEINITIALIZED - Indicates that the driver has been
      deinitialized

    This operation can be used to determine when any of the driver's
      module level operations has completed.

    If the status operation returns SYS_STATUS_BUSY, then a previous
      operation has not yet completed. Once the status operation returns
      SYS_STATUS_READY, any previous operations have completed.

    The value of SYS_STATUS_ERROR is negative (-1). Any value less than
      that is also an error state.

    This function will NEVER block waiting for hardware.

    If the Status operation returns an error value, the error may be
      cleared by calling the reinitialize operation. If that fails, the
      deinitialize operation will need to be called, followed by the
      initialize operation to return to normal operations.

*/

SYS_STATUS WDRV_PIC32MZW_Status(SYS_MODULE_OBJ object);

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_MACTasks(SYS_MODULE_OBJ object)

  Summary:
    Maintains the PIC32MZW MAC drivers state machine.

  Description:
    This function is used to maintain the driver's internal state machine.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called before calling this function.

  Parameters:
    object  - Driver object handle, returned from WDRV_PIC32MZW_Initialize

  Returns:
    None.

  Remarks:
    This function is normally not called directly by an application. It is
      called by the TCP/IP stack.

*/

void WDRV_PIC32MZW_MACTasks(SYS_MODULE_OBJ object);

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_Tasks(SYS_MODULE_OBJ object)

  Summary:
    Maintains the PIC32MZW drivers state machine.

  Description:
    This function is used to maintain the driver's internal state machine.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called before calling this function.

  Parameters:
    object  - Driver object handle, returned from WDRV_PIC32MZW_Initialize

  Returns:
    None.

  Remarks:
    This function is normally not called directly by an application. It is
      called by the system's Tasks routine (SYS_Tasks).

*/

void WDRV_PIC32MZW_Tasks(SYS_MODULE_OBJ object);

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_WIDProcess(uint16_t wid, uint16_t length, uint8_t *pData)

  Summary:
    WID callback.

  Description:
    Callback to handle WIDs from firmware:
      DRV_WIFI_WID_ASSOC_STAT
      DRV_WIFI_WID_GET_SCAN_RESULTS

  Precondition:
    None.

  Parameters:
    wid    - WID ID.
    length - Length of WID payload data.
    pData  - Pointer to WID payload

  Returns:
    None.

  Remarks:
    None.

*/

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW WID Processing Routines
// *****************************************************************************
// *****************************************************************************

void WDRV_PIC32MZW_WIDProcess(uint16_t wid, uint16_t length, const uint8_t *const pData);

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Interrupt Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_TasksRFMACISR(void)

  Summary:
    PIC32MZW RF MAC interrupt service routine.

  Description:
    PIC32MZW RF MAC interrupt service routine.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called before calling this function.

  Returns:
    None.

  Remarks:
    This function is normally not called directly by an application. It is
      called by the system's interrupt handler.

*/

void WDRV_PIC32MZW_TasksRFMACISR(void);

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_TasksRFTimer0ISR(void)

  Summary:
    PIC32MZW RF Timer 0 interrupt service routine.

  Description:
    PIC32MZW RF Timer 0 interrupt service routine.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called before calling this function.

  Returns:
    None.

  Remarks:
    This function is normally not called directly by an application. It is
      called by the system's interrupt handler.

*/

void WDRV_PIC32MZW_TasksRFTimer0ISR(void);

//*******************************************************************************
/*
  Function:
    void WDRV_PIC32MZW_TasksRFSMCISR(void)

  Summary:
    PIC32MZW RF SMC interrupt service routine.

  Description:
    PIC32MZW RF power-save interrupt service routine for WSM and WDS sleep modes.

  Precondition:
    WDRV_PIC32MZW_Initialize must have been called before calling this function.

  Returns:
    None.

  Remarks:
    This function is normally not called directly by an application. It is
      called by the system's interrupt handler.

*/

void WDRV_PIC32MZW_TasksRFSMCISR(void);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _WDRV_PIC32MZW_API_H */
