/*******************************************************************************
  PIC32MZW Driver Custom IE Implementation

  File Name:
    wdrv_pic32mzw_custie.c

  Summary:
    PIC32MZW wireless driver custom IE implementation.

  Description:
    This file provides an interface for creating and manipulating the vendor
    specific information element store. Custom IE's can be added to management
    frames and also IEs can be received from management frames.
 ******************************************************************************/

//DOM-IGNORE-BEGIN
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
// Section: PIC32MZW Driver Custom IE Store Context Implementation
// *****************************************************************************
// *****************************************************************************

//******************************************************************************
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
    The caller provides storage/memory for the custom IE store, this will be
    initialized and a pointer will be provided which can be used while calling
    the API WDRV_PIC32MZW_CustIESetTxData after custom IEs are added by API
    WDRV_PIC32MZW_CustIEStoreCtxAddIE.

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
