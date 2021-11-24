/*******************************************************************************
  PIC32MZW Driver Custom IE Header File 

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_custie.h

  Summary:
    PIC32MZW wireless driver custom IE.

  Description:
    This file provides an interface for manipulating the vendor specific 
    information element store. Custom VSIE's can be included in the Soft-AP
    beacons and probe responses.
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

#ifndef _WDRV_PIC32MZW_CUSTIE_H
#define _WDRV_PIC32MZW_CUSTIE_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "wdrv_pic32mzw_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Custom IE Data Types
// *****************************************************************************
// *****************************************************************************

#define WDRV_PIC32MZW_CUSTIE_MAX_STORAGE_LEN    DRV_PIC32MZW_MAX_VSIE_DATA_LEN
#define WDRV_PIC32MZW_CUSTIE_MIN_STORAGE_LEN    DRV_PIC32MZW_VSIE_DATA_SIZE_FIELD_LEN
#define WDRV_PIC32MZW_CUSTIE_DATA_OFFSET        DRV_PIC32MZW_VSIE_DATA_SIZE_FIELD_LEN


// *****************************************************************************
/*  Custom IE Structure

  Summary:
    Defines the format of a custom IE.

  Description:
    Custom IEs consist of an ID, length and data.

  Remarks:
    This definition does not allocate any storage for data.
*/

typedef struct
{
    /* ID. */
    uint8_t id;

    /* Length of data. */
    uint8_t length;

    /* Data. */
    uint8_t data[];
} WDRV_PIC32MZW_CUST_IE;

// *****************************************************************************
/*  Custom IE Store Structure

  Summary:
    Defines the storage used for holding custom IEs.

  Description:
    Custom IEs are passed to the Soft-AP via the custom IE store which packages
    the IEs together.

  Remarks:
    None.
*/

typedef struct
{
    /* Maximum length of the IE store data. */
    uint16_t maxLength;

    /* Current length of data in the store. */
    uint16_t curLength;

    /* IE data in store. */
    uint8_t ieData[];
} WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Custom IE Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT* WDRV_PIC32MZW_CustIEStoreCtxSetStorage
    (
        uint8_t *const pStorage,
        uint16_t lenStorage
    )

  Summary:
    Initialize the custom IE store.

  Description:
    The caller provides storage for the custom IE store, this will be initialized
    and a pointer provided which can be passed to WDRV_PIC32MZW_APSetCustIE
    after custom IEs are added by WDRV_PIC32MZW_CustIEStoreCtxAddIE.

  Precondition:
    None.

  Parameters:
    pStorage   - Pointer to storage to use for custom IE store.
    lenStorage - Length of storage pointed to by pStorage.

  Returns:
    Pointer to custom IE store, or NULL if error occurs.

  Remarks:
    lenStorage should be: (WDRV_PIC32MZW_CUSTIE_MIN_STORAGE_LEN <= lenStorage
    <= WDRV_PIC32MZW_CUSTIE_MAX_STORAGE_LEN + WDRV_PIC32MZW_CUSTIE_DATA_OFFSET).
    If less than WDRV_PIC32MZW_CUSTIE_MIN_STORAGE_LEN an error will be signalled,
    if more than WDRV_PIC32MZW_CUSTIE_MAX_STORAGE_LEN + WDRV_PIC32MZW_CUSTIE_DATA_OFFSET
    then WDRV_PIC32MZW_CUSTIE_MAX_STORAGE_LEN + WDRV_PIC32MZW_CUSTIE_DATA_OFFSET
    bytes will be used.

*/

WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT* WDRV_PIC32MZW_CustIEStoreCtxSetStorage
(
    uint8_t *const pStorage,
    uint16_t lenStorage
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_CustIEStoreCtxAddIE
    (
        WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx,
        uint8_t id,
        const uint8_t *const pData,
        uint8_t dataLength
    )

  Summary:
    Add data to the custom IE store.

  Description:
    The data and ID provided are copied into the custom IE store.

  Precondition:
    WDRV_PIC32MZW_CustIEStoreCtxSetStorage must have been called.

  Parameters:
    pCustIECtx - Pointer to custom IE store.
    id         - ID of custom IE.
    pData      - Pointer to data to be stored in the custom IE.
    dataLength - Length of data pointed to by pData.

  Returns:
    WDRV_PIC32MZW_STATUS_OK          - The data was added successfully.
    WDRV_PIC32MZW_STATUS_INVALID_ARG - The parameters were incorrect.
    WDRV_PIC32MZW_STATUS_NO_SPACE    - The data will not fit into the custom IE store.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_CustIEStoreCtxAddIE
(
    WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id,
    const uint8_t *const pData,
    uint8_t dataLength
);

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_CustIEStoreCtxRemoveIE
    (
        WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx,
        uint8_t id
    )

  Summary:
    Removes data from the custom IE store.

  Description:
    This function removes a custom IE from the store which matches the ID provided.

  Precondition:
    WDRV_PIC32MZW_CustIEStoreCtxSetStorage must have been called.

  Parameters:
    pCustIECtx - Pointer to custom IE store.
    id         - ID of custom IE to remove.

  Returns:
    WDRV_PIC32MZW_STATUS_OK          - The data was added successfully.
    WDRV_PIC32MZW_STATUS_INVALID_ARG - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_CustIEStoreCtxRemoveIE
(
    WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id
);

#endif /* _WDRV_PIC32MZW_CUSTIE_H */
