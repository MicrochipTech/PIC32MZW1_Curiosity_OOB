/*******************************************************************************
  PIC32MZW Driver Debugging Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_debug.h

  Summary:
    PIC32MZW wireless driver debug header file.

  Description:
    Provides methods to send formatted debugging information.
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

#ifndef _WDRV_PIC32MZW_DEBUG_H
#define _WDRV_PIC32MZW_DEBUG_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "configuration.h"
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

#ifndef WDRV_PIC32MZW1_DEVICE_USE_SYS_DEBUG
// *****************************************************************************
/*  Debug Callback

  Summary:
    Defines the debug callback.

  Description:
    The function callback provides a printf-like prototype.

  Remarks:
    None.
*/

typedef void (*WDRV_PIC32MZW_DEBUG_PRINT_CALLBACK)(const char*, ...);

// *****************************************************************************
/*  Debug Levels

  Summary:
    Defines for debugging verbosity level.

  Description:
    These defines can be assigned to WDRV_PIC32MZW_DEBUG_LEVEL to affect the level of
      verbosity provided by the debug output channel.

  Remarks:
    None.
*/

#define WDRV_PIC32MZW_DEBUG_TYPE_NONE       0
#define WDRV_PIC32MZW_DEBUG_TYPE_ERROR      1
#define WDRV_PIC32MZW_DEBUG_TYPE_INFORM     2
#define WDRV_PIC32MZW_DEBUG_TYPE_TRACE      3
#define WDRV_PIC32MZW_DEBUG_TYPE_VERBOSE    4

// *****************************************************************************
/*  Debug Verbosity

  Summary:
    Defines the chosen level of debugging verbosity supported.

  Description:
    This define set the debugging output verbosity level.

  Remarks:
    None.
*/

#ifndef WDRV_PIC32MZW_DEBUG_LEVEL
#define WDRV_PIC32MZW_DEBUG_LEVEL   WDRV_PIC32MZW_DEBUG_TYPE_TRACE
#endif

// *****************************************************************************
/*  Debugging Macros

  Summary:
    Macros to define debugging output.

  Description:
    Macros are provided for each level of verbosity.

  Remarks:
    None.
*/

#define WDRV_DBG_VERBOSE_PRINT(...)
#define WDRV_DBG_TRACE_PRINT(...)
#define WDRV_DBG_INFORM_PRINT(...)
#define WDRV_DBG_ERROR_PRINT(...)

#if (WDRV_PIC32MZW_DEBUG_LEVEL >= WDRV_PIC32MZW_DEBUG_TYPE_ERROR)
#undef WDRV_DBG_ERROR_PRINT
#define WDRV_DBG_ERROR_PRINT(...) do { if (pfPIC32MZWDebugPrintCb) { pfPIC32MZWDebugPrintCb(__VA_ARGS__); } } while (0)
#if (WDRV_PIC32MZW_DEBUG_LEVEL >= WDRV_PIC32MZW_DEBUG_TYPE_INFORM)
#undef WDRV_DBG_INFORM_PRINT
#define WDRV_DBG_INFORM_PRINT(...) do { if (pfPIC32MZWDebugPrintCb) { pfPIC32MZWDebugPrintCb(__VA_ARGS__); } } while (0)
#if (WDRV_PIC32MZW_DEBUG_LEVEL >= WDRV_PIC32MZW_DEBUG_TYPE_TRACE)
#undef WDRV_DBG_TRACE_PRINT
#define WDRV_DBG_TRACE_PRINT(...) do { if (pfPIC32MZWDebugPrintCb) { pfPIC32MZWDebugPrintCb(__VA_ARGS__); } } while (0)
#if (WDRV_PIC32MZW_DEBUG_LEVEL >= WDRV_PIC32MZW_DEBUG_TYPE_VERBOSE)
#undef WDRV_DBG_VERBOSE_PRINT
#define WDRV_DBG_VERBOSE_PRINT(...) do { if (pfPIC32MZWDebugPrintCb) { pfPIC32MZWDebugPrintCb(__VA_ARGS__); } } while (0)
#endif /* WDRV_PIC32MZW_DEBUG_TYPE_VERBOSE */
#endif /* WDRV_PIC32MZW_DEBUG_TYPE_TRACE */
#endif /* WDRV_PIC32MZW_DEBUG_TYPE_INFORM */
#endif /* WDRV_PIC32MZW_DEBUG_TYPE_ERROR */

// Reference debug output channel printf-like function.
extern WDRV_PIC32MZW_DEBUG_PRINT_CALLBACK pfPIC32MZWDebugPrintCb;
#else
#define WDRV_DBG_VERBOSE_PRINT(...)         do { _SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, __VA_ARGS__); } while (0)
#define WDRV_DBG_TRACE_PRINT(...)           do { _SYS_DEBUG_PRINT(SYS_ERROR_INFO, __VA_ARGS__); } while (0)
#define WDRV_DBG_INFORM_PRINT(...)          do { _SYS_DEBUG_PRINT(SYS_ERROR_WARNING, __VA_ARGS__); } while (0)
#define WDRV_DBG_ERROR_PRINT(...)           do { _SYS_DEBUG_PRINT(SYS_ERROR_ERROR, __VA_ARGS__); } while (0)

#endif

// Function to receive MAC RX packets for inspection
//#define WDRV_PIC32MZW_MAC_RX_PKT_INSPECT_HOOK

// Function to receive MAC TX packets for inspection
//#define WDRV_PIC32MZW_MAC_TX_PKT_INSPECT_HOOK

#ifdef WDRV_PIC32MZW_MAC_RX_PKT_INSPECT_HOOK
void WDRV_PIC32MZW_MAC_RX_PKT_INSPECT_HOOK(const TCPIP_MAC_PACKET *const ptrPacket);
#endif

#ifdef WDRV_PIC32MZW_MAC_TX_PKT_INSPECT_HOOK
void WDRV_PIC32MZW_MAC_TX_PKT_INSPECT_HOOK(const TCPIP_MAC_PACKET *const ptrPacket);
#endif

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _WDRV_PIC32MZW_DEBUG_H */
