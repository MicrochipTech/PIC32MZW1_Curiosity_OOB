/*******************************************************************************
  PIC32MZW Driver IE Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_ie.h

  Summary:
    PIC32MZW wireless driver IE.

  Description:
    This file provides an interface for manipulating the vendor specific
    information element. Custom IE's can be added to management frames and also
    IEs can be received from management frames.
 ******************************************************************************/

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

#ifndef _WDRV_PIC32MZW_IE_H
#define _WDRV_PIC32MZW_IE_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_custie.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Custom IE Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Vendor IE frame filter mask for Rx and Tx of management frames

  Summary:
    Defines frame filter mask values which is used while Rx/Tx of vendor IE tags.

  Description:
    Filter mask enables the application to choose the frame types on which
    custom IE can be added while Tx and the frame types from which vendor
    specific IE data can be extracted at Rx.

  Remarks:
    Bit position value 1 enables and 0 disables Rx/Tx of vendor IE data.
*/

typedef enum
{
    /* If set in Tx API, the custom IE data provided by the application will be
     * added to the beacon frames while transmitting beacons. If set in Rx API,
     * the vendor specific IE data from the received beacon frames will be
     * passed to the application. */
    WDRV_PIC32MZW_VENDOR_IE_BEACON       = 0x01,

    /* If set in Tx API, the custom IE data provided by the application will be
     * added to the probe request frames while transmitting probe requests. If
     * set in Rx API, the vendor specific IE data from the received probe request
     * frames will be passed to the application. */
    WDRV_PIC32MZW_VENDOR_IE_PROBE_REQ    = 0x02,

    /* If set in Tx API the custom IE data provided by the application will be
     * added to the probe response frames while transmitting probe responses. If
     * set in Rx API, the vendor specific IE data from the received probe
     * response frames will be passed to the application. */
    WDRV_PIC32MZW_VENDOR_IE_PROBE_RSP    = 0x04

} WDRV_PIC32MZW_IE_FRAME_TYPE_MASK;

// *****************************************************************************
/* Vendor Specific IE Information

  Summary:
    Defines received vendor specific IE related information.

  Description:
    Provides more information on the received vendor specific IE such as MAC
    address of the source, frame type on which the IE was received and RSSI of
    the received frame.

  Remarks:
    RSSI is received in dBm.
    frameType is of type WDRV_PIC32MZW_IE_FRAME_TYPE_MASK.
*/

typedef struct
{
    uint8_t sa[6];
    int8_t rssi;
    WDRV_PIC32MZW_IE_FRAME_TYPE_MASK frameType;
} WDRV_PIC32MZW_VENDORIE_INFO;

// *****************************************************************************
/* Vendor Specific IE Receive Callback Function Pointer

  Summary:
    Pointer to a callback function to receive vendor specific IE data and
    related information.

  Description:
    This defines a function pointer to a vendor specific IE callback which will
    receive IE tag data and some relevant information like, source MAC address,
    received frame type and rssi of the received frame.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    totalLen    - Total length of received vendor specific IEs.
    ieData      - Pointer to a byte array of received vendor specific IEs.
    pIEInfo     - Pointer to WDRV_PIC32MZW_VENDORIE_INFO structure.

  Returns:
    None.

  Remarks:
    The callback has to be set to start receiving vendor specific IEs. The IEs
    are received based on the vendor OUI and frame filter mask set using the API
    WDRV_PIC32MZW_CustIEGetRxData.

*/

typedef void (*WDRV_PIC32MZW_IE_RX_CALLBACK)
(
    DRV_HANDLE handle,
    const WDRV_PIC32MZW_VENDORIE_INFO *const pIEInfo,
    const uint8_t *const pIEData,
    uint16_t dataLen
);

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver IE Routines
// *****************************************************************************
// *****************************************************************************

//******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_IECustTxDataSet
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_IE_FRAME_TYPE_MASK frameMask,
        const WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx
    )

  Summary:
    Configures the custom IE.

  Description:
    Management frames like, beacons, probe request and probe response may
    contain an application provided custom IE. This function associates a custom
    IE store context with the STA/AP instance.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.
    WDRV_PIC32MZW_CustIEStoreCtxSetStorage should have been called to create a
    valid IE store context.

  Parameters:
    handle     - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    frameMask  - Frame filter mask to add custom IE store.
    pCustIECtx - Pointer to custom IE store context.

  Returns:
    WDRV_PIC32MZW_STATUS_OK            - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR - The request to the PIC32MZW was rejected.

  Remarks:
    1. Before calling the API WDRV_PIC32MZW_IECustTxDataSet it is expected that IE
    storage is created using the API WDRV_PIC32MZW_CustIEStoreCtxSetStorage.
    2. IEs can be added to the storage or removed from the storage using APIs
    WDRV_PIC32MZW_CustIEStoreCtxAddIE and WDRV_PIC32MZW_CustIEStoreCtxRemoveIE.
    3. To stop transmitting custom vendor specific IEs, this API can be called
    with frameMask = 0.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_IECustTxDataSet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_IE_FRAME_TYPE_MASK frameMask,
    const WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx
);

//******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_IERxDataGet
    (
        DRV_HANDLE handle,
        WDRV_PIC32MZW_IE_FRAME_TYPE_MASK frameMask,
        const uint32_t vendorOUI,
        const WDRV_PIC32MZW_IE_RX_CALLBACK pfVendorIERxCB
    )

  Summary:
    Registers callback function for received vendor specific IE data.

  Description:
    Managements frames like beacons, probe request and probe response may contain
    vendor specific IEs. This API enables application to receive vendor specific
    IE extracted from management frames based on the vendor OUI and frame filter
    mask provided by the application.

  Precondition:
    WDRV_PIC32MZW_Initialize should have been called.
    WDRV_PIC32MZW_Open should have been called to obtain a valid handle.

  Parameters:
    handle          - Client handle obtained by a call to WDRV_PIC32MZW_Open.
    frameMask       - Frame filter mask of type WDRV_PIC32MZW_IE_FRAME_TYPE_MASK
    vendorOUI       - Vendor OUI of the organization.
    pfVendorIERxCB  - Pointer to callback function to receive IE data.

  Returns:
    WDRV_PIC32MZW_STATUS_OK            - The request has been accepted.
    WDRV_PIC32MZW_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_PIC32MZW_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_REQUEST_ERROR - The request to the PIC32MZW was rejected.

  Remarks:
    1. To start receiving vendor specific IE data application must provide valid
    frameMask and vendorOUI.
    2. To stop receiving IE data this API can be called with frameMask 0.
    3. The parameter vendorOUI is organization's OUI of which IEs are being
    looked for.
    Example 1: If the organization's OUI is 24 bit like 80-1F-12, the parameter
    vendorOUI is expected to be 0x801F12 or decimal 8396562.
    Example 2: If the organization's OUI is 36 bit OUI, the first three bytes
    (IEEE Registration Authority) of the OUI-36 should be used for the parameter
    vendorOUI. So, if the organization's 36 bit OUI is 70-B3-D5-77-F, the
    parameter vendorOUI is expected to be 0x70B3D5 or decimal 7386069. In this
    case the application will receive all IEs matching to the OUI value 70-B3-D5,
    then it is the application's job to filter the 77-F ones.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_IERxDataGet
(
    DRV_HANDLE handle,
    WDRV_PIC32MZW_IE_FRAME_TYPE_MASK frameMask,
    const uint32_t vendorOUI,
    const WDRV_PIC32MZW_IE_RX_CALLBACK pfVendorIERxCB
);

#endif /* _WDRV_PIC32MZW_IE_H */
