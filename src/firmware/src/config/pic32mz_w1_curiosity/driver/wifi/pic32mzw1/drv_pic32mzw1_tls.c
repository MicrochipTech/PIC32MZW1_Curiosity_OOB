/*******************************************************************************
  PIC32MZW1 wolfssl TLS interface for wireless driver and firmware to support 
  enterprise security.

  File Name:
    drv_pic32mzw1_tls.c

  Summary:
    PIC32MZW1 Wolfssl TLS interface for wireless driver and firmware to support 
  enterprise security..

  Description:
    PIC32MZW1 Wolfssl TLS interface for wireless driver and firmware to support 
    Enterprise security
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "wdrv_pic32mzw_common.h"
#include "drv_pic32mzw1_tls.h"
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
#include "tcpip/src/link_list.h"
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/logging.h"
#include "wolfssl/wolfcrypt/types.h"
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */

#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT

/* TLS record header length */
#define DRV_PIC32MZW1_TLS_RECORD_HEADER_LENGTH      5

#define DRV_TLS_RECORD_TYPE_HANDSHAKE_PROTOCOL      22
#define DRV_TLS_RECORD_TYPE_CHANGE_CIPHER_SPEC      20
#define DRV_TLS_RECORD_TYPE_ALERT_PROTOCOL          21
#define DRV_TLS_RECORD_TYPE_APPLICATION_DATA        23

/* This is configuration structure of driver TLS module for enterprise security */
typedef struct
{
    /* Wolfssl Context pointer */
    WOLFSSL_CTX* tlsWolfsslCtx;
    
    /* Wolfssl Session pointer */
    WOLFSSL* tlsWolfsslSess;
    
    /* Server domain name against which either server certificate's subject alternative 
     * name(SAN) or common name(CN) shall be matched for successful enterprise connection */
    char serverDomainName[WDRV_PIC32MZW_ENT_AUTH_SERVER_DOMAIN_LEN_MAX + 1];
    
    /* This is the wolfssl tls stack receive data queue. */
    PROTECTED_SINGLE_LIST tlsRxQueue;

    /* This is the wolfssl tls stack transmit data queue. */
    PROTECTED_SINGLE_LIST tlsTxQueue;
    
    /* TLS session callback function of upper layer through which TLS events 
     * will be notified */
    DRV_PIC32MZW1_TLS_EVENT_CB fpTlsEventCb;
    
    /* TLS session callback function argument */
    uint32_t tlsEventCbArg;
    
    /* Last TLS record type written to Tx Buffer*/
    DRV_PIC32MZW1_TLS_CLIENT_RECORD_TYPE tlsLastRecordType;
    
} DRV_PIC32MZW1_WOLFSSL_TLS_CONF;

/* This is a structure for maintaining tls packet information. */
typedef struct _DRV_PIC32MZW1_TLS_PKT
{
    struct _DRV_PIC32MZW1_TLS_PKT  *pNext;
    uint16_t bufferSize;
    uint16_t offsetRead;
    uint8_t  data[];
} DRV_PIC32MZW1_TLS_PKT;

static DRV_PIC32MZW1_WOLFSSL_TLS_CONF *g_pic32mzw1TlsConf = NULL;

static int DRV_TLS_HandshakeDoneCallback(WOLFSSL* pSSLCtx, void* context)
{
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)context;
    if ((NULL == pSSLCtx) || (NULL == pic32mzw1TlsConf))
    {
        return -1;
    }
	
    /* Inform the upper layer if TLS handshake is complete & successful */
    if (NULL != pic32mzw1TlsConf->fpTlsEventCb)
    {
        pic32mzw1TlsConf->fpTlsEventCb(DRV_PIC32MZW1_TLS_EVENT_SESSION_ESTABLISHED, NULL, pic32mzw1TlsConf->tlsEventCbArg);
    }
    return 0;
}

static bool DRV_TLS_GetRecordHdr
(
    DRV_PIC32MZW1_TLS_RECORD_HDR *pTlsRecordHdr, 
    uint8_t *pBuf, 
    int buffSize,
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf    
)
{
    uint8_t contentType = 0;
    uint8_t tlsRecordHandshakeType = 0;
    
    if ((NULL == pBuf) || (NULL == pic32mzw1TlsConf) || (NULL == pTlsRecordHdr) || (buffSize < DRV_PIC32MZW1_TLS_RECORD_HEADER_LENGTH))
    {
        return false;
    }
    
    pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_UNKNOWN;
    pTlsRecordHdr->tlsVersion = 0;
    pTlsRecordHdr->tlsRecordLength = 0;
    contentType = pBuf[0];
    pTlsRecordHdr->tlsVersion = ((pBuf[1] << 8) | pBuf[2]);
    pTlsRecordHdr->tlsRecordLength = ((pBuf[3] << 8) | pBuf[4]);
    if (0 != pTlsRecordHdr->tlsRecordLength)
    {
        tlsRecordHandshakeType = pBuf[5];
    }
    pTlsRecordHdr->tlsRecordLength += DRV_PIC32MZW1_TLS_RECORD_HEADER_LENGTH;
    if (pTlsRecordHdr->tlsRecordLength != buffSize)
    {
        //invalid record
        return false;
    }
    
    if(DRV_TLS_RECORD_TYPE_HANDSHAKE_PROTOCOL == contentType)
    {
        //TODO: Check how to figure out finished message from spec since the data is encrypted
        /* Client finished should always be the first message after change cipher spec */
        if(DRV_PIC32MZW1_TLS_RECORD_TYPE_CLIENT_CHANGE_CIPHER_SPEC ==  pic32mzw1TlsConf->tlsLastRecordType)
        {
            pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_FINISHED;
        }
        else
        {
            switch(tlsRecordHandshakeType)
            {
                case 1:
                    pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_HELLO;
                    break;
                case 11:
                    pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_CERTIFICATE;
                    break;
                case 13:
                    pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_CERTIFICATE_REQUEST;
                    break;
                case 15:
                    pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_CERTIFICATE_VERIFY;
                    break;
                case 16:
                    pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_KEY_EXCHANGE;
                    break;
            /*    case 20:
                    pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_FINISHED;
                    break;
             */ 
                default:
                    pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_UNKNOWN;               
                    break;
            }
        }
    }
    else if(DRV_TLS_RECORD_TYPE_CHANGE_CIPHER_SPEC == contentType)
    {
        pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_CLIENT_CHANGE_CIPHER_SPEC;
    }
    else if(DRV_TLS_RECORD_TYPE_ALERT_PROTOCOL == contentType)
    {
        pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_ALERT;
    }
    else if(DRV_TLS_RECORD_TYPE_APPLICATION_DATA == contentType)
    {
        pTlsRecordHdr->tlsRecordType = DRV_PIC32MZW1_TLS_RECORD_TYPE_APPLICATION_DATA;
    }
	pic32mzw1TlsConf->tlsLastRecordType = pTlsRecordHdr->tlsRecordType;
	return true;
}


static int DRV_TLS_RecvCallback(WOLFSSL *pSSLCtx, char *pBuf, int buffSize, void *pAppCtx)
{
    DRV_PIC32MZW1_TLS_PKT *pTlsPkt = NULL;
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)pAppCtx;
    uint16_t pendReadTlsBuffLen = 0;
    uint16_t readTlsBuffLen = 0;
    uint16_t reqBufferSize = buffSize;
    
    if ((NULL == pSSLCtx) || (NULL == pBuf) || (NULL == pic32mzw1TlsConf) || (0 == buffSize))
    {
        return WOLFSSL_CBIO_ERR_WANT_READ;
    }

    if (TCPIP_Helper_ProtectedSingleListCount(&pic32mzw1TlsConf->tlsRxQueue) > 0)
    {
        pendReadTlsBuffLen = reqBufferSize;
        while(0 != pendReadTlsBuffLen)
        {
            if (TCPIP_Helper_ProtectedSingleListCount(&pic32mzw1TlsConf->tlsRxQueue) > 0)
            {
                pTlsPkt = (DRV_PIC32MZW1_TLS_PKT*)TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzw1TlsConf->tlsRxQueue);
                if (NULL == pTlsPkt)
                {
                    /* Failed to get data from tlsRxQueue */
                    return WOLFSSL_CBIO_ERR_GENERAL;
                }
                else
                {
                    uint16_t buffSizeDiff = (reqBufferSize - readTlsBuffLen);
                    if (((pTlsPkt->bufferSize - pTlsPkt->offsetRead)) <= buffSizeDiff)
                    {
                        memcpy((pBuf + readTlsBuffLen), (pTlsPkt->data + pTlsPkt->offsetRead), (pTlsPkt->bufferSize - pTlsPkt->offsetRead));
                        readTlsBuffLen += (pTlsPkt->bufferSize - pTlsPkt->offsetRead);
                        pendReadTlsBuffLen -= (pTlsPkt->bufferSize - pTlsPkt->offsetRead);
                        //Delete the packet after copy
                        OSAL_Free(pTlsPkt);
                    }
                    else
                    {
                        memcpy((pBuf + readTlsBuffLen), (pTlsPkt->data + pTlsPkt->offsetRead), buffSizeDiff);
                        pTlsPkt->offsetRead += buffSizeDiff;
                        readTlsBuffLen += buffSizeDiff;
                        pendReadTlsBuffLen -= buffSizeDiff;
                        /* Add the packet back to the queue at head position */
                        TCPIP_Helper_ProtectedSingleListHeadAdd(&pic32mzw1TlsConf->tlsRxQueue, (SGL_LIST_NODE*)pTlsPkt);
                    }
                }
            }
            else
            {
                /* only partial data read. 
                   wolfssl will call this function again to read pending data so now return what is available */
                return readTlsBuffLen;
            }
        }
        /* Data read full */
        return readTlsBuffLen;
    }
    /* No packet in queue */
    return WOLFSSL_CBIO_ERR_WANT_READ;
}

static int DRV_TLS_SendCallback(WOLFSSL *pSSLCtx, char *pBuf, int buffSize, void *pAppCtx)
{
    DRV_PIC32MZW1_TLS_PKT *pTlsPkt = NULL;
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)pAppCtx;
    DRV_PIC32MZW1_TLS_RECORD_HDR tlsRecordHdr;
    
    if ((NULL == pSSLCtx) || (NULL == pBuf) || (NULL == pic32mzw1TlsConf) || (0 == buffSize))
    {
        return WOLFSSL_CBIO_ERR_WANT_WRITE;
    }
    
    pTlsPkt = (DRV_PIC32MZW1_TLS_PKT *)OSAL_Malloc(sizeof(DRV_PIC32MZW1_TLS_PKT) + (sizeof(uint8_t) * buffSize));
    if(NULL == pTlsPkt)
    {
        return WOLFSSL_CBIO_ERR_GENERAL;
    }
    pTlsPkt->bufferSize = 0;
    pTlsPkt->offsetRead = 0;
    
    /* copy the data to the DRV_PIC32MZW1_TLS_PKT structure */
    memcpy(pTlsPkt->data, pBuf, buffSize);
    pTlsPkt->bufferSize = buffSize;
    
    if (false == DRV_TLS_GetRecordHdr(&tlsRecordHdr, pTlsPkt->data, pTlsPkt->bufferSize, pic32mzw1TlsConf))
    {
        /* Failed to get the record header - discard the buffer */
        //Delete the packet after copy
        OSAL_Free(pTlsPkt);
        return WOLFSSL_CBIO_ERR_GENERAL;
    }

    /* Add the packet to Tx queue and inform the upper layer */
    TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzw1TlsConf->tlsTxQueue, (SGL_LIST_NODE*)pTlsPkt);
	
    if (NULL != pic32mzw1TlsConf->fpTlsEventCb)
    {
        /* Notify the application packet availability in Tx queue */
        pic32mzw1TlsConf->fpTlsEventCb(DRV_PIC32MZW1_TLS_EVENT_TX_PKT_READY, &tlsRecordHdr, pic32mzw1TlsConf->tlsEventCbArg);
    }

    return buffSize;
}
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */

DRV_PIC32MZW1_TLS_HANDLE DRV_PIC32MZW1_TLS_Init
(
    uintptr_t tlsCtxHandle,
    const char *const pServerDomain   
)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT  
    int serverDomainLen = 0;
    WOLFSSL_CTX *pTlsCtx = (WOLFSSL_CTX *) tlsCtxHandle;
    if (NULL == pTlsCtx)
    {
        return DRV_PIC32MZW1_TLS_HANDLE_INVALID;
    }   
    if (NULL != pServerDomain)
    {
        serverDomainLen = strlen(pServerDomain);
    }
    
    g_pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *) OSAL_Malloc(sizeof(DRV_PIC32MZW1_WOLFSSL_TLS_CONF));
    if (NULL == g_pic32mzw1TlsConf)
    {
        return DRV_PIC32MZW1_TLS_HANDLE_INVALID;
    }

    /* Initialize g_pic32mzw1TlsConf */
    g_pic32mzw1TlsConf->tlsWolfsslCtx = pTlsCtx;
    g_pic32mzw1TlsConf->tlsWolfsslSess = NULL;
      
    /* Set server domain for validation of server cert's SAN(subject alternative name) or CN(common name) */
    memset(&g_pic32mzw1TlsConf->serverDomainName, 0, WDRV_PIC32MZW_ENT_AUTH_SERVER_DOMAIN_LEN_MAX+1);
    if(NULL != pServerDomain)
    {
        memcpy(&g_pic32mzw1TlsConf->serverDomainName, pServerDomain, serverDomainLen);
    }

    return (DRV_PIC32MZW1_TLS_HANDLE) g_pic32mzw1TlsConf;
#else
	return DRV_PIC32MZW1_TLS_HANDLE_INVALID;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}


bool DRV_PIC32MZW1_TLS_DeInit
(
    DRV_PIC32MZW1_TLS_HANDLE tlsHandle
)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)tlsHandle;
    
    if ((DRV_PIC32MZW1_TLS_HANDLE_INVALID == tlsHandle) || (NULL == pic32mzw1TlsConf))
    {
        return false;
    }
    
    /* Check if TLS session is active */
    if (NULL != pic32mzw1TlsConf->tlsWolfsslSess)
    {
        return false;
    }
    
    OSAL_Free(pic32mzw1TlsConf);
    pic32mzw1TlsConf = NULL;
    return true;
#else
	return false;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}


DRV_PIC32MZW1_TLS_SESSION_HANDLE DRV_PIC32MZW1_TLS_CreateSession
(
    DRV_PIC32MZW1_TLS_EVENT_CB	fpSessionCb,
    uint32_t userArg
)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
    WOLFSSL *pTlsSess = NULL;
    if ((NULL == g_pic32mzw1TlsConf) || (NULL == fpSessionCb))
    {
        return DRV_PIC32MZW1_TLS_SESSION_HANDLE_INVALID;
    }
    
    /* Check if TLS context is set by the driver */
    if (NULL == g_pic32mzw1TlsConf->tlsWolfsslCtx)
    {
        return DRV_PIC32MZW1_TLS_SESSION_HANDLE_INVALID;
    }
    
    /* Initialize TLS session */
    g_pic32mzw1TlsConf->tlsWolfsslSess = NULL;
    
    /* Create TLS session for the Init wolfssl context */
    pTlsSess = wolfSSL_new(g_pic32mzw1TlsConf->tlsWolfsslCtx);
    if (NULL == pTlsSess)
    {
        return DRV_PIC32MZW1_TLS_SESSION_HANDLE_INVALID;
    }
     
    /* Initialize Rx/Tx queues */
    TCPIP_Helper_ProtectedSingleListInitialize(&g_pic32mzw1TlsConf->tlsTxQueue);
    TCPIP_Helper_ProtectedSingleListInitialize(&g_pic32mzw1TlsConf->tlsRxQueue);
    
    /* Set custom I/O callbacks for Tx/Rx operations of wolfssl stack */
    wolfSSL_SSLSetIORecv(pTlsSess, &DRV_TLS_RecvCallback);
    wolfSSL_SSLSetIOSend(pTlsSess, &DRV_TLS_SendCallback);
    
    /* Set callback for handshake complete event */
    wolfSSL_SetHsDoneCb(pTlsSess, DRV_TLS_HandshakeDoneCallback, g_pic32mzw1TlsConf);
    
    wolfSSL_SetIOReadCtx(pTlsSess, g_pic32mzw1TlsConf);
    wolfSSL_SetIOWriteCtx(pTlsSess, g_pic32mzw1TlsConf);
    
    /* Prevent wolfSSL from freeing temporary arrays at the end of handshake 
     The temporary arrays are required for derivation of keying material for EAP-TLS later */
    wolfSSL_KeepArrays(pTlsSess);
   
    /* Enable server domain check */
    if ('\0' != g_pic32mzw1TlsConf->serverDomainName[0])
    {
        if (SSL_SUCCESS != wolfSSL_check_domain_name(pTlsSess, g_pic32mzw1TlsConf->serverDomainName))
        {
            return DRV_PIC32MZW1_TLS_SESSION_HANDLE_INVALID;
        }
    }

    /* store tls session info */
    g_pic32mzw1TlsConf->tlsWolfsslSess = pTlsSess;
    g_pic32mzw1TlsConf->fpTlsEventCb = fpSessionCb;
    g_pic32mzw1TlsConf->tlsEventCbArg = userArg;
    
    return (DRV_PIC32MZW1_TLS_SESSION_HANDLE) g_pic32mzw1TlsConf;
#else
    return DRV_PIC32MZW1_TLS_SESSION_HANDLE_INVALID;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}

bool DRV_PIC32MZW1_TLS_StartSession(DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)tlsSessHandle;
    int result;
    
    if ((NULL == pic32mzw1TlsConf) || (NULL == pic32mzw1TlsConf->tlsWolfsslSess))
    {
        return false;
    }
    
    result = wolfSSL_connect(pic32mzw1TlsConf->tlsWolfsslSess);
    if (SSL_SUCCESS != result)
    {
        int error = wolfSSL_get_error(pic32mzw1TlsConf->tlsWolfsslSess, result);
        /* Ignore I/O error */
        if (!((SSL_ERROR_WANT_READ == error) || (SSL_ERROR_WANT_WRITE == error)))
        {
            if (NULL != pic32mzw1TlsConf->fpTlsEventCb)
            {
                /* Failed to connect to the remote server */
                pic32mzw1TlsConf->fpTlsEventCb(DRV_PIC32MZW1_TLS_EVENT_SESSION_TERMINATED, NULL, pic32mzw1TlsConf->tlsEventCbArg);
                return false;
            }
        }
    }
    
    return true;
#else
	return false;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}

bool DRV_PIC32MZW1_TLS_TerminateSession(DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)tlsSessHandle;
    if (NULL == pic32mzw1TlsConf)
    {
        return false;
    }
    
    /* Check if TLS session was created */
    if (NULL == pic32mzw1TlsConf->tlsWolfsslSess)
    {
        return false;
    }
    
    /* Reset custom I/O callbacks for Tx/Rx operations of wolfssl stack */
    wolfSSL_SSLSetIORecv(pic32mzw1TlsConf->tlsWolfsslSess, NULL);
    wolfSSL_SSLSetIOSend(pic32mzw1TlsConf->tlsWolfsslSess, NULL);
    
    wolfSSL_SetIOReadCtx(pic32mzw1TlsConf->tlsWolfsslSess, NULL);
    wolfSSL_SetIOWriteCtx(pic32mzw1TlsConf->tlsWolfsslSess, NULL);
    
    /* wolfssl shutdown */
    wolfSSL_shutdown(pic32mzw1TlsConf->tlsWolfsslSess);
    
    /* Don't need temporary arrays anymore.Free it */
    wolfSSL_FreeArrays(pic32mzw1TlsConf->tlsWolfsslSess);
            
    /* Delete session */
    wolfSSL_free(pic32mzw1TlsConf->tlsWolfsslSess);
    pic32mzw1TlsConf->tlsWolfsslSess = NULL;
    
    pic32mzw1TlsConf->fpTlsEventCb = NULL;

    /* De-initialize Rx/Tx queues - deletes all the nodes */
    TCPIP_Helper_ProtectedSingleListDeinitialize(&pic32mzw1TlsConf->tlsTxQueue);
    TCPIP_Helper_ProtectedSingleListDeinitialize(&pic32mzw1TlsConf->tlsRxQueue);
    
    return true;
#else
	return false;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}

bool DRV_PIC32MZW1_TLS_ReadTxBuffer
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint16_t reqBufferSize,
    uint8_t	**pDataBuff
)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)tlsSessHandle;
    DRV_PIC32MZW1_TLS_PKT *pTlsPkt = NULL;
    uint8_t *pBuf = NULL;
    uint16_t pendReadTlsBuffLen = 0;
    uint16_t readTlsBuffLen = 0;
    
    if ((NULL == pic32mzw1TlsConf) || (NULL == pic32mzw1TlsConf->tlsWolfsslSess) || (NULL == pDataBuff))
    {
        return false;
    }
    
    /* If the data is read and the handshake is not complete trigger connect again if wolfssl is blocked to send data */
    if(!wolfSSL_is_init_finished(pic32mzw1TlsConf->tlsWolfsslSess) && wolfSSL_want_write(pic32mzw1TlsConf->tlsWolfsslSess))
    {
        DRV_PIC32MZW1_TLS_StartSession(tlsSessHandle);
    }
    
    /* Check if no packet in queue */
    if (TCPIP_Helper_ProtectedSingleListCount(&pic32mzw1TlsConf->tlsTxQueue) <= 0)
    {
        return false;
    }
    
    /* Allocate memory for total size */
    pBuf = OSAL_Malloc(reqBufferSize);
    if(NULL == pBuf)
    {
        return false;
    }
    
    pendReadTlsBuffLen = reqBufferSize;
    while(0 != pendReadTlsBuffLen)
    {
        if (TCPIP_Helper_ProtectedSingleListCount(&pic32mzw1TlsConf->tlsTxQueue) > 0)
        {
            pTlsPkt = (DRV_PIC32MZW1_TLS_PKT*)TCPIP_Helper_ProtectedSingleListHeadRemove(&pic32mzw1TlsConf->tlsTxQueue);
            if (NULL == pTlsPkt)
            {
                /* Failed to get data from tlsTxQueue */
                break;
            }
            else
            {
                uint16_t buffSizeDiff = (reqBufferSize - readTlsBuffLen);
                if (((pTlsPkt->bufferSize - pTlsPkt->offsetRead)) <= buffSizeDiff)
                {
                    memcpy((pBuf + readTlsBuffLen), (pTlsPkt->data + pTlsPkt->offsetRead), (pTlsPkt->bufferSize - pTlsPkt->offsetRead));
                    readTlsBuffLen += (pTlsPkt->bufferSize - pTlsPkt->offsetRead);
                    pendReadTlsBuffLen -= (pTlsPkt->bufferSize - pTlsPkt->offsetRead);
                    //Delete the TLS packet 
                    OSAL_Free(pTlsPkt);
                }
                else
                {
                    memcpy((pBuf + readTlsBuffLen), (pTlsPkt->data + pTlsPkt->offsetRead), buffSizeDiff);
                    pTlsPkt->offsetRead += buffSizeDiff;
                    readTlsBuffLen += buffSizeDiff;
                    pendReadTlsBuffLen -= buffSizeDiff;
                    /* Add the packet back to the queue at head position */
                    TCPIP_Helper_ProtectedSingleListHeadAdd(&pic32mzw1TlsConf->tlsTxQueue, (SGL_LIST_NODE*)pTlsPkt);
                }
            }
        }
        else
        {
            /* No packet in queue */
            break;
        }
    }
    
    if (0 == pendReadTlsBuffLen)
    {
        /* Data read full so return true */
        memcpy(*pDataBuff, pBuf, reqBufferSize);
        OSAL_Free(pBuf);
        return true;
    }
    else
    {
        OSAL_Free(pBuf);
        return false;
    }
#else
	return false;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}

bool DRV_PIC32MZW1_TLS_WriteRxBuffer
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint16_t bufferSize,
    uint8_t	*pDataBuff
)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)tlsSessHandle;
    DRV_PIC32MZW1_TLS_PKT *pTlsPkt = NULL;
    DRV_PIC32MZW1_TLS_RECORD_HDR tlsRecordHdr;
    
    if ((NULL == pic32mzw1TlsConf) || (NULL == pDataBuff) || (NULL == pic32mzw1TlsConf->tlsWolfsslSess))
    {
        return false;
    }
    pTlsPkt = (DRV_PIC32MZW1_TLS_PKT *)OSAL_Malloc(sizeof(DRV_PIC32MZW1_TLS_PKT) + (sizeof(uint8_t) * bufferSize));
    if (NULL == pTlsPkt)
    {
        return false;
    }

    memcpy(pTlsPkt->data, pDataBuff, bufferSize);
    pTlsPkt->bufferSize = bufferSize;
    pTlsPkt->offsetRead = 0;
    
    /* If the data is available and the handshake is not complete trigger connect again if wolfssl is blocked for data */
    if (!wolfSSL_is_init_finished(pic32mzw1TlsConf->tlsWolfsslSess) && wolfSSL_want_read(pic32mzw1TlsConf->tlsWolfsslSess))
    {
        /* Add the packet to Rx queue */
        TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzw1TlsConf->tlsRxQueue, (SGL_LIST_NODE*)pTlsPkt);
        
        DRV_PIC32MZW1_TLS_StartSession(tlsSessHandle);
    }
    else
    {
        /* If the session is already established let the upper-layer know we have received an Application data 
         * so it can call DRV_PIC32MZW1_TLS_ReadAppData API to receive the decrypted app data packet */
        if (false == DRV_TLS_GetRecordHdr(&tlsRecordHdr, pTlsPkt->data, pTlsPkt->bufferSize, pic32mzw1TlsConf))
        {
            /* Failed to get the record header - discard the buffer */
            OSAL_Free(pTlsPkt);
            return false;
        }
        /* Add the packet to Rx queue */
        TCPIP_Helper_ProtectedSingleListTailAdd(&pic32mzw1TlsConf->tlsRxQueue, (SGL_LIST_NODE*)pTlsPkt);
        
        if (DRV_PIC32MZW1_TLS_RECORD_TYPE_APPLICATION_DATA == tlsRecordHdr.tlsRecordType)
        {
            if (NULL != pic32mzw1TlsConf->fpTlsEventCb)
            {
                /* Notify the upper layer that TLS application data is received */
                pic32mzw1TlsConf->fpTlsEventCb(DRV_PIC32MZW1_TLS_EVENT_RX_APPLICATION_DATA, &tlsRecordHdr, pic32mzw1TlsConf->tlsEventCbArg);
            }
            
        }
    }
    
    return true;
#else
	return false;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}

bool DRV_PIC32MZW1_TLS_GenerateKey
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint8_t *pMsk,
    uint16_t keyLen,
    const char *pLabel        
)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT	
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)tlsSessHandle;  
    if (NULL == pic32mzw1TlsConf)
    {
        return false;
    }
    if ((NULL == pMsk) || (NULL == pLabel) || (0 == keyLen) || (NULL == pic32mzw1TlsConf->tlsWolfsslSess))
    {
        return false;
    }
   
    /* Check if the TLS handshake is complete or not */
    if(!wolfSSL_is_init_finished(pic32mzw1TlsConf->tlsWolfsslSess))
    {
        return false;
    }
    
    if(0 != wolfSSL_make_eap_keys(pic32mzw1TlsConf->tlsWolfsslSess, (void *)pMsk, keyLen, pLabel))
    {
        /* Failed to derive the session key */
        return false;
    }
    return true;
#else
	return false;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}


int32_t DRV_PIC32MZW1_TLS_WriteAppData
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint8_t *pAppData,
    uint16_t AppDataLen      
)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
    int ret = 0;
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)tlsSessHandle;  
    if (NULL == pic32mzw1TlsConf)
    {
        return false;
    }
    if ((NULL == pAppData) || (0 == AppDataLen) || (NULL == pic32mzw1TlsConf->tlsWolfsslSess))
    {
        return false;
    }
   
    /* Check if the TLS handshake is complete or not */
    if(!wolfSSL_is_init_finished(pic32mzw1TlsConf->tlsWolfsslSess))
    {
        return false;
    }
    
    ret = wolfSSL_write(pic32mzw1TlsConf->tlsWolfsslSess, pAppData, AppDataLen);
    if (ret < 0)
    {
        int error = wolfSSL_get_error(pic32mzw1TlsConf->tlsWolfsslSess, ret);
        if ((error == SSL_ERROR_WANT_READ) ||
            (error == SSL_ERROR_WANT_WRITE))
        {
            return 0;
        }        
    }    
    return ret;
#else
	return -1;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}

int32_t DRV_PIC32MZW1_TLS_ReadAppData
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint8_t *pAppData,
    uint16_t AppDataLen      
)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
    int ret = 0;
    DRV_PIC32MZW1_WOLFSSL_TLS_CONF *pic32mzw1TlsConf = (DRV_PIC32MZW1_WOLFSSL_TLS_CONF *)tlsSessHandle;  
    if (NULL == pic32mzw1TlsConf)
    {
        return false;
    }
    if ((NULL == pAppData) || (0 == AppDataLen) || (NULL == pic32mzw1TlsConf->tlsWolfsslSess))
    {
        return false;
    }
   
    /* Check if the TLS handshake is complete or not */
    if(!wolfSSL_is_init_finished(pic32mzw1TlsConf->tlsWolfsslSess))
    {
        return false;
    }
    
    ret = wolfSSL_read(pic32mzw1TlsConf->tlsWolfsslSess, pAppData, AppDataLen);
    if (ret < 0)
    {
        int error = wolfSSL_get_error(pic32mzw1TlsConf->tlsWolfsslSess, ret);
        if ((error == SSL_ERROR_WANT_READ) ||
            (error == SSL_ERROR_WANT_WRITE))
        {
            return 0;
        }
    }    
    return ret;
#else
	return -1;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}
