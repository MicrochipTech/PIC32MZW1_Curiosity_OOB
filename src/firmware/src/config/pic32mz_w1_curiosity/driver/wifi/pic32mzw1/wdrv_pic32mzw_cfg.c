/*******************************************************************************
  WLAN Medium Access Control (MAC) Layer Framework

  File Name:
    wdrv_pic32mzw_cfg.c

  Summary:
    This  module provide the interface of WLAN MAC functionality.

  Description:
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020-21 released Microchip Technology Inc.  All rights reserved.

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

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_cfg.h"

void DRV_PIC32MZW_WIDTxQueuePush(void *pPktBuff);

static const uint8_t widTypeLenMap[3] = {1, 2, 4};

void DRV_PIC32MZW_ProcessHostRsp(uint8_t *pHostRsp)
{
    uint8_t *pRespPtr;
    uint16_t len;
    uint16_t wid;
    uint16_t hostRspLen;

    if (NULL == pHostRsp)
    {
        return;
    }

    pRespPtr = pHostRsp;

    if ('R' != *pRespPtr++)
    {
        return;
    }

    pRespPtr++;

    hostRspLen = *pRespPtr++;
    hostRspLen |= ((uint16_t)(*pRespPtr++) << 8);

    hostRspLen -= 4;

    while (hostRspLen >= 3)
    {
        wid = *pRespPtr++;
        wid |= ((uint16_t)*pRespPtr++) << 8;

        len = *pRespPtr++;

        if (DRV_WIFI_WID_BIN_DATA == (wid >> 12))
        {
            if (hostRspLen < 4)
            {
                break;
            }

            len |= ((uint16_t)(*pRespPtr++) << 8);
            hostRspLen--;
        }

        hostRspLen -= 3;

        if (hostRspLen < len)
        {
            break;
        }

        WDRV_PIC32MZW_WIDProcess(wid, len, pRespPtr);

        hostRspLen -= len;
        pRespPtr += len;

        if (DRV_WIFI_WID_BIN_DATA == (wid >> 12))
        {
            if (0 == hostRspLen)
            {
                break;
            }

            pRespPtr++;
            hostRspLen--;
        }
    }

    DRV_PIC32MZW_PacketMemFree(DRV_PIC32MZW_ALLOC_OPT_PARAMS pHostRsp);
}

bool DRV_PIC32MZW_MultiWIDInit(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t bufferLen)
{
    if (NULL == pCtx)
    {
        return false;
    }

    pCtx->buffer = DRV_PIC32MZW_MemAlloc(DRV_PIC32MZW_ALLOC_OPT_PARAMS bufferLen);

    if (NULL == pCtx->buffer)
    {
        pCtx->pInPtr = NULL;
        return false;
    }

    pCtx->maxBufferLen  = bufferLen;
    pCtx->error         = false;
    pCtx->opType        = DRV_PIC32MZW_WIDOPTYPE_UNDEFINED;

    pCtx->buffer[0] = 0;
    pCtx->buffer[1] = 0;

    pCtx->pInPtr = &pCtx->buffer[4];

    return true;
}

bool DRV_PIC32MZW_MultiWIDAddValue(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t wid, uint32_t val)
{
    DRV_WIFI_WID_TYPE_T widType;

    if (NULL == pCtx)
    {
        return false;
    }

    if ((true == pCtx->error) || (NULL == pCtx->buffer) || (NULL == pCtx->pInPtr))
    {
        return false;
    }

    if ((DRV_PIC32MZW_WIDOPTYPE_WRITE != pCtx->opType) && (DRV_PIC32MZW_WIDOPTYPE_UNDEFINED != pCtx->opType))
    {
        pCtx->error = true;
        return false;
    }

    widType = wid >> 12;

    if (widType > DRV_WIFI_WID_INT)
    {
        pCtx->error = true;
        return false;
    }

    if (((pCtx->pInPtr - pCtx->buffer) + widTypeLenMap[widType] + 3) > pCtx->maxBufferLen)
    {
        pCtx->error = true;
        return false;
    }

    *pCtx->pInPtr++ = (wid & 0xff);
    *pCtx->pInPtr++ = ((wid >> 8) & 0xff);
    *pCtx->pInPtr++ = widTypeLenMap[widType];

    switch (widType)
    {
        case DRV_WIFI_WID_INT:
            *pCtx->pInPtr++ = (val & 0xff);
            val >>= 8;
            *pCtx->pInPtr++ = (val & 0xff);
            val >>= 8;

        case DRV_WIFI_WID_SHORT:
            *pCtx->pInPtr++ = (val & 0xff);
            val >>= 8;

        case DRV_WIFI_WID_CHAR:
            *pCtx->pInPtr++ = (val & 0xff);
            break;

        default:
            break;
    }

    pCtx->opType = DRV_PIC32MZW_WIDOPTYPE_WRITE;

    return true;
}

bool DRV_PIC32MZW_MultiWIDAddData(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t wid, const uint8_t *pData, uint16_t length)
{
    int i;
    DRV_WIFI_WID_TYPE_T widType;

    if (NULL == pCtx)
    {
        return false;
    }

    if ((true == pCtx->error) || (NULL == pCtx->buffer) || (NULL == pCtx->pInPtr))
    {
        return false;
    }

    if ((DRV_PIC32MZW_WIDOPTYPE_WRITE != pCtx->opType) && (DRV_PIC32MZW_WIDOPTYPE_UNDEFINED != pCtx->opType))
    {
        pCtx->error = true;
        return false;
    }

    if (((pCtx->pInPtr - pCtx->buffer) + length + 5) > pCtx->maxBufferLen)
    {
        pCtx->error = true;
        return false;
    }

    widType = wid >> 12;

    *pCtx->pInPtr++ = (wid & 0xff);
    *pCtx->pInPtr++ = ((wid >> 8) & 0xff);
    *pCtx->pInPtr++ = (length & 0xff);

    if (DRV_WIFI_WID_BIN_DATA == widType)
    {
        *pCtx->pInPtr++ = ((length >> 8) & 0xff);
    }

    if (length > 0)
    {
        memcpy(pCtx->pInPtr, pData, length);
        pCtx->pInPtr += length;
    }

    if (DRV_WIFI_WID_BIN_DATA == widType)
    {
        *pCtx->pInPtr = 0;
        for (i=0; i<length; i++)
        {
            *pCtx->pInPtr += pData[i];
        }
        pCtx->pInPtr++;
    }

    pCtx->opType = DRV_PIC32MZW_WIDOPTYPE_WRITE;

    return true;
}

bool DRV_PIC32MZW_MultiWIDAddString(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t wid, const char *pStr)
{
    if (NULL == pCtx)
    {
        return false;
    }

    return DRV_PIC32MZW_MultiWIDAddData(pCtx, wid, (uint8_t*)pStr, strlen(pStr));
}

bool DRV_PIC32MZW_MultiWIDAddQuery(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t wid)
{
    if (NULL == pCtx)
    {
        return false;
    }

    if ((true == pCtx->error) || (NULL == pCtx->buffer) || (NULL == pCtx->pInPtr))
    {
        return false;
    }

    if ((DRV_PIC32MZW_WIDOPTYPE_QUERY != pCtx->opType) && (DRV_PIC32MZW_WIDOPTYPE_UNDEFINED != pCtx->opType))
    {
        pCtx->error = true;
        return false;
    }

    if (((pCtx->pInPtr - pCtx->buffer) + 2) > pCtx->maxBufferLen)
    {
        pCtx->error = true;
        return false;
    }

    *pCtx->pInPtr++ = (wid & 0xff);
    *pCtx->pInPtr++ = ((wid >> 8) & 0xff);

    pCtx->opType = DRV_PIC32MZW_WIDOPTYPE_QUERY;

    return true;
}

bool DRV_PIC32MZW_MultiWid_Write(DRV_PIC32MZW_WIDCTX *pCtx)
{
    uint16_t length;

    if (NULL == pCtx)
    {
        return false;
    }

    if ((NULL == pCtx->buffer) || (NULL == pCtx->pInPtr))
    {
        return false;
    }

    length = pCtx->pInPtr - pCtx->buffer;

    if ((length > 4) || (true == pCtx->error))
    {
        switch (pCtx->opType)
        {
            case DRV_PIC32MZW_WIDOPTYPE_WRITE:
            {
                pCtx->buffer[0] = 'W';
                break;
            }

            case DRV_PIC32MZW_WIDOPTYPE_QUERY:
            {
                pCtx->buffer[0] = 'Q';
                break;
            }

            default:
            {
                pCtx->error = true;
                break;
            }
        }

        if (true == pCtx->error)
        {
            DRV_PIC32MZW_MemFree(DRV_PIC32MZW_ALLOC_OPT_PARAMS pCtx->buffer);
            pCtx->buffer = NULL;
            return false;
        }

        pCtx->buffer[2] = (length & 0xff);
        pCtx->buffer[3] = ((length >> 8) & 0xff);

        DRV_PIC32MZW_WIDTxQueuePush(pCtx->buffer);
    }
    else
    {
        DRV_PIC32MZW_MemFree(DRV_PIC32MZW_ALLOC_OPT_PARAMS pCtx->buffer);
    }

    pCtx->buffer = NULL;

    return true;
}
