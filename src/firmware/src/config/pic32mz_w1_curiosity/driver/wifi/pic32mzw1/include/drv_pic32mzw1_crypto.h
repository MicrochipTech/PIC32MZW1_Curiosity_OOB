/*******************************************************************************
Copyright (c) 2019 released Microchip Technology Inc. All rights reserved.

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

#ifndef _DRV_PIC32MZW1_CRYPTO_H
#define _DRV_PIC32MZW1_CRYPTO_H

#include <stdint.h>
#include <stdbool.h>

#define DRV_PIC32MZW1_CRYPTO_SHA256_DIGEST_SZ   32

/* This Enum contains the various finite cyclic groups available to the WLAN */
/* library. They are all ECC groups, defined in wdrv_pic32mzw_crypto.c. The  */
/* values used in this Enum should match the IANA definitions at:            */
/* https://www.iana.org/assignments/ipsec-registry/ipsec-registry.xhtml#ipsec-registry-10 */
typedef enum {
    DRV_PIC32MZW1_CRYPTO_FCG_NONE       = 0,
    DRV_PIC32MZW1_CRYPTO_FCG_CURVE_P256 = 19
} DRV_PIC32MZW1_CRYPTO_FCG_ID_T;

typedef enum
{
    DRV_PIC32MZW1_CRYPT_HMAC_SHA    = 1, 
    DRV_PIC32MZW1_CRYPT_HMAC_SHA256 = 2, 
    DRV_PIC32MZW1_CRYPT_HMAC_SHA384 = 5, 
    DRV_PIC32MZW1_CRYPT_HMAC_SHA512 = 4 
} DRV_PIC32MZW1_CRYPT_HMAC_HASH_T;

typedef enum {
    /* Success. */
    DRV_PIC32MZW1_CRYPTO_SUCCESS,
    /* Failure. */
    DRV_PIC32MZW1_CRYPTO_FAIL,
    /* Unavailable. */
    DRV_PIC32MZW1_CRYPTO_BUSY,
} DRV_PIC32MZW1_CRYPTO_STATUS_T;

/* Generic type for handling data buffers. */
typedef struct
{
    const uint8_t   *data;
    uint16_t        data_len;
} buffer_t;

/* Callback function type. */
typedef void (*DRV_PIC32MZW1_CRYPTO_CB)(DRV_PIC32MZW1_CRYPTO_STATUS_T result, uintptr_t context);

/*****************************************************************************/
/* Random functions:   DRV_PIC32MZW1_Crypto_Random                           */
/*****************************************************************************/
/* out = random array of length param_len.                                   */
/* out is only valid if return is true.                                      */
bool DRV_PIC32MZW1_Crypto_Random
(
        uint8_t         *out,
        uint16_t        param_len
);

/*****************************************************************************/
/* HMAC functions.          DRV_PIC32MZW1_Crypto_HMAC                        */
/*                                                                           */
/* The in/out arrays do not need to be distinct.                             */
/*****************************************************************************/
/* Run an HMAC-hash operation. */
bool DRV_PIC32MZW1_Crypto_HMAC
(
        const uint8_t                       *salt,
        uint16_t                            salt_len,
        const buffer_t                      *input_data_buffers,
        int                                 num_buffers,
        uint8_t                             *digest,
        DRV_PIC32MZW1_CRYPT_HMAC_HASH_T     type
);

/*****************************************************************************/
/* Big integer functions:   DRV_PIC32MZW1_Crypto_BigIntModAdd                */
/*                          DRV_PIC32MZW1_Crypto_BigIntModSubtract           */
/*                          DRV_PIC32MZW1_Crypto_BigIntModMultiply           */
/*                          DRV_PIC32MZW1_Crypto_BigIntModExponentiate       */
/*                          DRV_PIC32MZW1_Crypto_BigIntMod                   */
/*                                                                           */
/* Outputs are only valid if return is true.                                 */
/* Parameter param_len applies to all in/out arrays, with the exception of   */
/* the ain parameter of BigIntMod.                                           */
/* Parameter is_be (big endian) applies to all in/out arrays.                */
/* The in/out arrays do not need to be distinct (e.g. you can add in-place). */
/*****************************************************************************/
/* out = ain + bin. */
bool DRV_PIC32MZW1_Crypto_BigIntModAdd
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *ain,
        const uint8_t           *bin,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW1_CRYPTO_CB callback,
        uintptr_t               context
);
/* out = dimin - subin. */
bool DRV_PIC32MZW1_Crypto_BigIntModSubtract
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *dimin,
        const uint8_t           *subin,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW1_CRYPTO_CB callback,
        uintptr_t               context
);
/* out = ain * bin. */
bool DRV_PIC32MZW1_Crypto_BigIntModMultiply
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *ain,
        const uint8_t           *bin,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW1_CRYPTO_CB callback,
        uintptr_t               context
);
/* out = basein ^ expin. */
bool DRV_PIC32MZW1_Crypto_BigIntModExponentiate
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *basein,
        const uint8_t           *expin,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW1_CRYPTO_CB callback,
        uintptr_t               context
);
/* out = ain (modular reduction). */
bool DRV_PIC32MZW1_Crypto_BigIntMod
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *ain,
        uint16_t                ain_len,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW1_CRYPTO_CB callback,
        uintptr_t               context
);

/*****************************************************************************/
/* Elliptic curve functions.    DRV_PIC32MZW1_Crypto_ECCGetField             */
/*                              DRV_PIC32MZW1_Crypto_ECCGetOrder             */
/*                              DRV_PIC32MZW1_Crypto_ECCIsOnCurve            */
/*                              DRV_PIC32MZW1_Crypto_ECCBigIntModMultByA     */
/*                              DRV_PIC32MZW1_Crypto_ECCBigIntModAddB        */
/*                              DRV_PIC32MZW1_Crypto_ECCAdd                  */
/*                              DRV_PIC32MZW1_Crypto_ECCMultiply             */
/*                                                                           */
/* With the exception of ECCGetField and ECCGetOrder, outputs are only valid */
/* if the return is true.                                                    */
/* Additionally, for ECCAdd and ECCMultiply, the output is_infinity or       */
/* is_notoncurve should be checked in order to determine whether the other   */
/* outputs are valid.                                                        */
/* ECCIsOnCurve, ECCAdd and ECCMultiply can take either big or little endian */
/* parameters; the is_be parameter applies to all in/out arrays.             */
/* The other APIs require little endian parameters.                          */
/* The in/out arrays do not need to be distinct.                             */
/*****************************************************************************/
/* out is a pointer to the field of the curve. */
const uint8_t* DRV_PIC32MZW1_Crypto_ECCGetField
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id
);
/* out is a pointer to the order of the curve. */
const uint8_t* DRV_PIC32MZW1_Crypto_ECCGetOrder
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id
);
/* is_notoncurve = false if p(x,y) is on curve, true otherwise. */
bool DRV_PIC32MZW1_Crypto_ECCIsOnCurve
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            *is_notoncurve,
        const uint8_t                   *px,
        const uint8_t                   *py,
        bool                            is_be,
        DRV_PIC32MZW1_CRYPTO_CB         callback,
        uintptr_t                       context
);
/* out = a*in, with 'a' and modulo appropriate for curve_id */
/* Params must be little endian, of size equal to the curve's field size. */
bool DRV_PIC32MZW1_Crypto_ECCBigIntModMultByA
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        uint8_t                         *out,
        const uint8_t                   *in,
        DRV_PIC32MZW1_CRYPTO_CB         callback,
        uintptr_t                       context
);
/* out = in+b, with 'b' and modulo appropriate for curve_id */
/* Params must be little endian, of size equal to the curve's field size. */
bool DRV_PIC32MZW1_Crypto_ECCBigIntModAddB
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        uint8_t                         *out,
        const uint8_t                   *in,
        DRV_PIC32MZW1_CRYPTO_CB         callback,
        uintptr_t                       context
);
/* out(x,y) = elem-op(Pin(x,y), Qin(x,y)). If the result is the point at     */
/* infinity, is_infinity = true, and out(x,y) should be ignored.             */
bool DRV_PIC32MZW1_Crypto_ECCAdd
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            *is_infinity,
        uint8_t                         *outx,
        uint8_t                         *outy,
        const uint8_t                   *pinx,
        const uint8_t                   *piny,
        const uint8_t                   *qinx,
        const uint8_t                   *qiny,
        bool                            is_be,
        DRV_PIC32MZW1_CRYPTO_CB         callback,
        uintptr_t                       context
);
/* out(x,y) = scalar-op(kin, pin(x,y)). If the result is the point at        */
/* infinity, is_infinity = true, and out(x,y) should be ignored.             */
bool DRV_PIC32MZW1_Crypto_ECCMultiply
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            *is_infinity,
        uint8_t                         *outx,
        uint8_t                         *outy,
        const uint8_t                   *pinx,
        const uint8_t                   *piny,
        const uint8_t                   *kin,
        bool                            is_be,
        DRV_PIC32MZW1_CRYPTO_CB         callback,
        uintptr_t                       context
);

#endif /* _DRV_PIC32MZW1_CRYPTO_H */