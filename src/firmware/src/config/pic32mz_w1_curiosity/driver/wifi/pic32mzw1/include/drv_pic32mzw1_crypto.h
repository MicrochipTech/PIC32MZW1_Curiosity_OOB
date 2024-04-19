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

#ifndef _DRV_PIC32MZW1_CRYPTO_H
#define _DRV_PIC32MZW1_CRYPTO_H

#include <stdint.h>
#include <stdbool.h>

#define DRV_PIC32MZW_CRYPTO_SHA256_DIGEST_SZ    32

/* This Enum contains the various finite cyclic groups available to the WLAN */
/* library. They are all ECC groups, defined in wdrv_pic32mzw_crypto.c. The  */
/* values used in this Enum should match the IANA definitions at:            */
/* https://www.iana.org/assignments/ipsec-registry/ipsec-registry.xhtml#ipsec-registry-10 */
typedef enum {
    DRV_PIC32MZW_CRYPTO_FCG_NONE        = 0,
    DRV_PIC32MZW_CRYPTO_FCG_CURVE_P256  = 19,
    DRV_PIC32MZW_CRYPTO_FCG_MAX         = 0xFFFF
} DRV_PIC32MZW_CRYPTO_FCG_ID_T;

typedef enum {
    DRV_PIC32MZW_CRYPTO_NO_HASH = 0,
    DRV_PIC32MZW_CRYPTO_MD4     = 1,        
    DRV_PIC32MZW_CRYPTO_MD5     = 2,        
    DRV_PIC32MZW_CRYPTO_SHA     = 3,
    DRV_PIC32MZW_CRYPTO_SHA256  = 4,
    DRV_PIC32MZW_CRYPTO_SHA224  = 5,
    DRV_PIC32MZW_CRYPTO_SHA512  = 6,
    DRV_PIC32MZW_CRYPTO_SHA384  = 7        
} DRV_PIC32MZW_CRYPTO_HASH_T;

typedef enum {
    DRV_PIC32MZW_CRYPTO_COMPLETE    = 0,
    DRV_PIC32MZW_CRYPTO_PENDING,
    DRV_PIC32MZW_CRYPTO_BUSY,
    DRV_PIC32MZW_CRYPTO_INVALID_PARAM,
    DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR
} DRV_PIC32MZW_CRYPTO_RETURN_T;

/* Generic type for handling data buffers. */
typedef struct
{
    const uint8_t   *data;
    uint16_t        data_len;
} buffer_t;

/* Callback function type. */
typedef void (*DRV_PIC32MZW_CRYPTO_CB)(DRV_PIC32MZW_CRYPTO_RETURN_T result, uintptr_t context);

/*****************************************************************************/
/* Random functions:   DRV_PIC32MZW_Crypto_Random                            */
/*****************************************************************************/
/* out = random array of length param_len.                                   */
/* out is only valid if return is true.                                      */
bool DRV_PIC32MZW_Crypto_Random
(
        uint8_t         *out,
        uint16_t        param_len
);

/*****************************************************************************/
/* Hash functions:      DRV_PIC32MZW_Crypto_Hash                             */
/*                      DRV_PIC32MZW_Crypto_Hash_GetDigestSize               */
/*                                                                           */
/* The in/out arrays do not need to be distinct.                             */
/*****************************************************************************/
/* Run an entire hash algorithm. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_Hash
(
        const buffer_t              *input_data_buffers,
        int                         num_buffers,
        uint8_t                     *digest,
        DRV_PIC32MZW_CRYPTO_HASH_T  type
);

/* Get digest size corresponding to the hash type */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_Hash_GetDigestSize
(
    int32_t                     *digest_size,
    DRV_PIC32MZW_CRYPTO_HASH_T  hashType
);

/* Encrypt the input text using Des encryption with Electronic Codebook (ECB) mode*/
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_DES_Ecb_Blk_Crypt
(
    const uint8_t *pu8PlainText,
    uint16_t plainTextLen,
    const uint8_t *pu8CipherKey,    
    uint8_t *pu8CipherText    
);

/*****************************************************************************/
/* HMAC functions.          DRV_PIC32MZW_Crypto_HMAC                         */
/*                                                                           */
/* The in/out arrays do not need to be distinct.                             */
/*****************************************************************************/
/* Run an HMAC-hash operation. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_HMAC
(
        const uint8_t               *salt,
        uint16_t                    salt_len,
        const buffer_t              *input_data_buffers,
        int                         num_buffers,
        uint8_t                     *digest,
        DRV_PIC32MZW_CRYPTO_HASH_T  type
);

/*****************************************************************************/
/* Big integer functions:   DRV_PIC32MZW_Crypto_BigIntModAdd                 */
/*                          DRV_PIC32MZW_Crypto_BigIntModSubtract            */
/*                          DRV_PIC32MZW_Crypto_BigIntModMultiply            */
/*                          DRV_PIC32MZW_Crypto_BigIntModExponentiate        */
/*                          DRV_PIC32MZW_Crypto_BigIntMod                    */
/*                                                                           */
/* Outputs are only valid if return is DRV_PIC32MZW_CRYPTO_COMPLETE.         */
/* Parameter param_len applies to all in/out arrays, with the exception of   */
/* the ain parameter of BigIntMod.                                           */
/* Parameter is_be (big endian) applies to all in/out arrays.                */
/* The in/out arrays do not need to be distinct (e.g. you can add in-place). */
/*****************************************************************************/
/* out = ain + bin. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_BigIntModAdd
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *ain,
        const uint8_t           *bin,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW_CRYPTO_CB  callback,
        uintptr_t               context
);
/* out = dimin - subin. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_BigIntModSubtract
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *dimin,
        const uint8_t           *subin,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW_CRYPTO_CB  callback,
        uintptr_t               context
);
/* out = ain * bin. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_BigIntModMultiply
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *ain,
        const uint8_t           *bin,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW_CRYPTO_CB  callback,
        uintptr_t               context
);
/* out = basein ^ expin. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_BigIntModExponentiate
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *basein,
        const uint8_t           *expin,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW_CRYPTO_CB  callback,
        uintptr_t               context
);
/* out = ain (modular reduction). */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_BigIntMod
(
        const uint8_t           *mod,
        uint8_t                 *out,
        const uint8_t           *ain,
        uint16_t                ain_len,
        uint16_t                param_len,
        bool                    is_be,
        DRV_PIC32MZW_CRYPTO_CB  callback,
        uintptr_t               context
);

/*****************************************************************************/
/* Elliptic curve functions.    DRV_PIC32MZW_Crypto_ECCGetField              */
/*                              DRV_PIC32MZW_Crypto_ECCGetOrder              */
/*                              DRV_PIC32MZW_Crypto_ECCIsOnCurve             */
/*                              DRV_PIC32MZW_Crypto_ECCBigIntModMultByA      */
/*                              DRV_PIC32MZW_Crypto_ECCBigIntModAddB         */
/*                              DRV_PIC32MZW_Crypto_ECCAdd                   */
/*                              DRV_PIC32MZW_Crypto_ECCMultiply              */
/*                                                                           */
/* With the exception of ECCGetField and ECCGetOrder, outputs are only valid */
/* if the return is DRV_PIC32MZW_CRYPTO_COMPLETE.                            */
/* Additionally, for ECCAdd and ECCMultiply, the output is_infinity or       */
/* is_notoncurve should be checked in order to determine whether the other   */
/* outputs are valid.                                                        */
/* ECCIsOnCurve, ECCAdd and ECCMultiply can take either big or little endian */
/* parameters; the is_be parameter applies to all in/out arrays              */
/* The other APIs require little endian parameters.                          */
/* The in/out arrays do not need to be distinct.                             */
/*****************************************************************************/
/* out is a pointer to the field of the curve. */
const uint8_t* DRV_PIC32MZW_Crypto_ECCGetField
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id
);
/* out is a pointer to the order of the curve. */
const uint8_t* DRV_PIC32MZW_Crypto_ECCGetOrder
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id
);
/* is_notoncurve = false if p(x,y) is on curve, true otherwise. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_ECCIsOnCurve
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id,
        bool                            *is_notoncurve,
        const uint8_t                   *px,
        const uint8_t                   *py,
        bool                            is_be,
        DRV_PIC32MZW_CRYPTO_CB          callback,
        uintptr_t                       context
);
/* out = a*in, with 'a' and modulo appropriate for curve_id */
/* Params must be little endian, of size equal to the curve's field size. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_ECCBigIntModMultByA
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id,
        uint8_t                         *out,
        const uint8_t                   *in,
        DRV_PIC32MZW_CRYPTO_CB          callback,
        uintptr_t                       context
);
/* out = in+b, with 'b' and modulo appropriate for curve_id */
/* Params must be little endian, of size equal to the curve's field size. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_ECCBigIntModAddB
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id,
        uint8_t                         *out,
        const uint8_t                   *in,
        DRV_PIC32MZW_CRYPTO_CB          callback,
        uintptr_t                       context
);
/* out(x,y) = elem-op(Pin(x,y), Qin(x,y)). If the result is the point at     */
/* infinity, is_infinity = true, and out(x,y) should be ignored.             */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_ECCAdd
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id,
        bool                            *is_infinity,
        uint8_t                         *outx,
        uint8_t                         *outy,
        const uint8_t                   *pinx,
        const uint8_t                   *piny,
        const uint8_t                   *qinx,
        const uint8_t                   *qiny,
        bool                            is_be,
        DRV_PIC32MZW_CRYPTO_CB          callback,
        uintptr_t                       context
);
/* out(x,y) = scalar-op(kin, pin(x,y)). If the result is the point at        */
/* infinity, is_infinity = true, and out(x,y) should be ignored.             */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_ECCMultiply
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id,
        bool                            *is_infinity,
        uint8_t                         *outx,
        uint8_t                         *outy,
        const uint8_t                   *pinx,
        const uint8_t                   *piny,
        const uint8_t                   *kin,
        bool                            is_be,
        DRV_PIC32MZW_CRYPTO_CB          callback,
        uintptr_t                       context
);

#endif /* _DRV_PIC32MZW1_CRYPTO_H */
