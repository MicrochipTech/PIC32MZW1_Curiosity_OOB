/*******************************************************************************
Copyright (c) 2022 released Microchip Technology Inc. All rights reserved.

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

#ifndef _DRV_PIC32MZW1_TLS_H
#define _DRV_PIC32MZW1_TLS_H

/* Driver TLS module handle */
typedef uintptr_t DRV_PIC32MZW1_TLS_HANDLE;

/* Invalid TLS module handle */
#define DRV_PIC32MZW1_TLS_HANDLE_INVALID  (((DRV_PIC32MZW1_TLS_HANDLE) -1))

/* Driver TLS session handle */
typedef uintptr_t DRV_PIC32MZW1_TLS_SESSION_HANDLE;

/* Invalid TLS session handle */
#define DRV_PIC32MZW1_TLS_SESSION_HANDLE_INVALID  (((DRV_PIC32MZW1_TLS_SESSION_HANDLE) -1))

/* TLS event type */
typedef enum
{			
    /* 
	There is a TLS packet ready for transmission. The upper layer is responsible for calling 
	the corresponding transport API for sending the TLS packets to the other TLS party.
    */
    DRV_PIC32MZW1_TLS_EVENT_TX_PKT_READY,
    /*
	TLS session is terminated either locally or by the remote peer.
    */ 
    DRV_PIC32MZW1_TLS_EVENT_SESSION_TERMINATED,
    /*
     TLS session is successfully established with the peer and the handshake is complete.
    */
    DRV_PIC32MZW1_TLS_EVENT_SESSION_ESTABLISHED  
}DRV_PIC32MZW1_TLS_EVENT;

/* Callback function type through which TLS events will be notified */
typedef void (*DRV_PIC32MZW1_TLS_EVENT_CB)
(
    DRV_PIC32MZW1_TLS_EVENT	tlsEvent,
    void  *pvEventData,
    uint32_t userData
);

/* TLS record types */
typedef enum
{
    DRV_PIC32MZW1_TLS_RECORD_TYPE_UNKNOWN = 0,
    DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_HELLO,
    DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_CERTIFICATE, 
    DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_CERTIFICATE_REQUEST,
    DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_CERTIFICATE_VERIFY,
    DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_KEY_EXCHANGE,       
    DRV_PIC32MZW1_TLS_RECORD_TYPE_HANDSHAKE_CLIENT_FINISHED,
    DRV_PIC32MZW1_TLS_RECORD_TYPE_CLIENT_CHANGE_CIPHER_SPEC,
    DRV_PIC32MZW1_TLS_RECORD_TYPE_ALERT        
} DRV_PIC32MZW1_TLS_CLIENT_RECORD_TYPE;

/* TLS record header */
typedef struct
{
    /* TLS record type */
    DRV_PIC32MZW1_TLS_CLIENT_RECORD_TYPE tlsRecordType;
    /* TLS record version */
    uint16_t tlsVersion;
    /* TLS record length including header */
    uint16_t tlsRecordLength;
} DRV_PIC32MZW1_TLS_RECORD_HDR;

/* Initialize driver TLS module with the wolfssl context handle passed */
DRV_PIC32MZW1_TLS_HANDLE DRV_PIC32MZW1_TLS_Init
(
    uintptr_t tlsCtxHandle,
    const char *const pServerDomain
);

/* De-initialize driver TLS module */
bool DRV_PIC32MZW1_TLS_DeInit
(
    DRV_PIC32MZW1_TLS_HANDLE tlsHandle
);

/* Creates Wolfssl TLS session */
DRV_PIC32MZW1_TLS_SESSION_HANDLE DRV_PIC32MZW1_TLS_CreateSession
(
    DRV_PIC32MZW1_TLS_EVENT_CB	fpSessionCb,
    uint32_t userArg
);

/* Start TLS handshake */
bool DRV_PIC32MZW1_TLS_StartSession(DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle);

/* Terminate Wolfssl TLS session */
bool DRV_PIC32MZW1_TLS_TerminateSession(DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle);

/* Read from Wolfssl TLS Tx buffer queue */
bool DRV_PIC32MZW1_TLS_ReadTxBuffer
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint16_t reqBufferSize,
    uint8_t	**pDataBuff
);

/* Write to Wolfssl TLS Rx buffer queue */
bool DRV_PIC32MZW1_TLS_WriteRxBuffer
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint16_t bufferSize,
    uint8_t	*pDataBuff
);

/* Derive keying material required for EAP-TLS */
bool DRV_PIC32MZW1_TLS_DeriveSessionKey
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint8_t *pMskEmsk,
    uint16_t keyLen,
    const char *pLabel        
);

#endif /* _DRV_PIC32MZW1_TLS_H */
