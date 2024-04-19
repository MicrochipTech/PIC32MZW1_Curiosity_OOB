/*******************************************************************************
  WLAN Medium Access Control (MAC) Layer Framework

  File Name:
    wdrv_pic32mzw_cfg.c

  Summary:
    This  module provide the interface of WLAN MAC functionality.

  Description:
*******************************************************************************/

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
        pCtx->error  = true;
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

void DRV_PIC32MZW_MultiWIDDestroy(DRV_PIC32MZW_WIDCTX *pCtx)
{
    if ((NULL == pCtx) || (NULL == pCtx->buffer))
    {
        return;
    }

    DRV_PIC32MZW_MemFree(DRV_PIC32MZW_ALLOC_OPT_PARAMS pCtx->buffer);
    pCtx->buffer = NULL;
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

bool DRV_PIC32MZW_MultiWIDWrite(DRV_PIC32MZW_WIDCTX *pCtx)
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
            return false;
        }

        pCtx->buffer[2] = (length & 0xff);
        pCtx->buffer[3] = ((length >> 8) & 0xff);

        DRV_PIC32MZW_WIDTxQueuePush(pCtx->buffer);
    }
    else
    {
        return false;
    }

    pCtx->buffer = NULL;

    return true;
}
