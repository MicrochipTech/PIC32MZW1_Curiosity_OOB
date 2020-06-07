/*******************************************************************************
  PIC32MZW Driver Association Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_assoc.h

  Summary:
    PIC32MZW wireless driver association header file.

  Description:
    This interface provides information about the current association with a
    peer device.
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

#ifndef _WDRV_PIC32MZW_ASSOC_H
#define _WDRV_PIC32MZW_ASSOC_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_authctx.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Association Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  BSS Information

  Summary:
    Structure containing information about an association.

  Description:
    This structure contains the association information for a connection between
      an AP and STA.

  Remarks:
    None.
*/

typedef struct
{
    /* Primary driver handle. */
    DRV_HANDLE handle;

    /* MAC address of peer device. */
    WDRV_PIC32MZW_MAC_ADDR peerAddress;

    /* Authentication type used. */
    WDRV_PIC32MZW_AUTH_TYPE authType;

    /* Last RSSI value read. */
    int8_t rssi;
} WDRV_PIC32MZW_ASSOC_INFO;

// *****************************************************************************
/*  Association RSSI Callback.

  Summary:
    A callback to provide the current RSSI of the current association.

  Description:
    This callback provides details of the signal strength (RSSI) of the current
      association.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    assocHandle - Association handle.
    rssi        - The current RSSI of the association.

  Returns:
    None.

  Remarks:
    None.
*/

typedef void (*WDRV_PIC32MZW_ASSOC_RSSI_CALLBACK)
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
    int8_t rssi
);

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Association Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocPeerAddressGet
    (
        WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
        WDRV_PIC32MZW_MAC_ADDR *const pPeerAddress
    )

  Summary:
    Retrieve the current association peer device network address.

  Description:
    Attempts to retrieve the network address of the peer device in the
      current association.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
    A peer device needs to be connected and associated.

  Parameters:
    assocHandle  - Association handle.
    pPeerAddress - Pointer to structure to receive the network address.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - pPeerAddress will contain the network address.
    WDRV_PIC32MZW_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR  - The request to the PIC32MZW was rejected
                                            or there is no current association.
    WDRV_PIC32MZW_STATUS_RETRY_REQUEST  - The network address is not available
                                            but it will be requested from the PIC32MZW.
    WDRV_PIC32MZW_STATUS_NOT_CONNECTED  - Not currently connected.

  Remarks:
    If the network address is not currently known to the driver (stored within the
      PIC32MZW) a request will be sent to the PIC32MZW and the return status
      will be WDRV_PIC32MZW_STATUS_RETRY_REQUEST.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocPeerAddressGet
(
    WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
    WDRV_PIC32MZW_MAC_ADDR *const pPeerAddress
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocRSSIGet
    (
        WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
        int8_t *const pRSSI,
        WDRV_PIC32MZW_ASSOC_RSSI_CALLBACK const pfAssociationRSSICB
    )

  Summary:
    Retrieve the current association RSSI.

  Description:
    Attempts to retrieve the RSSI of the current association.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
    A peer device needs to be connected and associated.

  Parameters:
    assocHandle         - Association handle.
    pRSSI               - Pointer to variable to receive RSSI if available.
    pfAssociationRSSICB - Pointer to callback function to be used when
                            RSSI value is available.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - pRSSI will contain the RSSI.
    WDRV_PIC32MZW_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR  - The request to the PIC32MZW was rejected
                                            or there is no current association.
    WDRV_PIC32MZW_STATUS_RETRY_REQUEST  - The RSSI is not available but it will
                                            be requested from the PIC32MZW.
    WDRV_PIC32MZW_STATUS_NOT_CONNECTED  - Not currently connected.

  Remarks:
    If the RSSI is not currently known to the driver (stored within the
      PIC32MZW) a request will be sent to the PIC32MZW and the return status
      will be WDRV_PIC32MZW_STATUS_RETRY_REQUEST. The callback function
      pfAssociationRSSICB can be provided which will be called when the PIC32MZW
      provides the RSSI information to the driver. Alternatively the caller
      may poll this function until the return status is WDRV_PIC32MZW_STATUS_OK
      to obtain the RSSI in pRSSI.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocRSSIGet
(
    WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
    int8_t *const pRSSI,
    WDRV_PIC32MZW_ASSOC_RSSI_CALLBACK const pfAssociationRSSICB
);

#endif /* _WDRV_PIC32MZW_ASSOC_H */
