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

    /* Whether the AP sent a transition disable element in this association. */
    bool transitionDisable;

    /* Last RSSI value read. */
    int8_t rssi;

    /* Association ID. */
    uint8_t assocID;
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
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocTransitionDisableGet
    (
        WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
        bool *const pTransitionDisable
    )

  Summary:
    Determine whether transition algorithms should be disabled for this network.

  Description:
    Determines whether a WPA3 transition disable element was sent by the device
      (if the device is in AP mode) or received from the peer AP (if the device
      is in STA mode) during the 4-way handshake of the current association.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
    A peer device needs to be connected and associated.

  Parameters:
    assocHandle         - Association handle.
    pTransitionDisable  - Pointer to variable to receive the transition disable
                            status (true if transition algorithms should be
                            disabled, false otherwise).

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - pPeerAddress will contain the network address.
    WDRV_PIC32MZW_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR  - The request to the PIC32MZW was rejected
                                            or there is no current association.
    WDRV_PIC32MZW_STATUS_NOT_CONNECTED  - Not currently connected.

  Remarks:
    In STA mode, if the output is true, then the application should set the
      WDRV_PIC32MZW_AUTH_MOD_STA_TD modifier in the Auth context for all
      subsequent connection requests to this ESS.
    See WDRV_PIC32MZW_AuthCtxStaTransitionDisable.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocTransitionDisableGet
(
    WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
    bool *const pTransitionDisable
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

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocDisconnect(WDRV_PIC32MZW_ASSOC_HANDLE assocHandle)

  Summary:
    Disconnects an association.

  Description:
    This API disconnects the STA associated with softAP based on the input association handle.
    Association handle is updated to the application whenever a new STA is connected to the softAP
    This API can also be used in STA mode to disconnect the STA from an AP.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
    WDRV_PIC32MZW_APStart should have been called to start softAP in softAP mode.
    WDRV_PIC32MZW_BSSConnect should have been called to connect to an AP in STA mode.

  Parameters:
    assocHandle - WDRV_PIC32MZW_ASSOC_HANDLE obtained when STA joins an AP or an association is done.

  Returns:
    WDRV_PIC32MZW_STATUS_OK            - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR - The request to the PIC32MZW was rejected.
    WDRV_PIC32MZW_STATUS_NOT_CONNECTED - The association isn't valid.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AssocDisconnect(WDRV_PIC32MZW_ASSOC_HANDLE assocHandle);

#endif /* _WDRV_PIC32MZW_ASSOC_H */
