/*******************************************************************************
  PIC32MZW Wireless Driver

  File Name:
    drv_pic32mzw1_crypto.c

  Summary:
    PIC32MZW wireless driver interface to crypto functionality.

  Description:
    PIC32MZW wireless driver interface to crypto functionality.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

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
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "system_config.h"
#include "system_definitions.h"
#include "drv_pic32mzw1.h"
#include "drv_pic32mzw1_crypto.h"
#include "crypto/crypto.h"
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
#include "driver/ba414e/drv_ba414e.h"
#endif
#include "osal/osal.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
#define BA414E_MODULE_IDX       0
#define BA414E_MAX_OPERAND_LEN  64  // 512 bits
#endif

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
typedef struct {
    const uint8_t   *primeField_be;
    const uint8_t   *order_be;
    const uint8_t   *sqrt_exp_le;
} AUX_PARAMS;

typedef struct {
    DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id;
    DRV_BA414E_ECC_DOMAIN           curve_params;
    AUX_PARAMS                      curve_aux_params;
} CURVE_INFO;
#endif

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
/* Parameters for curve SecP256r1. */
static const uint8_t CRYPT_ECC_Curve_secp256r1_p_le[32] __attribute__((__aligned__)) = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0xff,0xff,0xff,0xff};
static const uint8_t CRYPT_ECC_Curve_secp256r1_n_le[32] __attribute__((__aligned__)) = {0x51,0x25,0x63,0xfc,0xc2,0xca,0xb9,0xf3,0x84,0x9e,0x17,0xa7,0xad,0xfa,0xe6,0xbc,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff};
static const uint8_t CRYPT_ECC_Curve_secp256r1_gx_le[32] __attribute__((__aligned__)) = {0x96,0xc2,0x98,0xd8,0x45,0x39,0xa1,0xf4,0xa0,0x33,0xeb,0x2d,0x81,0x7d,0x03,0x77,0xf2,0x40,0xa4,0x63,0xe5,0xe6,0xbc,0xf8,0x47,0x42,0x2c,0xe1,0xf2,0xd1,0x17,0x6b};
static const uint8_t CRYPT_ECC_Curve_secp256r1_gy_le[32] __attribute__((__aligned__)) = {0xf5,0x51,0xbf,0x37,0x68,0x40,0xb6,0xcb,0xce,0x5e,0x31,0x6b,0x57,0x33,0xce,0x2b,0x16,0x9e,0x0f,0x7c,0x4a,0xeb,0xe7,0x8e,0x9b,0x7f,0x1a,0xfe,0xe2,0x42,0xe3,0x4f};
static const uint8_t CRYPT_ECC_Curve_secp256r1_a_le[32] __attribute__((__aligned__)) = {0xfc,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0xff,0xff,0xff,0xff};
static const uint8_t CRYPT_ECC_Curve_secp256r1_b_le[32] __attribute__((__aligned__)) = {0x4b,0x60,0xd2,0x27,0x3e,0x3c,0xce,0x3b,0xf6,0xb0,0x53,0xcc,0xb0,0x06,0x1d,0x65,0xbc,0x86,0x98,0x76,0x55,0xbd,0xeb,0xb3,0xe7,0x93,0x3a,0xaa,0xd8,0x35,0xc6,0x5a};

/* Exponent for square root modulo the prime for curve SecP256r1. */
static const uint8_t secp256r1_sqrtexp_le[32] __attribute__((__aligned__)) = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0xc0,0xff,0xff,0xff,0x3f};

/* Big endian parameters for curve SecP256r1. */
static const uint8_t CRYPT_ECC_Curve_secp256r1_p_be[32] __attribute__((__aligned__)) = {0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static const uint8_t CRYPT_ECC_Curve_secp256r1_n_be[32] __attribute__((__aligned__)) = {0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51};

/* Array of all supported curves. */
static const CURVE_INFO g_SupportedCurves[] = {
    /* Curve SecP256r1. */
    {
        .curve_id                       = DRV_PIC32MZW1_CRYPTO_FCG_CURVE_P256,
        .curve_params.keySize           = 32,
        .curve_params.opSize            = DRV_BA414E_OPSZ_256,
        .curve_params.primeField        = CRYPT_ECC_Curve_secp256r1_p_le,
        .curve_params.order             = CRYPT_ECC_Curve_secp256r1_n_le,
        .curve_params.generatorX        = CRYPT_ECC_Curve_secp256r1_gx_le,
        .curve_params.generatorY        = CRYPT_ECC_Curve_secp256r1_gy_le,
        .curve_params.a                 = CRYPT_ECC_Curve_secp256r1_a_le,
        .curve_params.b                 = CRYPT_ECC_Curve_secp256r1_b_le,
        .curve_params.cofactor          = 1,
        .curve_aux_params.primeField_be = CRYPT_ECC_Curve_secp256r1_p_be,
        .curve_aux_params.order_be      = CRYPT_ECC_Curve_secp256r1_n_be,
        .curve_aux_params.sqrt_exp_le   = secp256r1_sqrtexp_le
    }
    /* No other curves supported. */
};
#define NUM_CURVES (sizeof(g_SupportedCurves) / sizeof(g_SupportedCurves[0]))
#endif

static CRYPT_RNG_CTX *rng_context;

/*****************************************************************************/
/* Internal functions                                                        */
/*****************************************************************************/

#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
static const CURVE_INFO *_DRV_PIC32MZW_GetCurve
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T curve_id
)
{
    int i = NUM_CURVES;

    while (i--)
    {
        if (g_SupportedCurves[i].curve_id == curve_id)
        {
            return &g_SupportedCurves[i];
        }
    }
    return NULL;
}

static const DRV_BA414E_ECC_DOMAIN *_DRV_PIC32MZW_GetDomain_Ba414e
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T curve_id
)
{
    const CURVE_INFO *curve = _DRV_PIC32MZW_GetCurve(curve_id);

    if (NULL != curve)
    {
        return &curve->curve_params;
    }
    return NULL;
}

static uint16_t _DRV_PIC32MZW_GetBufLen_Ba414e
(
        uint16_t buf_len
)
{
    if (buf_len < 16)
    {
        buf_len = 16;
    }
    return (buf_len + 7) & ~0x7;
}

/* It is the caller's responsibility to check param_len <= 64 (bytes). */
static DRV_BA414E_OPERAND_SIZE _DRV_PIC32MZW_GetOperandSize_Ba414e
(
        uint16_t operand_len
)
{
    if (operand_len <= 16)
    {
        return DRV_BA414E_OPSZ_128;
    }
    if (operand_len <= 24)
    {
        return DRV_BA414E_OPSZ_192;
    }
    if (operand_len <= 32)
    {
        return DRV_BA414E_OPSZ_256;
    }
    if (operand_len <= 40)
    {
        return DRV_BA414E_OPSZ_320;
    }
    if (operand_len <= 48)
    {
        return DRV_BA414E_OPSZ_384;
    }
    if (operand_len <= 56)
    {
        return DRV_BA414E_OPSZ_448;
    }
    return DRV_BA414E_OPSZ_512;
}

/* out and in must not overlap. */
static void _DRV_PIC32MZW_CopyBuffer_Ba414e
(
        uint8_t *out,
        const uint8_t *in,
        uint16_t buf_len,
        bool is_be
)
{
    if (is_be)
    {
        while (buf_len--)
        {
            *out++ = in[buf_len];
        }
    }
    else
    {
        memcpy (out, in, buf_len);
    }
}

/* out = sqrt(in), modulo curve->curve_params.primeField */
/* (curve->curve_params.primeField % 4) must be 3. */
/* Params must be little endian, of size curve->curve_params.opSize */
static bool _DRV_PIC32MZW_BigIntModSquareroot
(
        const CURVE_INFO *curve,
        uint8_t *out,
        const uint8_t *in
)
{
    if ((NULL == curve) || (NULL == out) || (NULL == in))
    {
        return false;
    }

    /* Our method of square root by exponentiation is only valid if mod is   */
    /* 3 mod 4.                                                              */
    if ((curve->curve_params.primeField[0] & 3) != 3)
    {
        return false;
    }

    if (true == DRV_PIC32MZW1_Crypto_BigIntModExponentiate(
            curve->curve_params.primeField,
            out,
            in,
            curve->curve_aux_params.sqrt_exp_le,
            curve->curve_params.keySize,
            false))
    {
        return true;
    }
    return false;
}
#endif

/*****************************************************************************/
/* Big integer functions:   DRV_PIC32MZW1_Crypto_Random                      */
/*****************************************************************************/
bool DRV_PIC32MZW1_Crypto_Random_Init(CRYPT_RNG_CTX *pRngCtx)
{
    rng_context = pRngCtx;
    return true;
}
/* out = random array of length param_len.                                   */
/* out is only valid if return is true.                                      */
bool DRV_PIC32MZW1_Crypto_Random
(
        uint8_t         *out,
        uint16_t        param_len
)
{
    bool ret = true;

    if (NULL == out)
    {
        return false;
    }

    if (NULL == rng_context)
    {
        return false;
    }

    if (0 > CRYPT_RNG_BlockGenerate(rng_context,
                                    out,
                                    param_len))
    {
        ret = false;
    }

    return ret;
}

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
)
{
    CRYPT_HMAC_CTX *hmac_context;
    int i;

    if ((NULL == salt) || (NULL == input_data_buffers) || (NULL == digest))
    {
        return false;
    }
    for (i = 0; i < num_buffers; i++)
    {
        if (NULL == input_data_buffers[i].data)
        {
            return false;
        }
    }

    hmac_context = OSAL_Malloc(sizeof(CRYPT_HMAC_CTX));
    if (NULL == hmac_context)
    {
        return false;
    }

    /* Initialise the HMAC. */
    if (0 != CRYPT_HMAC_SetKey(hmac_context, CRYPT_HMAC_SHA256, salt, salt_len))
    {
        goto ERR;
    }

    /* Add each buffer. */
    for (i = 0; i < num_buffers; i++)
    {
        if (0 != CRYPT_HMAC_DataAdd(
                hmac_context,
                input_data_buffers[i].data,
                input_data_buffers[i].data_len))
        {
            goto ERR;
        }
    }

    /* Finalise the HMAC and obtain the digest. */
    if (0 != CRYPT_HMAC_Finalize(hmac_context, digest))
    {
        goto ERR;
    }

    OSAL_Free(hmac_context);
    return true;
ERR:
    OSAL_Free(hmac_context);
    return false;
}

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
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    DRV_HANDLE handle;
    DRV_BA414E_OP_RESULT result;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *out_param = out;

    if ((NULL == mod) || (NULL == out) || (NULL == ain) || (NULL == bin))
    {
        return false;
    }
    if (param_len > BA414E_MAX_OPERAND_LEN)
    {
        return false;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(param_len);
    if (is_be || (buffer_len_ba414e != param_len))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(4*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return false;
        }

        memset(buffers_ba414e, 0, 4*buffer_len_ba414e);
        out = buffers_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out + buffer_len_ba414e, ain, param_len, is_be);
        ain = out + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(ain + buffer_len_ba414e), bin, param_len, is_be);
        bin = ain + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(bin + buffer_len_ba414e), mod, param_len, is_be);
        mod = bin + buffer_len_ba414e;
    }

    /* Open BA414E driver in blocking mode. */
    handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);

    result = DRV_BA414E_PRIM_ModAddition(
            handle,
            _DRV_PIC32MZW_GetOperandSize_Ba414e(buffer_len_ba414e),
            out,
            mod,
            ain, bin,
            NULL, 0);

    DRV_BA414E_Close(handle);

    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out_param, out, param_len, is_be);
        OSAL_Free(buffers_ba414e);
    }

    if (DRV_BA414E_OP_SUCCESS == result)
    {
        return true;
    }
#endif
    return false;
}
/* out = dimin - subin. */
bool DRV_PIC32MZW1_Crypto_BigIntModSubtract
(
        const uint8_t   *mod,
        uint8_t         *out,
        const uint8_t   *dimin,
        const uint8_t   *subin,
        uint16_t        param_len,
        bool            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    DRV_HANDLE handle;
    DRV_BA414E_OP_RESULT result;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *out_param = out;

    if ((NULL == mod) || (NULL == out) || (NULL == dimin) || (NULL == subin))
    {
        return false;
    }
    if (param_len > BA414E_MAX_OPERAND_LEN)
    {
        return false;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(param_len);
    if (is_be || (buffer_len_ba414e != param_len))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(4*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return false;
        }

        memset(buffers_ba414e, 0, 4*buffer_len_ba414e);
        out = buffers_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out + buffer_len_ba414e, dimin, param_len, is_be);
        dimin = out + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(dimin + buffer_len_ba414e), subin, param_len, is_be);
        subin = dimin + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(subin + buffer_len_ba414e), mod, param_len, is_be);
        mod = subin + buffer_len_ba414e;
    }

    /* Open BA414E driver in blocking mode. */
    handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);

    result = DRV_BA414E_PRIM_ModSubtraction(
            handle,
            _DRV_PIC32MZW_GetOperandSize_Ba414e(buffer_len_ba414e),
            out,
            mod,
            dimin, subin,
            NULL, 0);

    DRV_BA414E_Close(handle);

    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out_param, out, param_len, is_be);
        OSAL_Free(buffers_ba414e);
    }

    if (DRV_BA414E_OP_SUCCESS == result)
    {
        return true;
    }
#endif
    return false;
}
/* out = ain * bin. */
bool DRV_PIC32MZW1_Crypto_BigIntModMultiply
(
        const uint8_t   *mod,
        uint8_t         *out,
        const uint8_t   *ain,
        const uint8_t   *bin,
        uint16_t        param_len,
        bool            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    DRV_HANDLE handle;
    DRV_BA414E_OP_RESULT result;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *out_param = out;

    if ((NULL == mod) || (NULL == out) || (NULL == ain) || (NULL == bin))
    {
        return false;
    }
    if (param_len > BA414E_MAX_OPERAND_LEN)
    {
        return false;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(param_len);
    if (is_be || (buffer_len_ba414e != param_len))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(4*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return false;
        }

        memset(buffers_ba414e, 0, 4*buffer_len_ba414e);
        out = buffers_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out + buffer_len_ba414e, ain, param_len, is_be);
        ain = out + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(ain + buffer_len_ba414e), bin, param_len, is_be);
        bin = ain + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(bin + buffer_len_ba414e), mod, param_len, is_be);
        mod = bin + buffer_len_ba414e;
    }

    /* Open BA414E driver in blocking mode. */
    handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);

    result = DRV_BA414E_PRIM_ModMultiplication(
            handle,
            _DRV_PIC32MZW_GetOperandSize_Ba414e(buffer_len_ba414e),
            out,
            mod,
            ain, bin,
            NULL, 0);

    DRV_BA414E_Close(handle);

    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out_param, out, param_len, is_be);
        OSAL_Free(buffers_ba414e);
    }

    if (DRV_BA414E_OP_SUCCESS == result)
    {
        return true;
    }
#endif
    return false;
}
/* out = basein ^ expin. */
bool DRV_PIC32MZW1_Crypto_BigIntModExponentiate
(
        const uint8_t   *mod,
        uint8_t         *out,
        const uint8_t   *basein,
        const uint8_t   *expin,
        uint16_t        param_len,
        bool            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    DRV_HANDLE handle;
    DRV_BA414E_OP_RESULT result;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *out_param = out;

    if ((NULL == mod) || (NULL == out) || (NULL == basein) || (NULL == expin))
    {
        return false;
    }
    if (param_len > BA414E_MAX_OPERAND_LEN)
    {
        return false;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(param_len);
    if (is_be || (buffer_len_ba414e != param_len))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(4*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return false;
        }

        memset(buffers_ba414e, 0, 4*buffer_len_ba414e);
        out = buffers_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out + buffer_len_ba414e, basein, param_len, is_be);
        basein = out + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(basein + buffer_len_ba414e), expin, param_len, is_be);
        expin = basein + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(expin + buffer_len_ba414e), mod, param_len, is_be);
        mod = expin + buffer_len_ba414e;
    }

    /* Open BA414E driver in blocking mode. */
    handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);

    result = DRV_BA414E_PRIM_ModExponentiation(
            handle,
            _DRV_PIC32MZW_GetOperandSize_Ba414e(buffer_len_ba414e),
            out,
            mod,
            basein, expin,
            NULL, 0);

    DRV_BA414E_Close(handle);

    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out_param, out, param_len, is_be);
        OSAL_Free(buffers_ba414e);
    }

    if (DRV_BA414E_OP_SUCCESS == result)
    {
        return true;
    }
#endif
    return false;
}

/*****************************************************************************/
/* Elliptic curve functions.    DRV_PIC32MZW1_Crypto_ECCGetField             */
/*                              DRV_PIC32MZW1_Crypto_ECCGetOrder             */
/*                              DRV_PIC32MZW1_Crypto_ECCIsOnCurve            */
/*                              DRV_PIC32MZW1_Crypto_ECCGetY                 */
/*                              DRV_PIC32MZW1_Crypto_ECCInverse              */
/*                              DRV_PIC32MZW1_Crypto_ECCAdd                  */
/*                              DRV_PIC32MZW1_Crypto_ECCMultiply             */
/*                                                                           */
/* With the exception of ECCGetField and ECCGetOrder, outputs are only valid */
/* if the return is true.                                                    */
/* Additionally, for ECCAdd, ECCMultiply and ECCGetY, the output is_infinity */
/* or is_on_curve should be checked in order to determine whether the other  */
/* outputs are valid.                                                        */
/* Arrays pointed to by params are big endian.                               */
/*****************************************************************************/
/* out is a pointer to the field of the curve. */
const uint8_t* DRV_PIC32MZW1_Crypto_ECCGetField
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    const CURVE_INFO *curve = _DRV_PIC32MZW_GetCurve(curve_id);

    if (NULL != curve)
    {
        if (is_be)
        {
            return curve->curve_aux_params.primeField_be;
        }
        return curve->curve_params.primeField;
    }
#endif
    return NULL;
}
/* out is a pointer to the order of the curve. */
const uint8_t* DRV_PIC32MZW1_Crypto_ECCGetOrder
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    const CURVE_INFO *curve = _DRV_PIC32MZW_GetCurve(curve_id);

    if (NULL != curve)
    {
        if (is_be)
        {
            return curve->curve_aux_params.order_be;
        }
        return curve->curve_params.order;
    }
#endif
    return NULL;
}
/* out = true if p(x,y) is on curve, out = false otherwise. */
bool DRV_PIC32MZW1_Crypto_ECCIsOnCurve
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            *out,
        const uint8_t                   *px,
        const uint8_t                   *py,
        bool                            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    DRV_HANDLE handle;
    const DRV_BA414E_ECC_DOMAIN *domain = _DRV_PIC32MZW_GetDomain_Ba414e(curve_id);
    DRV_BA414E_OP_RESULT result;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;

    if ((NULL == out) || (NULL == px) || (NULL == py))
    {
        return false;
    }
    if (NULL == domain)
    {
        return false;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(domain->keySize);
    if (is_be || (buffer_len_ba414e != domain->keySize))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(2*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return false;
        }

        memset(buffers_ba414e, 0, 2*buffer_len_ba414e);
        _DRV_PIC32MZW_CopyBuffer_Ba414e(buffers_ba414e, px, domain->keySize, is_be);
        px = buffers_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(px + buffer_len_ba414e), py, domain->keySize, is_be);
        py = px + buffer_len_ba414e;
    }

    /* Open BA414E driver in blocking mode. */
    handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);

    result = DRV_BA414E_PRIM_EccCheckPointOnCurve(
            handle,
            domain,
            px, py,
            NULL, 0);

    DRV_BA414E_Close(handle);

    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        OSAL_Free(buffers_ba414e);
    }

    switch (result)
    {
        case DRV_BA414E_OP_SUCCESS:
        {
            /* For check operations, return value                            */
            /* DRV_CRYPTO_P32MZW1_ASYM_RESULT_OK is used to mean the check   */
            /* passed, as well as successful operation.                      */
            *out = true;
            return true;
        }
        case DRV_BA414E_OP_POINT_NOT_ON_CURVE:
        {
            *out = false;
            return true;
        }
        default:
        {
            break;
        }
    }
#endif
    return false;
}
/* yout = sqrt(xin^3 + ax + b). If the point (xin, yout) is not on the       */
/* curve, is_on_curve = false, and yout should be ignored.                   */
bool DRV_PIC32MZW1_Crypto_ECCGetY
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        bool                            *is_on_curve,
        uint8_t                         *yout,
        const uint8_t                   *xin,
        bool                            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    const DRV_BA414E_ECC_DOMAIN *domain = _DRV_PIC32MZW_GetDomain_Ba414e(curve_id);
    uint16_t buffer_len_ba414e;
    uint8_t *val1, *val2;
    uint8_t *buffers_ba414e = NULL;
    uint16_t buffers_ba414e_len;
    uint8_t *yout_param = yout;

    if ((NULL == is_on_curve) || (NULL == yout) || (NULL == xin))
    {
        return false;
    }
    if (NULL == domain)
    {
        return false;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(domain->keySize);
    buffers_ba414e_len = 2*buffer_len_ba414e;
    if (is_be || (buffer_len_ba414e != domain->keySize))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e_len += 2*buffer_len_ba414e;
    }
    buffers_ba414e = OSAL_Malloc(buffers_ba414e_len);
    if (NULL == buffers_ba414e)
    {
        return false;
    }

    memset(buffers_ba414e, 0, buffers_ba414e_len);
    val1 = buffers_ba414e;
    val2 = val1 + buffer_len_ba414e;
    if (buffers_ba414e_len > 2*buffer_len_ba414e)
    {
        yout = val2 + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e(yout + buffer_len_ba414e, xin, domain->keySize, is_be);
        xin = yout + buffer_len_ba414e;
    }

    /* Calculate x^3. */
    val1[0] = 3;
    if (true != DRV_PIC32MZW1_Crypto_BigIntModExponentiate(
            domain->primeField,
            val2,
            xin,
            val1,
            domain->keySize,
            false))
    {
        return false;
    }

    /* Calculate a*x. */
    if (true != DRV_PIC32MZW1_Crypto_BigIntModMultiply(
            domain->primeField,
            val1,
            domain->a,
            xin,
            domain->keySize,
            false))
    {
        return false;
    }
    /* Calculate x^3 + a*x. */
    if (true != DRV_PIC32MZW1_Crypto_BigIntModAdd(
            domain->primeField,
            yout,
            val1,
            val2,
            domain->keySize,
            false))
    {
        return false;
    }
    /* Calculate x^3 + a*x + b. */
    if (true != DRV_PIC32MZW1_Crypto_BigIntModAdd(
            domain->primeField,
            val1,
            yout,
            domain->b,
            domain->keySize,
            false))
    {
        return false;
    }
    /* Calculate candidate for y. */
    if (true != _DRV_PIC32MZW_BigIntModSquareroot(
            _DRV_PIC32MZW_GetCurve(curve_id),
            yout,
            val1))
    {
        return false;
    }

    if (true != DRV_PIC32MZW1_Crypto_ECCIsOnCurve(
            curve_id,
            is_on_curve,
            xin,
            yout,
            false))
    {
        return false;
    }

    if (buffers_ba414e_len > 2*buffer_len_ba414e)
    {
        /* In this case we need to copy the output into the original yout. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(yout_param, yout, domain->keySize, is_be);
    }
    OSAL_Free(buffers_ba414e);
    return true;
#else
    return false;
#endif
}
/* out(x,y) = inverse(pin(x,y)). */
bool DRV_PIC32MZW1_Crypto_ECCInverse
(
        DRV_PIC32MZW1_CRYPTO_FCG_ID_T   curve_id,
        uint8_t                         *outx,
        uint8_t                         *outy,
        const uint8_t                   *pinx,
        const uint8_t                   *piny,
        bool                            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    const CURVE_INFO *curve = _DRV_PIC32MZW_GetCurve(curve_id);
    const uint8_t *modulus;

    if ((NULL == outx) || (NULL == outy) || (NULL == pinx) || (NULL == piny))
    {
        return false;
    }
    if (NULL == curve)
    {
        return false;
    }

    if (is_be)
    {
        modulus = curve->curve_aux_params.primeField_be;
    }
    else
    {
        modulus = curve->curve_params.primeField;
    }
    if (true == DRV_PIC32MZW1_Crypto_BigIntModSubtract(
            modulus,
            outy,
            modulus,
            piny,
            curve->curve_params.keySize,
            is_be))
    {
        memcpy(outx, pinx, curve->curve_params.keySize);
        return true;
    }
#endif
    return false;
}
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
        bool                            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    DRV_HANDLE handle;
    const DRV_BA414E_ECC_DOMAIN *domain = _DRV_PIC32MZW_GetDomain_Ba414e(curve_id);
    DRV_BA414E_OP_RESULT result;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *outx_param = outx;
    uint8_t *outy_param = outy;

    if (
            (NULL == is_infinity)
        ||  (NULL == outx) || (NULL == outy)
        ||  (NULL == pinx) || (NULL == piny)
        ||  (NULL == qinx) || (NULL == qiny)
    )
    {
        return false;
    }
    if (NULL == domain)
    {
        return false;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(domain->keySize);
    if (is_be || (buffer_len_ba414e != domain->keySize))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(6*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return false;
        }

        memset(buffers_ba414e, 0, 6*buffer_len_ba414e);
        outx = buffers_ba414e;
        outy = outx + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e(outy + buffer_len_ba414e, pinx, domain->keySize, is_be);
        pinx = outy + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(pinx + buffer_len_ba414e), piny, domain->keySize, is_be);
        piny = pinx + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(piny + buffer_len_ba414e), qinx, domain->keySize, is_be);
        qinx = piny + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(qinx + buffer_len_ba414e), qiny, domain->keySize, is_be);
        qiny = qinx + buffer_len_ba414e;
    }

    /* Open BA414E driver in blocking mode. */
    handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);

    if (
            (0 == memcmp(piny, qiny, domain->keySize))
        &&  (0 == memcmp(pinx, qinx, domain->keySize))
    )
    {
        result = DRV_BA414E_PRIM_EccPointDouble(
                handle,
                domain,
                outx, outy,
                pinx, piny,
                NULL, 0);
    }
    else
    {
        result = DRV_BA414E_PRIM_EccPointAddition(
                handle,
                domain,
                outx, outy,
                pinx, piny,
                qinx, qiny,
                NULL, 0);
    }

    DRV_BA414E_Close(handle);

    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(outx_param, outx, domain->keySize, is_be);
        _DRV_PIC32MZW_CopyBuffer_Ba414e(outy_param, outy, domain->keySize, is_be);
        OSAL_Free(buffers_ba414e);
    }

    switch (result)
    {
        case DRV_BA414E_OP_SUCCESS:
        {
            *is_infinity = false;
            break;
        }
        case DRV_BA414E_OP_POINT_AT_INFINITY:
        {
            *is_infinity = true;
            break;
        }
        default:
        {
            return false;
        }
    }
    return true;
#else
    return false;
#endif
}
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
        bool                            is_be
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    DRV_HANDLE handle;
    const DRV_BA414E_ECC_DOMAIN *domain = _DRV_PIC32MZW_GetDomain_Ba414e(curve_id);
    DRV_BA414E_OP_RESULT result;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *outx_param = outx;
    uint8_t *outy_param = outy;

    if (
            (NULL == is_infinity)
        ||  (NULL == outx) || (NULL == outy)
        ||  (NULL == pinx) || (NULL == piny)
        ||  (NULL == kin)
    )
    {
        return false;
    }
    if (NULL == domain)
    {
        return false;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(domain->keySize);
    if (is_be || (buffer_len_ba414e != domain->keySize))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(5*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return false;
        }

        memset(buffers_ba414e, 0, 5*buffer_len_ba414e);
        outx = buffers_ba414e;
        outy = outx + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e(outy + buffer_len_ba414e, pinx, domain->keySize, is_be);
        pinx = outy + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(pinx + buffer_len_ba414e), piny, domain->keySize, is_be);
        piny = pinx + buffer_len_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(piny + buffer_len_ba414e), kin, domain->keySize, is_be);
        kin = piny + buffer_len_ba414e;
    }

    /* Open BA414E driver in blocking mode. */
    handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);

    result = DRV_BA414E_PRIM_EccPointMultiplication(
            handle,
            domain,
            outx, outy,
            pinx, piny,
            kin,
            NULL, 0);

    DRV_BA414E_Close(handle);

    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(outx_param, outx, domain->keySize, is_be);
        _DRV_PIC32MZW_CopyBuffer_Ba414e(outy_param, outy, domain->keySize, is_be);
        OSAL_Free(buffers_ba414e);
    }

    switch (result)
    {
        case DRV_BA414E_OP_SUCCESS:
        {
            *is_infinity = false;
            break;
        }
        case DRV_BA414E_OP_POINT_AT_INFINITY:
        {
            *is_infinity = true;
            break;
        }
        default:
        {
            return false;
        }
    }
    return true;
#else
    return false;
#endif
}
