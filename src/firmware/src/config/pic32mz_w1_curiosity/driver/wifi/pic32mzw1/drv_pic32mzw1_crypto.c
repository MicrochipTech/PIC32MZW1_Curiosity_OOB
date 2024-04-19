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
#include <stdio.h>

#include "wdrv_pic32mzw_common.h"
#include "drv_pic32mzw1.h"
#include "drv_pic32mzw1_crypto.h"
#include "crypto/crypto.h"
#include "osal/osal.h"
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
#include "driver/ba414e/drv_ba414e.h"
#endif
#ifdef WDRV_PIC32MZW_BIGINTSW_SUPPORT
#include "wolfssl/wolfcrypt/tfm.h"
#endif
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
#include "wolfssl/wolfcrypt/types.h"
#include "wolfssl/wolfcrypt/hash.h"
#include "wolfssl/wolfcrypt/des3.h"
#endif
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
    DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id;
    DRV_BA414E_ECC_DOMAIN           curve_params;
} CURVE_INFO;

typedef struct {
    DRV_PIC32MZW_CRYPTO_CB  fw_cb_ba414e;
    uintptr_t               fw_context_ba414e;
    DRV_HANDLE              handle;
    uint8_t                 *buffers_ba414e;
    uint8_t                 *out_param1;
    uint8_t                 *out_param2;
    uint16_t                param_len;
    bool                    is_be;
    bool                    *out_bool;
} CB_BA414E_INFO;
#endif

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
/* Parameters for curve SecP256r1. */
static const uint8_t CRYPT_ECC_Curve_secp256r1_p_le[32] __attribute__((aligned(4))) = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0xff,0xff,0xff,0xff};
static const uint8_t CRYPT_ECC_Curve_secp256r1_n_le[32] __attribute__((aligned(4))) = {0x51,0x25,0x63,0xfc,0xc2,0xca,0xb9,0xf3,0x84,0x9e,0x17,0xa7,0xad,0xfa,0xe6,0xbc,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff};
static const uint8_t CRYPT_ECC_Curve_secp256r1_gx_le[32] __attribute__((aligned(4))) = {0x96,0xc2,0x98,0xd8,0x45,0x39,0xa1,0xf4,0xa0,0x33,0xeb,0x2d,0x81,0x7d,0x03,0x77,0xf2,0x40,0xa4,0x63,0xe5,0xe6,0xbc,0xf8,0x47,0x42,0x2c,0xe1,0xf2,0xd1,0x17,0x6b};
static const uint8_t CRYPT_ECC_Curve_secp256r1_gy_le[32] __attribute__((aligned(4))) = {0xf5,0x51,0xbf,0x37,0x68,0x40,0xb6,0xcb,0xce,0x5e,0x31,0x6b,0x57,0x33,0xce,0x2b,0x16,0x9e,0x0f,0x7c,0x4a,0xeb,0xe7,0x8e,0x9b,0x7f,0x1a,0xfe,0xe2,0x42,0xe3,0x4f};
static const uint8_t CRYPT_ECC_Curve_secp256r1_a_le[32] __attribute__((aligned(4))) = {0xfc,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0xff,0xff,0xff,0xff};
static const uint8_t CRYPT_ECC_Curve_secp256r1_b_le[32] __attribute__((aligned(4))) = {0x4b,0x60,0xd2,0x27,0x3e,0x3c,0xce,0x3b,0xf6,0xb0,0x53,0xcc,0xb0,0x06,0x1d,0x65,0xbc,0x86,0x98,0x76,0x55,0xbd,0xeb,0xb3,0xe7,0x93,0x3a,0xaa,0xd8,0x35,0xc6,0x5a};

/* Array of all supported curves. */
static const CURVE_INFO g_SupportedCurves[] = {
    /* Curve SecP256r1. */
    {
        .curve_id                       = DRV_PIC32MZW_CRYPTO_FCG_CURVE_P256,
        .curve_params.keySize           = 32,
        .curve_params.opSize            = DRV_BA414E_OPSZ_256,
        .curve_params.primeField        = CRYPT_ECC_Curve_secp256r1_p_le,
        .curve_params.order             = CRYPT_ECC_Curve_secp256r1_n_le,
        .curve_params.generatorX        = CRYPT_ECC_Curve_secp256r1_gx_le,
        .curve_params.generatorY        = CRYPT_ECC_Curve_secp256r1_gy_le,
        .curve_params.a                 = CRYPT_ECC_Curve_secp256r1_a_le,
        .curve_params.b                 = CRYPT_ECC_Curve_secp256r1_b_le,
        .curve_params.cofactor          = 1,
    }
    /* No other curves supported. */
};
#define NUM_CURVES (sizeof(g_SupportedCurves) / sizeof(g_SupportedCurves[0]))

static CB_BA414E_INFO g_cb_ba414e_info[DRV_BA414E_NUM_CLIENTS] = {};
#endif

static CRYPT_RNG_CTX *rng_context;

void DRV_PIC32MZW_CryptoCallbackPush
(
    DRV_PIC32MZW_CRYPTO_CB fw_cb,
    DRV_PIC32MZW_CRYPTO_RETURN_T status,
    uintptr_t context
);

/*****************************************************************************/
/* Internal functions                                                        */
/*****************************************************************************/
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT

static enum wc_HashType DRV_PIC32MZW_Crypto_Hash_GetWCHashType(DRV_PIC32MZW_CRYPTO_HASH_T hashType)
{
    enum wc_HashType wcHashType = WC_HASH_TYPE_NONE;
    switch(hashType)
    {
        case DRV_PIC32MZW_CRYPTO_MD4:
            wcHashType = WC_HASH_TYPE_MD4;
            break;
        case DRV_PIC32MZW_CRYPTO_MD5:
            wcHashType = WC_HASH_TYPE_MD5;
            break;
        case DRV_PIC32MZW_CRYPTO_SHA:
            wcHashType = WC_HASH_TYPE_SHA;
            break;
        case DRV_PIC32MZW_CRYPTO_SHA224:
            wcHashType = WC_HASH_TYPE_SHA224;
            break;
        case DRV_PIC32MZW_CRYPTO_SHA256:
            wcHashType = WC_HASH_TYPE_SHA256;
            break;
        case DRV_PIC32MZW_CRYPTO_SHA384:
            wcHashType = WC_HASH_TYPE_SHA384;
            break;
        case DRV_PIC32MZW_CRYPTO_SHA512:
            wcHashType = WC_HASH_TYPE_SHA512;
            break;    
        default: 
            wcHashType = WC_HASH_TYPE_NONE;
            break;
    }
    return wcHashType;
}
#endif 

#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
static const CURVE_INFO *_DRV_PIC32MZW_GetCurve
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T curve_id
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
        DRV_PIC32MZW_CRYPTO_FCG_ID_T curve_id
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

static CB_BA414E_INFO *_DRV_PIC32MZW_NewCbInfo_Ba414e
(
        DRV_PIC32MZW_CRYPTO_CB fw_cb
)
{
    int index = DRV_BA414E_NUM_CLIENTS;

    if (NULL == fw_cb)
    {
        return NULL;
    }
    while (index--)
    {
        CB_BA414E_INFO *info = &g_cb_ba414e_info[index];

        if (NULL == info->fw_cb_ba414e)
        {
            info->fw_cb_ba414e = fw_cb;
            return info;
        }
    }
    return NULL;
}

static bool _DRV_PIC32MZW_CheckValidCbInfo_Ba414e(CB_BA414E_INFO *info)
{
    int index = DRV_BA414E_NUM_CLIENTS;

    while (index--)
    {
        if (info == &g_cb_ba414e_info[index])
        {
            if (NULL != info->fw_cb_ba414e)
            {
                return true;
            }
        }
    }
    return false;
}

static void _DRV_PIC32MZW_Callback_Ba414e
(
        DRV_BA414E_OP_RESULT result,
        uintptr_t context
)
{
    CB_BA414E_INFO                  *info = (CB_BA414E_INFO *)context;
    DRV_PIC32MZW_CRYPTO_RETURN_T    fw_result;

    if (true != _DRV_PIC32MZW_CheckValidCbInfo_Ba414e(info))
    {
        return;
    }
    DRV_BA414E_Close(info->handle);

    if (NULL != info->out_bool)
    {
        *(info->out_bool) = false;
    }
    switch (result)
    {
        case DRV_BA414E_OP_POINT_NOT_ON_CURVE:
        case DRV_BA414E_OP_POINT_AT_INFINITY:
        {
            if (NULL != info->out_bool)
            {
                *(info->out_bool) = true;
            }
        }
        // intentional fallthrough
        case DRV_BA414E_OP_SUCCESS:
        {
            if (NULL != info->buffers_ba414e)
            {
                /* In this case we created new buffers for the calculation. */
                if (NULL != info->out_param1)
                {
                    _DRV_PIC32MZW_CopyBuffer_Ba414e(
                            info->out_param1,
                            info->buffers_ba414e,
                            info->param_len,
                            info->is_be);
                }
                if (NULL != info->out_param2)
                {
                    _DRV_PIC32MZW_CopyBuffer_Ba414e(
                            info->out_param2,
                            info->buffers_ba414e + _DRV_PIC32MZW_GetBufLen_Ba414e(info->param_len),
                            info->param_len,
                            info->is_be);
                }
                OSAL_Free(info->buffers_ba414e);
            }
            fw_result = DRV_PIC32MZW_CRYPTO_COMPLETE;
        }
        break;
        default:
        {
            fw_result = DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
        }
        break;
    }
    DRV_PIC32MZW_CryptoCallbackPush(info->fw_cb_ba414e, fw_result, info->fw_context_ba414e);
    memset(info, 0, sizeof(CB_BA414E_INFO));
}
#endif

/*****************************************************************************/
/* Big integer functions:   DRV_PIC32MZW_Crypto_Random                       */
/*****************************************************************************/
bool DRV_PIC32MZW_Crypto_Random_Init(CRYPT_RNG_CTX *pRngCtx)
{
    rng_context = pRngCtx;
    return true;
}
/* out = random array of length param_len.                                   */
/* out is only valid if return is true.                                      */
bool DRV_PIC32MZW_Crypto_Random
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
/* Hash functions:      DRV_PIC32MZW_Crypto_Hash                             */
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
)
{
#ifdef WDRV_PIC32MZW_WOLFSSL_SUPPORT
    int i, ret=0;
    enum wc_HashType wcHashType;
    
    if((NULL == input_data_buffers) || (NULL == digest) || (0 == num_buffers))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }  
    for (i = 0; i < num_buffers; i++)
    {
        if (NULL == input_data_buffers[i].data)
        {
            return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
        }
    }
    /* Validate hashType and hash buffer size */
    wcHashType = DRV_PIC32MZW_Crypto_Hash_GetWCHashType(type);
    if(WC_HASH_TYPE_NONE == wcHashType)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    if(WC_HASH_TYPE_MD4 == wcHashType)
    {
#if !defined(NO_MD4)
        Md4 md4HashAlgCtx;
        wc_InitMd4(&md4HashAlgCtx);
        /* Add each buffer. */
        for (i = 0; i < num_buffers; i++)
        {
            wc_Md4Update(&md4HashAlgCtx, input_data_buffers[i].data, input_data_buffers[i].data_len);
        }
        /* Obtain the MD4 hash */
        wc_Md4Final(&md4HashAlgCtx, digest);
#else
		return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
#endif
    }
    else
    {
#if !defined(NO_HASH_WRAPPER)
        wc_HashAlg hashAlgCtx;
        ret = wc_HashInit(&hashAlgCtx, wcHashType);
        if(ret < 0)
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
        }
        /* Add each buffer. */
        for (i = 0; i < num_buffers; i++)
        {
            if (wc_HashUpdate(
                    &hashAlgCtx,
                    wcHashType,
                    input_data_buffers[i].data,
                    input_data_buffers[i].data_len) < 0)
            {
                wc_HashFree(&hashAlgCtx, wcHashType);
                return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
            }
        }
        /* Finalize the Hash and obtain the digest. */
        if (wc_HashFinal(&hashAlgCtx, wcHashType, digest) < 0)
        {
            wc_HashFree(&hashAlgCtx, wcHashType);
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
        }

        wc_HashFree(&hashAlgCtx, wcHashType);
#else
		return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
#endif
    }
    return DRV_PIC32MZW_CRYPTO_COMPLETE;
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}

/* Get digest size corresponding to the hash type */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_Hash_GetDigestSize
(
    int32_t                     *digest_size,
    DRV_PIC32MZW_CRYPTO_HASH_T  hashType
)
{  
#if defined(WDRV_PIC32MZW_WOLFSSL_SUPPORT) && !defined(NO_HASH_WRAPPER)
    if(NULL == digest_size)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    enum wc_HashType wcHashType = DRV_PIC32MZW_Crypto_Hash_GetWCHashType(hashType);
    if(WC_HASH_TYPE_NONE == wcHashType)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    *digest_size = wc_HashGetDigestSize(wcHashType);
    return DRV_PIC32MZW_CRYPTO_COMPLETE;
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}

/* Encrypt the input text using Des encryption with Electronic Codebook (ECB) mode*/
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_DES_Ecb_Blk_Crypt
(
    const uint8_t *pu8PlainText,
    uint16_t plainTextLen,
    const uint8_t *pu8CipherKey,    
    uint8_t *pu8CipherText    
)
{
#if defined(WDRV_PIC32MZW_WOLFSSL_SUPPORT) && defined(WOLFSSL_DES_ECB)
    uint8_t iv[DES_BLOCK_SIZE] = {0};
    if ((NULL == pu8PlainText) || (0 == plainTextLen) || (NULL == pu8CipherKey) || (NULL == pu8CipherText))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    memset(iv, 0x0, DES_BLOCK_SIZE);
    Des desCtx;
    /* set the encryption key */
    if (wc_Des_SetKey(&desCtx, (const byte*) pu8CipherKey, (const byte*) iv, DES_ENCRYPTION) != 0) 
    {
        return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
    }
	/* encrypt the data */
    if (wc_Des_EcbEncrypt(&desCtx, (byte*) pu8CipherText, (const byte*) pu8PlainText, plainTextLen) != 0)
    {
        return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
    }
    return DRV_PIC32MZW_CRYPTO_COMPLETE;
#else
    
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif /* WDRV_PIC32MZW_WOLFSSL_SUPPORT */
}

/*****************************************************************************/
/* HMAC functions.          DRV_PIC32MZW_Crypto_HMAC                         */
/*                                                                           */
/* The in/out arrays do not need to be distinct.                             */
/*****************************************************************************/
/* Run a HMACSHA256 operation. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_HMAC
(
        const uint8_t               *salt,
        uint16_t                    salt_len,
        const buffer_t              *input_data_buffers,
        int                         num_buffers,
        uint8_t                     *digest,
        DRV_PIC32MZW_CRYPTO_HASH_T  type
)
{
    CRYPT_HMAC_CTX *hmac_context;
    int i;
    int crypt_type;

    if ((NULL == salt) || (NULL == input_data_buffers) || (NULL == digest))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    for (i = 0; i < num_buffers; i++)
    {
        if (NULL == input_data_buffers[i].data)
        {
            return false;
        }
    }
    
    if      (DRV_PIC32MZW_CRYPTO_SHA    == type) crypt_type = CRYPT_HMAC_SHA;
    else if (DRV_PIC32MZW_CRYPTO_SHA256 == type) crypt_type = CRYPT_HMAC_SHA256;
    else if (DRV_PIC32MZW_CRYPTO_SHA384 == type) crypt_type = CRYPT_HMAC_SHA384;
    else if (DRV_PIC32MZW_CRYPTO_SHA512 == type) crypt_type = CRYPT_HMAC_SHA512;
    else return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    
    hmac_context = OSAL_Malloc(sizeof(CRYPT_HMAC_CTX));
    if (NULL == hmac_context)
    {
        return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
    }
    
    /* Initialise the HMAC. */
    if (0 != CRYPT_HMAC_SetKey(hmac_context, crypt_type, salt, salt_len))
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
    return DRV_PIC32MZW_CRYPTO_COMPLETE;
ERR:
    OSAL_Free(hmac_context);
    return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
}

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
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    CB_BA414E_INFO *cb_info = NULL;
    DRV_HANDLE handle;
    DRV_BA414E_OP_RESULT result = DRV_BA414E_OP_ERROR;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *out_param = out;

    if ((NULL == mod) || (NULL == out) || (NULL == ain) || (NULL == bin))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    if (param_len > BA414E_MAX_OPERAND_LEN)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(param_len);
    if (is_be || (buffer_len_ba414e != param_len))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(4*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
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

    if (NULL != callback)
    {
        /* Set up callback info and open BA414E driver in non-blocking mode. */
        cb_info = _DRV_PIC32MZW_NewCbInfo_Ba414e(callback);
        if (NULL == cb_info)
        {
            goto _ERR;
        }
        cb_info->fw_context_ba414e = context;
        cb_info->buffers_ba414e = buffers_ba414e;
        cb_info->out_param1 = out_param;
        cb_info->param_len = param_len;
        cb_info->is_be = is_be;
        cb_info->handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING);
        handle = cb_info->handle;
    }
    else
    {
        /* Open BA414E driver in blocking mode. */
        handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);
    }

    if (DRV_HANDLE_INVALID != handle)
    {
        result = DRV_BA414E_PRIM_ModAddition(
                handle,
                _DRV_PIC32MZW_GetOperandSize_Ba414e(buffer_len_ba414e),
                out,
                mod,
                ain, bin,
                _DRV_PIC32MZW_Callback_Ba414e, (uintptr_t)cb_info);

        if (NULL != cb_info)
        {
            if (DRV_BA414E_OP_PENDING == result)
            {
                /* handle and buffers_ba414e will be cleaned up in callback. */
                return DRV_PIC32MZW_CRYPTO_PENDING;
            }
            memset(cb_info, 0, sizeof(CB_BA414E_INFO));
            result = DRV_BA414E_OP_ERROR;
        }

        DRV_BA414E_Close(handle);
    }

_ERR:
    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out_param, out, param_len, is_be);
        OSAL_Free(buffers_ba414e);
    }

    if (DRV_BA414E_OP_SUCCESS == result)
    {
        return DRV_PIC32MZW_CRYPTO_COMPLETE;
    }
    return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif
}
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
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    CB_BA414E_INFO *cb_info = NULL;
    DRV_HANDLE handle;
    DRV_BA414E_OP_RESULT result = DRV_BA414E_OP_ERROR;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *out_param = out;

    if ((NULL == mod) || (NULL == out) || (NULL == dimin) || (NULL == subin))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    if (param_len > BA414E_MAX_OPERAND_LEN)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(param_len);
    if (is_be || (buffer_len_ba414e != param_len))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(4*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
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

    if (NULL != callback)
    {
        /* Set up callback info and open BA414E driver in non-blocking mode. */
        cb_info = _DRV_PIC32MZW_NewCbInfo_Ba414e(callback);
        if (NULL == cb_info)
        {
            goto _ERR;
        }
        cb_info->fw_context_ba414e = context;
        cb_info->buffers_ba414e = buffers_ba414e;
        cb_info->out_param1 = out_param;
        cb_info->param_len = param_len;
        cb_info->is_be = is_be;
        cb_info->handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING);
        handle = cb_info->handle;
    }
    else
    {
        /* Open BA414E driver in blocking mode. */
        handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);
    }

    if (DRV_HANDLE_INVALID != handle)
    {
        result = DRV_BA414E_PRIM_ModSubtraction(
                handle,
                _DRV_PIC32MZW_GetOperandSize_Ba414e(buffer_len_ba414e),
                out,
                mod,
                dimin, subin,
                _DRV_PIC32MZW_Callback_Ba414e, (uintptr_t)cb_info);

        if (NULL != cb_info)
        {
            if (DRV_BA414E_OP_PENDING == result)
            {
                /* handle and buffers_ba414e will be cleaned up in callback. */
                return DRV_PIC32MZW_CRYPTO_PENDING;
            }
            memset(cb_info, 0, sizeof(CB_BA414E_INFO));
            result = DRV_BA414E_OP_ERROR;
        }

        DRV_BA414E_Close(handle);
    }

_ERR:
    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out_param, out, param_len, is_be);
        OSAL_Free(buffers_ba414e);
    }

    if (DRV_BA414E_OP_SUCCESS == result)
    {
        return DRV_PIC32MZW_CRYPTO_COMPLETE;
    }
    return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif
}
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
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    CB_BA414E_INFO *cb_info = NULL;
    DRV_HANDLE handle;
    DRV_BA414E_OP_RESULT result = DRV_BA414E_OP_ERROR;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *out_param = out;

    if ((NULL == mod) || (NULL == out) || (NULL == ain) || (NULL == bin))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    if (param_len > BA414E_MAX_OPERAND_LEN)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(param_len);
    if (is_be || (buffer_len_ba414e != param_len))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(4*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
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

    if (NULL != callback)
    {
        /* Set up callback info and open BA414E driver in non-blocking mode. */
        cb_info = _DRV_PIC32MZW_NewCbInfo_Ba414e(callback);
        if (NULL == cb_info)
        {
            goto _ERR;
        }
        cb_info->fw_context_ba414e = context;
        cb_info->buffers_ba414e = buffers_ba414e;
        cb_info->out_param1 = out_param;
        cb_info->param_len = param_len;
        cb_info->is_be = is_be;
        cb_info->handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING);
        handle = cb_info->handle;
    }
    else
    {
        /* Open BA414E driver in blocking mode. */
        handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);
    }

    if (DRV_HANDLE_INVALID != handle)
    {
        result = DRV_BA414E_PRIM_ModMultiplication(
                handle,
                _DRV_PIC32MZW_GetOperandSize_Ba414e(buffer_len_ba414e),
                out,
                mod,
                ain, bin,
                _DRV_PIC32MZW_Callback_Ba414e, (uintptr_t)cb_info);

        if (NULL != cb_info)
        {
            if (DRV_BA414E_OP_PENDING == result)
            {
                /* handle and buffers_ba414e will be cleaned up in callback. */
                return DRV_PIC32MZW_CRYPTO_PENDING;
            }
            memset(cb_info, 0, sizeof(CB_BA414E_INFO));
            result = DRV_BA414E_OP_ERROR;
        }

        DRV_BA414E_Close(handle);
    }

_ERR:
    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out_param, out, param_len, is_be);
        OSAL_Free(buffers_ba414e);
    }

    if (DRV_BA414E_OP_SUCCESS == result)
    {
        return DRV_PIC32MZW_CRYPTO_COMPLETE;
    }
    return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif
}
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
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    CB_BA414E_INFO *cb_info = NULL;
    DRV_HANDLE handle;
    DRV_BA414E_OP_RESULT result = DRV_BA414E_OP_ERROR;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;
    uint8_t *out_param = out;

    if ((NULL == mod) || (NULL == out) || (NULL == basein) || (NULL == expin))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    if (param_len > BA414E_MAX_OPERAND_LEN)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(param_len);
    if (is_be || (buffer_len_ba414e != param_len))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(4*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
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

    if (NULL != callback)
    {
        /* Set up callback info and open BA414E driver in non-blocking mode. */
        cb_info = _DRV_PIC32MZW_NewCbInfo_Ba414e(callback);
        if (NULL == cb_info)
        {
            goto _ERR;
        }
        cb_info->fw_context_ba414e = context;
        cb_info->buffers_ba414e = buffers_ba414e;
        cb_info->out_param1 = out_param;
        cb_info->param_len = param_len;
        cb_info->is_be = is_be;
        cb_info->handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING);
        handle = cb_info->handle;
    }
    else
    {
        /* Open BA414E driver in blocking mode. */
        handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);
    }

    if (DRV_HANDLE_INVALID != handle)
    {
        result = DRV_BA414E_PRIM_ModExponentiation(
                handle,
                _DRV_PIC32MZW_GetOperandSize_Ba414e(buffer_len_ba414e),
                out,
                mod,
                basein, expin,
                _DRV_PIC32MZW_Callback_Ba414e, (uintptr_t)cb_info);

        if (NULL != cb_info)
        {
            if (DRV_BA414E_OP_PENDING == result)
            {
                /* handle and buffers_ba414e will be cleaned up in callback. */
                return DRV_PIC32MZW_CRYPTO_PENDING;
            }
            memset(cb_info, 0, sizeof(CB_BA414E_INFO));
            result = DRV_BA414E_OP_ERROR;
        }

        DRV_BA414E_Close(handle);
    }

_ERR:
    if (NULL != buffers_ba414e)
    {
        /* In this case we created new buffers for the calculation. */
        _DRV_PIC32MZW_CopyBuffer_Ba414e(out_param, out, param_len, is_be);
        OSAL_Free(buffers_ba414e);
    }

    if (DRV_BA414E_OP_SUCCESS == result)
    {
        return DRV_PIC32MZW_CRYPTO_COMPLETE;
    }
    return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif
}
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
)
{
    /* BA414E driver does not export modular reduction (WSGRIO2-1238). Use   */
    /* the blocking, big-endian implementation from wolfCrypt tfm instead.   */
#ifdef WDRV_PIC32MZW_BIGINTSW_SUPPORT
    if ((NULL == callback) && (true == is_be))
    {
        fp_int fp_a, fp_b, fp_c;

        if ((NULL == mod) || (NULL == out) || (NULL == ain))
        {
            return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
        }

        fp_zero(&fp_c);
        if (
                (0 != fp_read_unsigned_bin(&fp_b, mod, param_len))
            ||  (0 != fp_read_unsigned_bin(&fp_a, ain, ain_len))
        )
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
        }

        /* c = a % b;           */
        if (0 != fp_mod(&fp_a, &fp_b, &fp_c))
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
        }

        if (0 != fp_to_unsigned_bin_len(&fp_c, out, param_len))
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
        }
        return DRV_PIC32MZW_CRYPTO_COMPLETE;
    }
#endif /* WDRV_PIC32MZW_BIGINTSW_SUPPORT */
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
}

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
/* parameters; the is_be parameter applies to all in/out arrays.             */
/* The other APIs require little endian parameters.                          */
/* The in/out arrays do not need to be distinct.                             */
/*****************************************************************************/
/* out is a pointer to the field of the curve. */
const uint8_t* DRV_PIC32MZW_Crypto_ECCGetField
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    const CURVE_INFO *curve = _DRV_PIC32MZW_GetCurve(curve_id);

    if (NULL != curve)
    {
        return curve->curve_params.primeField;
    }
#endif
    return NULL;
}
/* out is a pointer to the order of the curve. */
const uint8_t* DRV_PIC32MZW_Crypto_ECCGetOrder
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    const CURVE_INFO *curve = _DRV_PIC32MZW_GetCurve(curve_id);

    if (NULL != curve)
    {
        return curve->curve_params.order;
    }
#endif
    return NULL;
}
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
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    CB_BA414E_INFO *cb_info = NULL;
    DRV_HANDLE handle;
    const DRV_BA414E_ECC_DOMAIN *domain = _DRV_PIC32MZW_GetDomain_Ba414e(curve_id);
    DRV_BA414E_OP_RESULT result = DRV_BA414E_OP_ERROR;
    uint16_t buffer_len_ba414e;
    uint8_t *buffers_ba414e = NULL;

    if ((NULL == is_notoncurve) || (NULL == px) || (NULL == py))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    if (NULL == domain)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(domain->keySize);
    if (is_be || (buffer_len_ba414e != domain->keySize))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(2*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
        }

        memset(buffers_ba414e, 0, 2*buffer_len_ba414e);
        _DRV_PIC32MZW_CopyBuffer_Ba414e(buffers_ba414e, px, domain->keySize, is_be);
        px = buffers_ba414e;
        _DRV_PIC32MZW_CopyBuffer_Ba414e((uint8_t*)(px + buffer_len_ba414e), py, domain->keySize, is_be);
        py = px + buffer_len_ba414e;
    }

    if (NULL != callback)
    {
        /* Set up callback info and open BA414E driver in non-blocking mode. */
        cb_info = _DRV_PIC32MZW_NewCbInfo_Ba414e(callback);
        if (NULL == cb_info)
        {
            goto _ERR;
        }
        cb_info->fw_context_ba414e = context;
        cb_info->buffers_ba414e = buffers_ba414e;
        cb_info->out_bool = is_notoncurve;
        cb_info->handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING);
        handle = cb_info->handle;
    }
    else
    {
        /* Open BA414E driver in blocking mode. */
        handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);
    }

    if (DRV_HANDLE_INVALID != handle)
    {
        result = DRV_BA414E_PRIM_EccCheckPointOnCurve(
                handle,
                domain,
                px, py,
                _DRV_PIC32MZW_Callback_Ba414e, (uintptr_t)cb_info);

        if (NULL != cb_info)
        {
            if (DRV_BA414E_OP_PENDING == result)
            {
                /* handle and buffers_ba414e will be cleaned up in callback. */
                return DRV_PIC32MZW_CRYPTO_PENDING;
            }
            memset(cb_info, 0, sizeof(CB_BA414E_INFO));
            result = DRV_BA414E_OP_ERROR;
        }

        DRV_BA414E_Close(handle);
    }

_ERR:
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
            *is_notoncurve = false;
            return DRV_PIC32MZW_CRYPTO_COMPLETE;
        }
        case DRV_BA414E_OP_POINT_NOT_ON_CURVE:
        {
            *is_notoncurve = true;
            return DRV_PIC32MZW_CRYPTO_COMPLETE;
        }
        default:
        {
            break;
        }
    }
    return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif
}
/* out = a*in, with 'a' and modulo appropriate for curve_id */
/* Params must be little endian, of size equal to the curve's field size. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_ECCBigIntModMultByA
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id,
        uint8_t                         *out,
        const uint8_t                   *in,
        DRV_PIC32MZW_CRYPTO_CB          callback,
        uintptr_t                       context
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    const CURVE_INFO *curve = _DRV_PIC32MZW_GetCurve(curve_id);

    if ((NULL == out) || (NULL == in))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    if (NULL == curve)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    return DRV_PIC32MZW_Crypto_BigIntModMultiply(
            curve->curve_params.primeField,
            out,
            in,
            curve->curve_params.a,
            curve->curve_params.keySize,
            false,
            callback,
            context);
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif
}
/* out = in+b, with 'b' and modulo appropriate for curve_id */
/* Params must be little endian, of size equal to the curve's field size. */
DRV_PIC32MZW_CRYPTO_RETURN_T DRV_PIC32MZW_Crypto_ECCBigIntModAddB
(
        DRV_PIC32MZW_CRYPTO_FCG_ID_T    curve_id,
        uint8_t                         *out,
        const uint8_t                   *in,
        DRV_PIC32MZW_CRYPTO_CB          callback,
        uintptr_t                       context
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    const CURVE_INFO *curve = _DRV_PIC32MZW_GetCurve(curve_id);

    if ((NULL == out) || (NULL == in))
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    if (NULL == curve)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    return DRV_PIC32MZW_Crypto_BigIntModAdd(
            curve->curve_params.primeField,
            out,
            in,
            curve->curve_params.b,
            curve->curve_params.keySize,
            false,
            callback,
            context);
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif
}
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
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    CB_BA414E_INFO *cb_info = NULL;
    DRV_HANDLE handle;
    const DRV_BA414E_ECC_DOMAIN *domain = _DRV_PIC32MZW_GetDomain_Ba414e(curve_id);
    DRV_BA414E_OP_RESULT result = DRV_BA414E_OP_ERROR;
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
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    if (NULL == domain)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(domain->keySize);
    if (is_be || (buffer_len_ba414e != domain->keySize))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(6*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
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

    if (NULL != callback)
    {
        /* Set up callback info and open BA414E driver in non-blocking mode. */
        cb_info = _DRV_PIC32MZW_NewCbInfo_Ba414e(callback);
        if (NULL == cb_info)
        {
            goto _ERR;
        }
        cb_info->fw_context_ba414e = context;
        cb_info->buffers_ba414e = buffers_ba414e;
        cb_info->out_param1 = outx_param;
        cb_info->out_param2 = outy_param;
        cb_info->param_len = domain->keySize;
        cb_info->is_be = is_be;
        cb_info->out_bool = is_infinity;
        cb_info->handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING);
        handle = cb_info->handle;
    }
    else
    {
        /* Open BA414E driver in blocking mode. */
        handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);
    }

    if (DRV_HANDLE_INVALID != handle)
    {
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
                    _DRV_PIC32MZW_Callback_Ba414e, (uintptr_t)cb_info);
        }
        else
        {
            result = DRV_BA414E_PRIM_EccPointAddition(
                    handle,
                    domain,
                    outx, outy,
                    pinx, piny,
                    qinx, qiny,
                    _DRV_PIC32MZW_Callback_Ba414e, (uintptr_t)cb_info);
        }

        if (NULL != cb_info)
        {
            if (DRV_BA414E_OP_PENDING == result)
            {
                /* handle and buffers_ba414e will be cleaned up in callback. */
                return DRV_PIC32MZW_CRYPTO_PENDING;
            }
            memset(cb_info, 0, sizeof(CB_BA414E_INFO));
            result = DRV_BA414E_OP_ERROR;
        }

        DRV_BA414E_Close(handle);
    }

_ERR:
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
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
        }
    }
    return DRV_PIC32MZW_CRYPTO_COMPLETE;
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif
}
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
)
{
#ifdef WDRV_PIC32MZW_BA414E_SUPPORT
    CB_BA414E_INFO *cb_info = NULL;
    DRV_HANDLE handle;
    const DRV_BA414E_ECC_DOMAIN *domain = _DRV_PIC32MZW_GetDomain_Ba414e(curve_id);
    DRV_BA414E_OP_RESULT result = DRV_BA414E_OP_ERROR;
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
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }
    if (NULL == domain)
    {
        return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
    }

    buffer_len_ba414e = _DRV_PIC32MZW_GetBufLen_Ba414e(domain->keySize);
    if (is_be || (buffer_len_ba414e != domain->keySize))
    {
        /* In this case we need to copy the parameters into new buffers,     */
        /* suitable for BA414E.                                              */
        buffers_ba414e = OSAL_Malloc(5*buffer_len_ba414e);
        if (NULL == buffers_ba414e)
        {
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
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

    if (NULL != callback)
    {
        /* Set up callback info and open BA414E driver in non-blocking mode. */
        cb_info = _DRV_PIC32MZW_NewCbInfo_Ba414e(callback);
        if (NULL == cb_info)
        {
            goto _ERR;
        }
        cb_info->fw_context_ba414e = context;
        cb_info->buffers_ba414e = buffers_ba414e;
        cb_info->out_param1 = outx_param;
        cb_info->out_param2 = outy_param;
        cb_info->param_len = domain->keySize;
        cb_info->is_be = is_be;
        cb_info->out_bool = is_infinity;
        cb_info->handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING);
        handle = cb_info->handle;
    }
    else
    {
        /* Open BA414E driver in blocking mode. */
        handle = DRV_BA414E_Open(BA414E_MODULE_IDX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);
    }

    if (DRV_HANDLE_INVALID != handle)
    {
        result = DRV_BA414E_PRIM_EccPointMultiplication(
                handle,
                domain,
                outx, outy,
                pinx, piny,
                kin,
                _DRV_PIC32MZW_Callback_Ba414e, (uintptr_t)cb_info);

        if (NULL != cb_info)
        {
            if (DRV_BA414E_OP_PENDING == result)
            {
                /* handle and buffers_ba414e will be cleaned up in callback. */
                return DRV_PIC32MZW_CRYPTO_PENDING;
            }
            memset(cb_info, 0, sizeof(CB_BA414E_INFO));
            result = DRV_BA414E_OP_ERROR;
        }

        DRV_BA414E_Close(handle);
    }

_ERR:
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
            return DRV_PIC32MZW_CRYPTO_INTERNAL_ERROR;
        }
    }
    return DRV_PIC32MZW_CRYPTO_COMPLETE;
#else
    return DRV_PIC32MZW_CRYPTO_INVALID_PARAM;
#endif
}
