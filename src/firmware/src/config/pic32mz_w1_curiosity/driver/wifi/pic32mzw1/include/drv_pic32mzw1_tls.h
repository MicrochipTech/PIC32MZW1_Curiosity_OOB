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
    DRV_PIC32MZW1_TLS_EVENT_SESSION_ESTABLISHED,
    /*
     There is a TLS application data received. The upper layer is responsible for calling
    the corresponding receive API for receiving the decrypted TLS application data packet.
     */
    DRV_PIC32MZW1_TLS_EVENT_RX_APPLICATION_DATA
}DRV_PIC32MZW1_TLS_EVENT;

/* Callback function type through which TLS events will be notified */
typedef void (*DRV_PIC32MZW1_TLS_EVENT_CB)
(
    DRV_PIC32MZW1_TLS_EVENT tlsEvent,
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
    DRV_PIC32MZW1_TLS_RECORD_TYPE_ALERT,
    DRV_PIC32MZW1_TLS_RECORD_TYPE_APPLICATION_DATA
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
    DRV_PIC32MZW1_TLS_EVENT_CB  fpSessionCb,
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
    uint8_t **pDataBuff
);

/* Write to Wolfssl TLS Rx buffer queue */
bool DRV_PIC32MZW1_TLS_WriteRxBuffer
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint16_t bufferSize,
    uint8_t *pDataBuff
);

/* Derive keying material required for EAP-TLS and EAP-TTLS */
bool DRV_PIC32MZW1_TLS_GenerateKey
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint8_t *pMsk,
    uint16_t keyLen,
    const char *pLabel
);

/* Write Application data to encrypt and send to server */
int32_t DRV_PIC32MZW1_TLS_WriteAppData
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint8_t *pAppData,
    uint16_t AppDataLen
);

/* Read Application data - decrypted and received from server */
int32_t DRV_PIC32MZW1_TLS_ReadAppData
(
    DRV_PIC32MZW1_TLS_SESSION_HANDLE tlsSessHandle,
    uint8_t *pAppData,
    uint16_t AppDataLen
);

#endif /* _DRV_PIC32MZW1_TLS_H */
