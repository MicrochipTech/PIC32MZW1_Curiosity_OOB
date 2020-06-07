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

/* Generic type for handling data buffers. */
typedef struct
{
    const uint8_t   *data;
    uint16_t        data_len;
} buffer_t;

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
/* HMAC_SHA256 functions.   DRV_PIC32MZW1_Crypto_HMACSHA256                  */
/*****************************************************************************/
/* Run a HMACSHA256 operation. */
bool DRV_PIC32MZW1_Crypto_HMACSHA256
(
        const uint8_t   *salt,
        uint16_t        salt_len,
        const buffer_t  *input_data_buffers,
        int             num_buffers,
        uint8_t         *digest
);

/*****************************************************************************/
/* Big integer functions:   DRV_PIC32MZW1_Crypto_BigIntModAdd                */
/*                          DRV_PIC32MZW1_Crypto_BigIntModSubtract           */
/*                          DRV_PIC32MZW1_Crypto_BigIntModMultiply           */
/*                          DRV_PIC32MZW1_Crypto_BigIntModExponentiate       */
/*                                                                           */
/* Outputs are only valid if return is true.                                 */
/* Parameters param_len and is_be (big endian) apply to all in/out arrays.   */
/*****************************************************************************/
/* out = ain + bin. */
bool DRV_PIC32MZW1_Crypto_BigIntModAdd
(
        const uint8_t   *mod,
        uint8_t         *out,
        const uint8_t   *ain,
        const uint8_t   *bin,
        uint16_t        param_len,
        bool            is_be
);
/* out = dimin - subin. */
bool DRV_PIC32MZW1_Crypto_BigIntModSubtract
(
        const uint8_t   *mod,
        uint8_t         *out,
        const uint8_t   *dimin,
        const uint8_t   *subin,
        uint16_t        param_len,
        bool            is_be
);
/* out = ain * bin. */
bool DRV_PIC32MZW1_Crypto_BigIntModMultiply
(
        const uint8_t   *mod,
        uint8_t         *out,
        const uint8_t   *ain,
        const uint8_t   *bin,
        uint16_t        param_len,
        bool            is_be
);
/* out = basein ^ expin. */
bool DRV_PIC32MZW1_Crypto_BigIntModExponentiate
(
        const uint8_t   *mod,
        uint8_t         *out,
        const uint8_t   *basein,
        const uint8_t   *expin,
        uint16_t        param_len,
        bool            is_be
);

/*****************************************************************************/
/* Elliptic curve functions.    DRV_PIC32MZW_Crypto_ECCGetField              */
/*                              DRV_PIC32MZW_Crypto_ECCGetOrder              */
/*                              DRV_PIC32MZW_Crypto_ECCIsOnCurve             */
/*                              DRV_PIC32MZW_Crypto_ECCGetY                  */
/*                              DRV_PIC32MZW_Crypto_ECCInverse               */
/*                              DRV_PIC32MZW_Crypto_ECCAdd                   */
/*                              DRV_PIC32MZW_Crypto_ECCMultiply              */
/*                                                                           */
/* With the exception of ECCGetField and ECCGetOrder, outputs are only valid */
/* if the return is true.                                                    */
/* Additionally, for ECCAdd, ECCMultiply and ECCGetY, the output is_infinity */
/* or is_on_curve should be checked in order to determine whether the other  */
/* outputs are valid.                                                        */
/* Parameter is_be (big endian) applies to all in/out arrays.                */
/*****************************************************************************/
/* out is a pointer to the field of the curve. */
const uint8_t* DRV_PIC32MZW1_Crypto_ECCGetField
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            is_be
);
/* out is a pointer to the order of the curve. */
const uint8_t* DRV_PIC32MZW1_Crypto_ECCGetOrder
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            is_be
);
/* out = true if p(x,y) is on curve, out = false otherwise. */
bool DRV_PIC32MZW1_Crypto_ECCIsOnCurve
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            *out,
        const uint8_t                   *px,
        const uint8_t                   *py,
        bool                            is_be
);
/* yout = sqrt(xin^3 + ax + b). */
bool DRV_PIC32MZW1_Crypto_ECCGetY
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            *is_on_curve,
        uint8_t                         *yout,
        const uint8_t                   *xin,
        bool                            is_be
);
/* out(x,y) = inverse(pin(x,y)). */
bool DRV_PIC32MZW1_Crypto_ECCInverse
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        uint8_t                         *outx,
        uint8_t                         *outy,
        const uint8_t                   *pinx,
        const uint8_t                   *piny,
        bool                            is_be
);
/* out(x,y) = elem-op(Pin(x,y), Qin(x,y)). Returns false if output is point  */
/* at infinity.                                                              */
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
        bool                            is_be
);
/* out(x,y) = scalar-op(kin, pin(x,y)). Returns false if output is point at  */
/* infinity.                                                                 */
bool DRV_PIC32MZW1_Crypto_ECCMultiply
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            *is_infinity,
        uint8_t                         *outx,
        uint8_t                         *outy,
        const uint8_t                   *pinx,
        const uint8_t                   *piny,
        const uint8_t                   *kin,
        bool                            is_be
);

#endif /* _DRV_PIC32MZW1_CRYPTO_H */