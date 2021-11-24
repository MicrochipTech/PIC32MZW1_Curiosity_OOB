/*******************************************************************************
  PIC32MZW Driver Custom IE Implementation

  File Name:
    wdrv_pic32mzw_custie.c

  Summary:
    PIC32MZW wireless driver custom IE implementation.

  Description:
    This file provides an interface for manipulating the vendor specific
    information element store. Custom IE's can be included in the Soft-AP
    beacons and probe responses.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2021 released Microchip Technology Inc.  All rights reserved.

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
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_custie.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Custom IE Implementation
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
      and a pointer provided which can be passed to WDRV_PIC32MZW_APSetCustIE after
      custom IEs are added by WDRV_PIC32MZW_CustIEStoreCtxAddIE.

  Remarks:
    See wdrv_pic32mzw_custie.h for usage information.

*/

WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT* WDRV_PIC32MZW_CustIEStoreCtxSetStorage
(
    uint8_t *const pStorage,
    uint16_t lenStorage
)
{
    WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *pCustIECtx;

    /* Ensure user application provided storage is valid. */
    if ((NULL == pStorage) || (lenStorage < WDRV_PIC32MZW_CUSTIE_MIN_STORAGE_LEN))
    {
        return NULL;
    }

    /* Initialize the storage area. */

    pCustIECtx = (WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT*)pStorage;

    memset(pStorage, 0, lenStorage);

    pCustIECtx->maxLength = (lenStorage > WDRV_PIC32MZW_CUSTIE_MAX_STORAGE_LEN + WDRV_PIC32MZW_CUSTIE_DATA_OFFSET) ? WDRV_PIC32MZW_CUSTIE_MAX_STORAGE_LEN : (lenStorage - WDRV_PIC32MZW_CUSTIE_DATA_OFFSET);

    return pCustIECtx;
}

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

  Remarks:
    See wdrv_pic32mzw_custie.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_CustIEStoreCtxAddIE
(
    WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id,
    const uint8_t *const pData,
    uint8_t dataLength
)
{
    WDRV_PIC32MZW_CUST_IE *pIE;
    uint16_t dataOffset;

    /* Ensure the storage context is valid. */
    if ((NULL == pCustIECtx) || (NULL == pData))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }
    
    /* Ensure the new IE will fit. */
    if ((pCustIECtx->maxLength - pCustIECtx->curLength) < (dataLength + WDRV_PIC32MZW_CUSTIE_DATA_OFFSET))
    {
        return WDRV_PIC32MZW_STATUS_NO_SPACE;
    }

    /* Walk the IEs until the end is found. */
    dataOffset = 0;
    pIE = (WDRV_PIC32MZW_CUST_IE*)&pCustIECtx->ieData[dataOffset];

    while((0 != pIE->id) && (dataOffset < pCustIECtx->curLength))
    {
        dataOffset += (WDRV_PIC32MZW_CUSTIE_DATA_OFFSET + pIE->length);
        pIE = (WDRV_PIC32MZW_CUST_IE*)&pCustIECtx->ieData[dataOffset];
    }

    /* Copy in new IE. */
    pIE->id = id;
    pIE->length = dataLength;
    memcpy(&pIE->data, pData, dataLength);

    pCustIECtx->curLength += (dataLength + WDRV_PIC32MZW_CUSTIE_DATA_OFFSET);

    return WDRV_PIC32MZW_STATUS_OK;
}

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

  Remarks:
    See wdrv_pic32mzw_custie.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_CustIEStoreCtxRemoveIE
(
    WDRV_PIC32MZW_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id
)
{
    WDRV_PIC32MZW_CUST_IE *pIE;
    uint16_t dataOffset;

    /* Ensure the storage context and ID are valid. */
    if ((NULL == pCustIECtx) || (0 == id))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Walk the IEs looking for the supplied ID. */
    dataOffset = 0;
    pIE = (WDRV_PIC32MZW_CUST_IE*)&pCustIECtx->ieData[dataOffset];

    while((0 != pIE->id) && (dataOffset < pCustIECtx->curLength))
    {
        if (pIE->id == id)
        {
            /* The ID has been found, copy remaining IEs over the top to remove it. */
            pCustIECtx->curLength -= (WDRV_PIC32MZW_CUSTIE_DATA_OFFSET + pIE->length);

            memcpy(&pCustIECtx->ieData[dataOffset],
                    &pCustIECtx->ieData[dataOffset+(WDRV_PIC32MZW_CUSTIE_DATA_OFFSET + pIE->length)],
                    pCustIECtx->maxLength - dataOffset - (WDRV_PIC32MZW_CUSTIE_DATA_OFFSET + pIE->length));

            pIE = (WDRV_PIC32MZW_CUST_IE*)&pCustIECtx->ieData[pCustIECtx->curLength];

            pIE->id = 0;
            pIE->length = 0;
            break;
        }

        dataOffset += (WDRV_PIC32MZW_CUSTIE_DATA_OFFSET + pIE->length);
        pIE = (WDRV_PIC32MZW_CUST_IE*)&pCustIECtx->ieData[dataOffset];
    }
    
    return WDRV_PIC32MZW_STATUS_OK;
}
