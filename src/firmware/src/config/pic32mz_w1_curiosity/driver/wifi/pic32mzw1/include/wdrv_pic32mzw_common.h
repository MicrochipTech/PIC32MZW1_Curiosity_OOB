/*******************************************************************************
  PIC32MZW Driver Common Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_common.h

  Summary:
    PIC32MZW wireless driver common header file.

  Description:
    This file provides common elements of the PIC32MZW driver API.
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

#ifndef _WDRV_PIC32MZW_COMMON_H
#define _WDRV_PIC32MZW_COMMON_H

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
#include "osal/osal.h"
#include "wdrv_pic32mzw_debug.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver RF and MAC Minimum configuration requirements
// *****************************************************************************
// *****************************************************************************
#define WDRV_PIC32MZW_RF_MAC_MIN_REQ_CONFIG (DRV_PIC32MZW_POWER_ON_CAL_CONFIG | \
                                             DRV_PIC32MZW_FACTORY_CAL_CONFIG | \
                                             DRV_PIC32MZW_GAIN_TABLE_CONFIG | \
                                             DRV_PIC32MZW_MAC_ADDRESS_CONFIG)

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Common Data Types
// *****************************************************************************
// *****************************************************************************

/* Maximum length of an SSID. */
#define WDRV_PIC32MZW_MAX_SSID_LEN              32

/* Address of a MAC address. */
#define WDRV_PIC32MZW_MAC_ADDR_LEN              6

/* Length of 40 bit WEP key. */
#define WDRV_PIC32MZW_WEP_40_KEY_STRING_SIZE    10

/* Length of 104 bit WEP key. */
#define WDRV_PIC32MZW_WEP_104_KEY_STRING_SIZE   26

/* Length of PSK (ASCII encoded binary). */
#define WDRV_PIC32MZW_PSK_LEN                   64

/* Maximum length of a WPA Personal Password. */
#define WDRV_PIC32MZW_MAX_PSK_PASSWORD_LEN      63

/* Minimum length of a WPA Personal Password. */
#define WDRV_PIC32MZW_MIN_PSK_PASSWORD_LEN      8

// *****************************************************************************
/*  WiFi Channels

  Summary:
    A list of supported WiFi channels.

  Description:
    A list of supported WiFi channels.

  Remarks:
    None.

*/

typedef enum _WDRV_PIC32MZW_CHANNEL_ID
{
    /* Any valid channel. */
    WDRV_PIC32MZW_CID_ANY,

    /* 2.4 GHz channel 1 - 2412 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH1,

    /* 2.4 GHz channel 2 - 2417 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH2,

    /* 2.4 GHz channel 3 - 2422 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH3,

    /* 2.4 GHz channel 4 - 2427 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH4,

    /* 2.4 GHz channel 5 - 2432 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH5,

    /* 2.4 GHz channel 6 - 2437 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH6,

    /* 2.4 GHz channel 7 - 2442 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH7,

    /* 2.4 GHz channel 8 - 2447 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH8,

    /* 2.4 GHz channel 9 - 2452 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH9,

    /* 2.4 GHz channel 10 - 2457 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH10,

    /* 2.4 GHz channel 11 - 2462 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH11,

    /* 2.4 GHz channel 12 - 2467 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH12,

    /* 2.4 GHz channel 13 - 2472 MHz. */
    WDRV_PIC32MZW_CID_2_4G_CH13
} WDRV_PIC32MZW_CHANNEL_ID;

// *****************************************************************************
/*  Common API Return Status Code

  Summary:
    API return status codes.

  Description:
    All API functions which return a status code will use one of these to be
      consistent.

  Remarks:
    None.

*/

typedef enum _WDRV_PIC32MZW_STATUS
{
    /* Operation was successful. */
    WDRV_PIC32MZW_STATUS_OK = 0,

    /* Driver instance has not been opened. */
    WDRV_PIC32MZW_STATUS_NOT_OPEN,

    /* The arguments supplied are not valid. */
    WDRV_PIC32MZW_STATUS_INVALID_ARG,

    /* A scan operation is currently in progress. */
    WDRV_PIC32MZW_STATUS_SCAN_IN_PROGRESS,

    /* No BSS information is available. */
    WDRV_PIC32MZW_STATUS_NO_BSS_INFO,

    /* No more BSS scan results are available. */
    WDRV_PIC32MZW_STATUS_BSS_FIND_END,

    /* The connection attempt has failed. */
    WDRV_PIC32MZW_STATUS_CONNECT_FAIL,

    /* The disconnection attempt has failed. */
    WDRV_PIC32MZW_STATUS_DISCONNECT_FAIL,

    /* The requested operation could not be completed. */
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR,

    /* The context being referenced is invalid. */
    WDRV_PIC32MZW_STATUS_INVALID_CONTEXT,

    /* Request could not complete, but may if tried again. */
    WDRV_PIC32MZW_STATUS_RETRY_REQUEST,

    /* Out of space in resource. */
    WDRV_PIC32MZW_STATUS_NO_SPACE,

    /* No Ethernet buffer was available. */
    WDRV_PIC32MZW_STATUS_NO_ETH_BUFFER,

    /* Not currently connected. */
    WDRV_PIC32MZW_STATUS_NOT_CONNECTED,

    /* RF or MAC configuration is not configured*/
    WDRV_PIC32MZW_STATUS_RF_MAC_CONFIG_NOT_VALID,

    /* The requested operation is not supported. */
    WDRV_PIC32MZW_STATUS_OPERATION_NOT_SUPPORTED
} WDRV_PIC32MZW_STATUS;

// *****************************************************************************
/*  Extended system status

  Summary:
    Defines extended system states.

  Description:
    An extended state gives information about availability of RF config

  Remarks:
    None.

*/

typedef enum
{
    /* RF initialisation is in progress*/
    WDRV_PIC32MZW_SYS_STATUS_RF_INIT_BUSY = SYS_STATUS_READY_EXTENDED,

    /* RF configuration is missing */
    WDRV_PIC32MZW_SYS_STATUS_RF_CONF_MISSING,

    /* RF is configured and is ready */
    WDRV_PIC32MZW_SYS_STATUS_RF_READY

} WDRV_PIC32MZW_SYS_STATUS;

// *****************************************************************************
/*  Connection State

  Summary:
    Defines possible connection states.

  Description:
    A connection can currently either be connected or disconnect.

  Remarks:
    None.

*/

typedef enum
{
    /* Association state is disconnected. */
    WDRV_PIC32MZW_CONN_STATE_DISCONNECTED,

    /* Association state is connecting. */
    WDRV_PIC32MZW_CONN_STATE_CONNECTING,

    /* Association state is connected. */
    WDRV_PIC32MZW_CONN_STATE_CONNECTED,

    /* Association state is connection failed. */
    WDRV_PIC32MZW_CONN_STATE_FAILED,

} WDRV_PIC32MZW_CONN_STATE;

// *****************************************************************************
/*  SSID

  Summary:
    Structure to hold an SSID.

  Description:
    The SSID consist of a buffer and a length field.

  Remarks:
    None.

*/

typedef struct _WDRV_PIC32MZW_SSID
{
    /* SSID name, up to WDRV_PIC32MZW_MAX_SSID_LEN characters long. */
    uint8_t name[WDRV_PIC32MZW_MAX_SSID_LEN];

    /* Length of SSID name. */
    uint8_t length;
} WDRV_PIC32MZW_SSID;

// *****************************************************************************
/*  SSID Linked List

  Summary:
    Structure to hold an SSID linked list element.

  Description:
    An element structure which can form part of an SSID linked list.

  Remarks:
    None.

*/

typedef struct _WDRV_PIC32MZW_SSID_LIST
{
    /* Pointer to next SSID element in list. */
    struct _WDRV_PIC32MZW_SSID_LIST *pNext;

    /* SSID structure. */
    WDRV_PIC32MZW_SSID ssid;
} WDRV_PIC32MZW_SSID_LIST;


// *****************************************************************************
/*  MAC Address

  Summary:
    Structure to hold a MAC address.

  Description:
    The MAC address consist of a buffer and a valid flag.

  Remarks:
    None.

*/

typedef struct _WDRV_PIC32MZW_MAC_ADDR
{
    /* MAC address, must be WDRV_PIC32MZW_MAC_ADDR_LEN characters long. */
    uint8_t addr[WDRV_PIC32MZW_MAC_ADDR_LEN];

    /* Is the address valid? */
    bool valid;
} WDRV_PIC32MZW_MAC_ADDR;

// *****************************************************************************
/*  Association Handle

  Summary:
    A handle representing an association instance.

  Description:
    An association handle references a single association instance between AP and STA.

  Remarks:
    None.

*/

typedef uintptr_t WDRV_PIC32MZW_ASSOC_HANDLE;

// *****************************************************************************
/* Invalid Association Handle

 Summary:
    Invalid association handle.

 Description:
    Defines a value for an association handle which isn't yet valid.

 Remarks:
    None.
*/

#define WDRV_PIC32MZW_ASSOC_HANDLE_INVALID  (((WDRV_PIC32MZW_ASSOC_HANDLE) -1))

// *****************************************************************************
/*  Connection Notify Callback

  Summary:
    Callback to notify the user of a change in connection state.

  Description:
    When the connection state changes this callback enable the driver to signal
      the user about that event and reason.

  Parameters:
    handle          - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    assocHandle     - Association handle.
    currentState    - Current connection state.

  Returns:
    None.

  Remarks:
    None.

*/

typedef void (*WDRV_PIC32MZW_BSSCON_NOTIFY_CALLBACK)
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
    WDRV_PIC32MZW_CONN_STATE currentState
);

// *****************************************************************************
/* Generic Status Callback Function Pointer

  Summary:
    Pointer to a generic status callback function.

  Description:
    This defines a generic status function callback type which can be passed
    into certain functions to receive feedback.

  Parameters:
    handle  - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    status  - A status value.

  Returns:
    None.

  Remarks:
    The value of the status passed to the function is dependant on the function
    used to register the callback.

    See WDRV_PIC32MZW_OTAUpdateFromURL, WDRV_PIC32MZW_SwitchActiveFirmwareImage,
    WDRV_PIC32MZW_HostFileRead and WDRV_PIC32MZW_HostFileErase.

*/

typedef void (*WDRV_PIC32MZW_STATUS_CALLBACK)
(
    DRV_HANDLE handle,
    uint8_t status
);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _WDRV_PIC32MZW_COMMON_H */
