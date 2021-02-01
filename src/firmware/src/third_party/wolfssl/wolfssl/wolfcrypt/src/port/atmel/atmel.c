/* atmel.c
 *
 * Copyright (C) 2006-2019 wolfSSL Inc.
 *
 * This file is part of wolfSSL.
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/port/atmel/atmel.h>
#include "tng/tng_atcacert_client.h"

#if defined(WOLFSSL_ATMEL) || defined(WOLFSSL_ATECC508A) || defined(WOLFSSL_ATECC_PKCB)

#include <wolfssl/wolfcrypt/memory.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/ssl.h>
#include <wolfssl/internal.h>

#ifdef NO_INLINE
#include <wolfssl/wolfcrypt/misc.h>
#else
#define WOLFSSL_MISC_INCLUDED
#include <wolfcrypt/src/misc.c>
#endif

#ifdef WOLFSSL_ATECC508A

#ifdef WOLFSSL_ATECC508A_TLS
//extern ATCA_STATUS device_init_default(void);
#endif

static int mAtcaInitDone = 0;

#ifndef SINGLE_THREADED
static wolfSSL_Mutex mSlotMutex;
#endif

extern ATCAIfaceCfg atecc608_0_init_data;

/**
 * \brief Generate random number to be used for hash.
 */
int atmel_get_random_number(uint32_t count, uint8_t *rand_out) {
    ATMEL_ENTRY_LOG();
    int ret = 0;
#ifdef WOLFSSL_ATECC508A
    uint8_t i = 0;
    uint32_t copy_count = 0;
    uint8_t rng_buffer[RANDOM_NUM_SIZE];

    if (rand_out == NULL) {
        return -1;
    }

    while (i < count) {
        ret = atcab_random(rng_buffer);
        if (ret != ATCA_SUCCESS) {
            SYS_CONSOLE_PRINT("    Failed to create random number!\r\n");
            return -1;
        }
        copy_count = (count - i > RANDOM_NUM_SIZE) ? RANDOM_NUM_SIZE : count - i;
        XMEMCPY(&rand_out[i], rng_buffer, copy_count);
        i += copy_count;
    }
#ifdef ATCAPRINTF
    atcab_printbin_label((const char *) "\r\nRandom Number", rand_out, count);
#endif
#else
    /* TODO: Use on-board TRNG */
#endif
    return ret;
}

int atmel_get_random_block(unsigned char *output, unsigned int sz) {
    ATMEL_ENTRY_LOG();
    return atmel_get_random_number((uint32_t) sz, (uint8_t *) output);
}

#if defined(WOLFSSL_ATMEL) && defined(WOLFSSL_ATMEL_TIME)
#include "asf.h"
#include "rtc_calendar.h"
extern struct rtc_module *_rtc_instance[RTC_INST_NUM];

long atmel_get_curr_time_and_date(long *tm) {
    long rt = 0;

    /* Get current time */
    struct rtc_calendar_time rtcTime;
    const int monthDay[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int month, year, yearLeap;

    rtc_calendar_get_time(_rtc_instance[0], &rtcTime);

    /* Convert rtc_calendar_time to seconds since UTC */
    month = rtcTime.month % 12;
    year = rtcTime.year + rtcTime.month / 12;
    if (month < 0) {
        month += 12;
        year--;
    }
    yearLeap = (month > 1) ? year + 1 : year;
    rt = rtcTime.second + 60 * (rtcTime.minute + 60 * (rtcTime.hour + 24 * (monthDay[month] + rtcTime.day - 1 + 365 * (year - 70) + (yearLeap - 69) / 4 - (yearLeap - 1) / 100 + (yearLeap + 299) / 400)));

    (void) tm;
    return rt;
}
#endif

#ifdef WOLFSSL_ATECC508A

int atmel_ecc_translate_err(int status) {
    switch (status) {
        case ATCA_SUCCESS:
            return 0;
        case ATCA_BAD_PARAM:
            ATMEL_DEBUG_LOG("    ATECC Failure: ATCA_BAD_PARAM\n");
            return BAD_FUNC_ARG;
        case ATCA_ALLOC_FAILURE:
            ATMEL_DEBUG_LOG("    ATECC Failure: ATCA_ALLOC_FAILURE\n");
            return MEMORY_E;
        default:
            ATMEL_DEBUG_LOG("    ATECC Failure: %x\n", (word32) status);
            break;
    }
    return WC_HW_E;
}

/* Function to allocate new slotId number */
int atmel_ecc_alloc(int slotType) {
    ATMEL_ENTRY_LOG();
#ifdef WOLFSSL_ATECC508A_DEBUG
    char slotString[4][20] = {"ATMEL_SLOT_ENCKEY", "ATMEL_SLOT_DEVICE", "ATMEL_SLOT_ECDHE", "ATMEL_SLOT_ECDHE_ENC"};
#endif
    int slotId = ATECC_INVALID_SLOT;

#ifndef SINGLE_THREADED
    wc_LockMutex(&mSlotMutex);
#endif

    {
        switch (slotType) {
            case ATMEL_SLOT_ENCKEY:
                /* not reserved in mSlotList, so return */
                slotId = ATECC_SLOT_I2C_ENC;
                goto exit;
            case ATMEL_SLOT_DEVICE:
                /* not reserved in mSlotList, so return */
                slotId = ATECC_SLOT_AUTH_PRIV;
                goto exit;
            case ATMEL_SLOT_ECDHE:
                slotId = ATECC_SLOT_ECDHE_PRIV;
                break;
            case ATMEL_SLOT_ECDHE_ENC:
                slotId = ATECC_SLOT_ENC_PARENT;
                break;
        }
    }

exit:
#ifndef SINGLE_THREADED
    wc_UnLockMutex(&mSlotMutex);
#endif
    ATMEL_DEBUG_LOG("    slot alloc for type: %s. Giving %d\r\n", slotString[slotType], slotId);
    return slotId;
}

/* Function to return slotId number to available list */
void atmel_ecc_free(int slotId) {
    ATMEL_ENTRY_LOG();
    return;
}

/**
 * \brief Callback function for getting the current encryption key
 */
int atmel_get_enc_key_default(byte *enckey, word16 keysize) {
    ATMEL_ENTRY_LOG();
    if (enckey == NULL || keysize != ATECC_KEY_SIZE) {
        return BAD_FUNC_ARG;
    }

    XMEMSET(enckey, 0xFF, keysize); /* use default value */

    return 0;
}

/**
 * \brief Write enc key before.
 */
static int atmel_init_enc_key(void) {
    ATMEL_ENTRY_LOG();
    int ret;
    uint8_t read_key[ATECC_KEY_SIZE];
    uint8_t writeBlock = 0;
    uint8_t writeOffset = 0;
    int slotId;

    slotId = atmel_ecc_alloc(ATMEL_SLOT_ENCKEY);

    /* check for encryption key slotId */
    if (slotId == ATECC_INVALID_SLOT)
        return BAD_FUNC_ARG;

    /* get encryption key */
    ATECC_GET_ENC_KEY(read_key, sizeof (read_key));
    ATMEL_DEBUG_LOG("    calling atcab_write_zone with slotId: %d\r\n", slotId);
    ret = atcab_write_zone(ATCA_ZONE_DATA, slotId, writeBlock, writeOffset,
            read_key, ATCA_BLOCK_SIZE);
    ForceZero(read_key, sizeof (read_key));
    ret = atmel_ecc_translate_err(ret);

    return ret;
}

int atmel_get_rev_info(word32 *revision) {
    ATMEL_ENTRY_LOG();
    int ret;
    ret = atcab_info((uint8_t *) revision);
    ret = atmel_ecc_translate_err(ret);
    return ret;
}

void atmel_show_rev_info(void) {
    ATMEL_ENTRY_LOG();
#ifdef WOLFSSL_ATECC508A_DEBUG
    word32 revision = 0;
    atmel_get_rev_info(&revision);
    char displayStr[15];
    size_t displen = sizeof (displayStr);
    atcab_bin2hex((uint8_t *) & revision, 4, displayStr, &displen);

    ATMEL_DEBUG_LOG("    ATECC608A Revision: %s\r\n", displayStr);
#endif
}

int atmel_ecc_create_pms(int slotId, const uint8_t *peerKey, uint8_t *pms) {
    ATMEL_ENTRY_LOG();
    ATMEL_DEBUG_LOG("    atmel_ecc_create_pms called with slot id : %d\r\n", slotId);
    int ret;
    uint8_t read_key[ATECC_KEY_SIZE];
    int slotIdEnc;

    slotIdEnc = atmel_ecc_alloc(ATMEL_SLOT_ECDHE_ENC);
    if (slotIdEnc == ATECC_INVALID_SLOT)
        return BAD_FUNC_ARG;

    /* get encryption key */
    ATECC_GET_ENC_KEY(read_key, sizeof (read_key));

    /* send the encrypted version of the ECDH command */
    //const uint8_t num_in[NONCE_NUMIN_SIZE]={0}; //should use rand

#ifdef WOLFSSL_ATECC508A_DEBUG
    ATMEL_DEBUG_LOG("    about to call atcab_ecdh_ioenc with slotId: %d and slotIdEnc: %d\r\n", slotId, slotIdEnc);

    char displayStr[256];
    size_t displen = sizeof (displayStr);
    atcab_bin2hex(peerKey, 64, displayStr, &displen);
    ATMEL_DEBUG_LOG("    peerKey: %s\r\n", displayStr);

    displen = sizeof (displayStr);
    atcab_bin2hex(read_key, 32, displayStr, &displen);
    ATMEL_DEBUG_LOG("    readKey: %s\r\n", displayStr);
#endif
    //use new ECC608 API. atcab_ecdh_enc used in 508 will fail
    ret = atcab_ecdh_ioenc(slotId, peerKey, pms, read_key);
    //ret = atcab_ecdh_enc(slotId, peerKey, pms, read_key, slotIdEnc,num_in);
    ret = atmel_ecc_translate_err(ret);


    /* free the ECDHE slot */
    atmel_ecc_free(slotIdEnc);

    return ret;
}

int atmel_ecc_create_key(int slotId, byte *peerKey) {
    ATMEL_ENTRY_LOG();
    int ret;

    /* verify provided slotId */
    if (slotId == ATECC_INVALID_SLOT) {
        return WC_HW_WAIT_E;
    }

    /* generate new ephemeral key on device */
    ATMEL_DEBUG_LOG("    Calling atcab_genkey with slotId: %d\r\n", slotId);
    ret = atcab_genkey(slotId, peerKey);
    ret = atmel_ecc_translate_err(ret);
    return ret;
}

int atmel_ecc_sign(int slotId, const byte *message, byte *signature) {
    ATMEL_ENTRY_LOG();
    int ret;

    ATMEL_DEBUG_LOG("    Calling atcab_sign with slotId: %d\r\n", slotId);
    ret = atcab_sign(slotId, message, signature);
    ret = atmel_ecc_translate_err(ret);
    return ret;
}

int atmel_ecc_verify(const byte *message, const byte *signature,
        const byte *pubkey, int *verified) {
    ATMEL_ENTRY_LOG();
    int ret;

    ret = atcab_verify_extern(message, signature, pubkey, (bool *) verified);
    ret = atmel_ecc_translate_err(ret);
    return ret;
}

#endif /* WOLFSSL_ATECC508A */

int atmel_init(void) {
    ATMEL_ENTRY_LOG();
    int ret = 0;

#ifdef WOLFSSL_ATECC508A
    if (!mAtcaInitDone) {
        ATCA_STATUS status;

#ifndef SINGLE_THREADED
        wc_InitMutex(&mSlotMutex);
#endif

        /* Setup the hardware interface */

        /* Initialize the CryptoAuthLib to communicate with ATECC508A */
        atcab_release();
        atcab_wakeup();
        status = atcab_init(&atecc608_0_init_data);
        if (status != ATCA_SUCCESS) {
            SYS_CONSOLE_PRINT("    Failed to initialize atcab\r\n");
            return WC_HW_E;
        }

        /* show revision information */
        atmel_show_rev_info();

#ifdef WOLFSSL_ATECC508A_TLS
        /* Configure the ECC508 for use with TLS API functions */
        //        device_init_default();
#endif

        /* Init the I2C pipe encryption key. */
        /* Value is generated/stored during pair for the ATECC508A and stored
            on micro flash */
        /* For this example its a fixed value */

        if (atmel_init_enc_key() != 0) {
            SYS_CONSOLE_PRINT("    TNGinit: Failed to initialize transport key\r\n");
            return WC_HW_E;
        }
        mAtcaInitDone = 1;
        //register certs to wolf cert store.
    }
#endif /* WOLFSSL_ATECC508A */
    return ret;
}

void atmel_finish(void) {
    ATMEL_ENTRY_LOG();
#ifdef WOLFSSL_ATECC508A
    if (mAtcaInitDone) {
        atcab_release();

#ifndef SINGLE_THREADED
        wc_FreeMutex(&mSlotMutex);
#endif

        mAtcaInitDone = 0;
    }
#endif
}

/* Reference PK Callbacks */
#ifdef HAVE_PK_CALLBACKS

/**
 * \brief Used on the server-side only for creating the ephemeral key for ECDH
 */
int atcatls_create_key_cb(WOLFSSL *ssl, ecc_key *key, unsigned int keySz,
        int ecc_curve, void *ctx) {
    ATMEL_ENTRY_LOG();
    int ret;
    uint8_t peerKey[ATECC_PUBKEY_SIZE];
    uint8_t *qx = &peerKey[0];
    uint8_t *qy = &peerKey[ATECC_PUBKEY_SIZE / 2];
    int slotId;

    (void) ssl;
    (void) ctx;

    /* ATECC508A only supports P-256 */
    if (ecc_curve == ECC_SECP256R1) {
        atcab_wakeup();
        slotId = atmel_ecc_alloc(ATMEL_SLOT_ECDHE);
        if (slotId == ATECC_INVALID_SLOT)
            return WC_HW_WAIT_E;

        /* generate new ephemeral key on device */
        ret = atmel_ecc_create_key(slotId, peerKey);

        /* load generated ECC508A public key into key, used by wolfSSL */
        if (ret == 0) {
            ret = wc_ecc_import_unsigned(key, qx, qy, NULL, ECC_SECP256R1);
        }

        if (ret == 0) {
            key->slot = slotId;
        } else {
            atmel_ecc_free(slotId);
            ATMEL_DEBUG_LOG("    atcatls_create_key_cb: ret %d\r\n", ret);
        }
    } else {
#ifndef WOLFSSL_ATECC508A_NOSOFTECC
        /* use software for non P-256 cases */
        WC_RNG rng;
        ret = wc_InitRng(&rng);
        if (ret == 0) {
            ret = wc_ecc_make_key_ex(&rng, keySz, key, ecc_curve);
            wc_FreeRng(&rng);
        }
#else
        ret = NOT_COMPILED_IN;
#endif /* !WOLFSSL_ATECC508A_NOSOFTECC */
    }
    return ret;
}

/**
 * \brief Creates a shared secret using a peer public key and a device key
 */
int atcatls_create_pms_cb(WOLFSSL *ssl, ecc_key *otherKey,
        unsigned char *pubKeyDer, word32 *pubKeySz,
        unsigned char *out, word32 *outlen,
        int side, void *ctx) {
    ATMEL_ENTRY_LOG();
    ATMEL_DEBUG_LOG("    processing pms CB as side %d\r\n", side);
    int ret;
    ecc_key tmpKey;
    uint8_t peerKeyBuf[ATECC_PUBKEY_SIZE];
    uint8_t *peerKey = peerKeyBuf;
    uint8_t *qx = &peerKey[0];
    uint8_t *qy = &peerKey[ATECC_PUBKEY_SIZE / 2];
    word32 qxLen = ATECC_PUBKEY_SIZE / 2, qyLen = ATECC_PUBKEY_SIZE / 2;

    if (pubKeyDer == NULL || pubKeySz == NULL || out == NULL || outlen == NULL) {
        ATMEL_DEBUG_LOG("    Bad Argument passed\r\n");
        return BAD_FUNC_ARG;
    }

    (void) ssl;
    (void) ctx;
    (void) otherKey;

    ret = wc_ecc_init(&tmpKey);
    if (ret != 0) {
        ATMEL_DEBUG_LOG("    wc_ecc_init failed(%d)\r\n",ret);
        return ret;
    }

    /* ATECC508A only supports P-256 */
    ATMEL_DEBUG_LOG("    keyType=%d\r\n",otherKey->dp->id);
    if (otherKey->dp->id == ECC_SECP256R1) {
        atcab_wakeup();
        XMEMSET(peerKey, 0, ATECC_PUBKEY_SIZE);

        /* for client: create and export public key */
        if (side == WOLFSSL_CLIENT_END) {
            int slotId = atmel_ecc_alloc(ATMEL_SLOT_ECDHE);
            if (slotId == ATECC_INVALID_SLOT)
                return WC_HW_WAIT_E;
            tmpKey.slot = slotId;

            /* generate new ephemeral key on device */
            
            ret = atmel_ecc_create_key(slotId, peerKey);
            if (ret != ATCA_SUCCESS) {
                ATMEL_DEBUG_LOG("    atmel_ecc_create_key failed (%d)\r\n",ret);
                goto exit;
            }

            /* convert raw unsigned public key to X.963 format for TLS */
            //ATMEL_DEBUG_LOG("    calling wc_ecc_import_unsigned\r\n");
            ret = wc_ecc_import_unsigned(&tmpKey, qx, qy, NULL, ECC_SECP256R1);
            if (ret == 0) {
                //ATMEL_DEBUG_LOG("    calling wc_ecc_export_x963\r\n");
                ret = wc_ecc_export_x963(&tmpKey, pubKeyDer, pubKeySz);
            }

            /* export peer's key as raw unsigned for hardware */
            if (ret == 0) {
                //ATMEL_DEBUG_LOG("    calling wc_ecc_export_public_raw\r\n");
                ret = wc_ecc_export_public_raw(otherKey, qx, &qxLen, qy, &qyLen);
            }
        }/* for server: import public key */
        else if (side == WOLFSSL_SERVER_END) {
            tmpKey.slot = otherKey->slot;

            /* import peer's key and export as raw unsigned for hardware */
            ret = wc_ecc_import_x963_ex(pubKeyDer, *pubKeySz, &tmpKey, ECC_SECP256R1);
            if (ret == 0) {
                ret = wc_ecc_export_public_raw(&tmpKey, qx, &qxLen, qy, &qyLen);
            }
        } else {
            ret = BAD_FUNC_ARG;
        }

        if (ret != 0) {
            goto exit;
        }

        ret = atmel_ecc_create_pms(tmpKey.slot, peerKey, out);
        *outlen = ATECC_KEY_SIZE;

#ifndef WOLFSSL_ATECC508A_NOIDLE
        /* put chip into idle to prevent watchdog situation on chip */
        atcab_idle();
#endif

        (void) qxLen;
        (void) qyLen;
    } else {
#ifndef WOLFSSL_ATECC508A_NOSOFTECC
        /* use software for non P-256 cases */
        ecc_key *privKey = NULL;
        ecc_key *pubKey = NULL;

        /* for client: create and export public key */
        if (side == WOLFSSL_CLIENT_END) {
            WC_RNG rng;
            privKey = &tmpKey;
            pubKey = otherKey;

            ret = wc_InitRng(&rng);
            if (ret == 0) {
                ret = wc_ecc_make_key_ex(&rng, 0, privKey, otherKey->dp->id);
                if (ret == 0) {
                    ret = wc_ecc_export_x963(privKey, pubKeyDer, pubKeySz);
                }
                wc_FreeRng(&rng);
            }
        }/* for server: import public key */
        else if (side == WOLFSSL_SERVER_END) {
            privKey = otherKey;
            pubKey = &tmpKey;

            ret = wc_ecc_import_x963_ex(pubKeyDer, *pubKeySz, pubKey,
                    otherKey->dp->id);
        } else {
            ret = BAD_FUNC_ARG;
        }

        /* generate shared secret and return it */
        if (ret == 0) {
            ret = wc_ecc_shared_secret(privKey, pubKey, out, outlen);
        }
#else
        ret = NOT_COMPILED_IN;
#endif /* !WOLFSSL_ATECC508A_NOSOFTECC */
    }

exit:
    wc_ecc_free(&tmpKey);


    if (ret != 0) {
        ATMEL_DEBUG_LOG("   atcab_ecdh_ioenc: ret %d\r\n", ret);
    }

    return ret;
}

/**
 * \brief Sign received digest using private key on device
 */
int atcatls_sign_certificate_cb(WOLFSSL *ssl, const byte *in, unsigned int inSz,
        byte *out, word32 *outSz, const byte *key, unsigned int keySz, void *ctx) {
    ATMEL_ENTRY_LOG();
    int ret;
    byte sigRs[ATECC_SIG_SIZE];
    int slotId;

    (void) ssl;
    (void) inSz;
    (void) key;
    (void) keySz;
    (void) ctx;

    if (in == NULL || out == NULL || outSz == NULL) {
        return BAD_FUNC_ARG;
    }
    atcab_wakeup();
    slotId = atmel_ecc_alloc(ATMEL_SLOT_DEVICE);
    if (slotId == ATECC_INVALID_SLOT)
        return WC_HW_WAIT_E;

    /* We can only sign with P-256 */
    ret = atmel_ecc_sign(slotId, in, sigRs);
    if (ret != ATCA_SUCCESS) {
        ATMEL_DEBUG_LOG("    atmel_ecc_sign failed\r\n");
        ret = WC_HW_E;
        goto exit;
    }

#ifndef WOLFSSL_ATECC508A_NOIDLE
    /* put chip into idle to prevent watchdog situation on chip */
    atcab_idle();
#endif

    /* Encode with ECDSA signature */
    ret = wc_ecc_rs_raw_to_sig(
            &sigRs[0], ATECC_SIG_SIZE / 2,
            &sigRs[ATECC_SIG_SIZE / 2], ATECC_SIG_SIZE / 2,
            out, outSz);
    if (ret != 0) {
        ATMEL_DEBUG_LOG("    wc_ecc_rs_raw_to_sig failed\r\n");
        goto exit;
    }

exit:

    atmel_ecc_free(slotId);

    if (ret != 0) {
        ATMEL_DEBUG_LOG("    atcatls_sign_certificate_cb: ret %d\r\n", ret);
    }

    return ret;
}

/**
 * \brief Verify signature received from peers to prove peer's private key.
 */
int atcatls_verify_signature_cb(WOLFSSL *ssl, const byte *sig, unsigned int sigSz,
        const byte *hash, unsigned int hashSz, const byte *key, unsigned int keySz, int *result,
        void *ctx) {
    ATMEL_ENTRY_LOG();
    int ret;
    ecc_key tmpKey;
    word32 idx = 0;
    uint8_t peerKey[ATECC_PUBKEY_SIZE];
    uint8_t *qx = &peerKey[0];
    uint8_t *qy = &peerKey[ATECC_PUBKEY_SIZE / 2];
    word32 qxLen = ATECC_PUBKEY_SIZE / 2, qyLen = ATECC_PUBKEY_SIZE / 2;
    byte sigRs[ATECC_SIG_SIZE];
    word32 rSz = ATECC_SIG_SIZE / 2;
    word32 sSz = ATECC_SIG_SIZE / 2;

    (void) sigSz;
    (void) hashSz;
    (void) ctx;

    if (ssl == NULL || key == NULL || sig == NULL || hash == NULL || result == NULL) {
        return BAD_FUNC_ARG;
    }

    /* import public key */
    ret = wc_ecc_init(&tmpKey);
    if (ret == 0) {
        ret = wc_EccPublicKeyDecode(key, &idx, &tmpKey, keySz);
    }
    if (ret != 0) {
        goto exit;
    }

    if (tmpKey.dp->id == ECC_SECP256R1) {
        atcab_wakeup();
        /* export public as unsigned bin for hardware */
        ret = wc_ecc_export_public_raw(&tmpKey, qx, &qxLen, qy, &qyLen);
        wc_ecc_free(&tmpKey);

        /* decode the ECDSA signature */
        ret = wc_ecc_sig_to_rs(sig, sigSz,
                &sigRs[0], &rSz,
                &sigRs[ATECC_SIG_SIZE / 2], &sSz);
        if (ret != 0) {
            goto exit;
        }

        ret = atmel_ecc_verify(hash, sigRs, peerKey, result);
        if (ret != ATCA_SUCCESS || !*result) {
            ret = WC_HW_E;
            goto exit;
        }

#ifndef WOLFSSL_ATECC508A_NOIDLE
        /* put chip into idle to prevent watchdog situation on chip */
        atcab_idle();
#endif
    } else {
#ifndef WOLFSSL_ATECC508A_NOSOFTECC
        ret = wc_ecc_verify_hash(sig, sigSz, hash, hashSz, result, &tmpKey);
#else
        ret = NOT_COMPILED_IN;
#endif /* !WOLFSSL_ATECC508A_NOSOFTECC */
    }

    (void) rSz;
    (void) sSz;
    (void) qxLen;
    (void) qyLen;

    ret = 0; /* success */

exit:

    if (ret != 0) {
        ATMEL_DEBUG_LOG("    atcatls_verify_signature_cb: ret %d\r\n", ret);
    }

    return ret;
}

static int atcatls_set_certificates(WOLFSSL_CTX *ctx) {
    int ret = 0;
    ATMEL_ENTRY_LOG();
    ATCA_STATUS status;

    ATMEL_DEBUG_LOG("    getting device certs\r\n");
/*
const char* device_cert= "-----BEGIN CERTIFICATE-----\n"
"MIICHjCCAcWgAwIBAgIQRzDeBDxczfs6wtmOGxLLcDAKBggqhkjOPQQDAjBPMSEw\n"
"HwYDVQQKDBhNaWNyb2NoaXAgVGVjaG5vbG9neSBJbmMxKjAoBgNVBAMMIUNyeXB0\n"
"byBBdXRoZW50aWNhdGlvbiBTaWduZXIgMjcyMDAgFw0xOTEyMTAyMDAwMDBaGA8y\n"
"MDQ3MTIxMDIwMDAwMFowQjEhMB8GA1UECgwYTWljcm9jaGlwIFRlY2hub2xvZ3kg\n"
"SW5jMR0wGwYDVQQDDBRzbjAxMjNBQTI0QkVGNzk4M0QwMTBZMBMGByqGSM49AgEG\n"
"CCqGSM49AwEHA0IABDXCOfMFIn5QA73DZBMB5xSNLlI6WH920o+e9cYRhtOe1V0m\n"
"YP829MJtOLGIYbPI5F6NIvuMfTIbvJwaTAQ6UYSjgY0wgYowKgYDVR0RBCMwIaQf\n"
"MB0xGzAZBgNVBAUTEmV1aTQ4XzY4MjcxOTRBQTk0NTAMBgNVHRMBAf8EAjAAMA4G\n"
"A1UdDwEB/wQEAwIDiDAdBgNVHQ4EFgQUd8GVFiep9UhdS7+4dxfAsCYE554wHwYD\n"
"VR0jBBgwFoAUKrerI8TiU+jR3cvMuSfRtJDhm6cwCgYIKoZIzj0EAwIDRwAwRAIg\n"
"Oq4yeXfZtVb8VGOZuH3J+3iK/hquKSo0qj+K9TqYqyYCIBfUeKMJdjrAaAB1JpTm\n"
"xXXOFoYShnColbvA4srKAnrZ\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN CERTIFICATE-----\n"
"MIICBDCCAaqgAwIBAgIQZ7676YoNv9Em0LhATDwXYTAKBggqhkjOPQQDAjBPMSEw\n"
"HwYDVQQKDBhNaWNyb2NoaXAgVGVjaG5vbG9neSBJbmMxKjAoBgNVBAMMIUNyeXB0\n"
"byBBdXRoZW50aWNhdGlvbiBSb290IENBIDAwMjAgFw0xODEyMTQyMDAwMDBaGA8y\n"
"MDQ5MTIxNDIwMDAwMFowTzEhMB8GA1UECgwYTWljcm9jaGlwIFRlY2hub2xvZ3kg\n"
"SW5jMSowKAYDVQQDDCFDcnlwdG8gQXV0aGVudGljYXRpb24gU2lnbmVyIDI3MjAw\n"
"WTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATmVSoWJPK+gvAgQ+d+DR0vB3UbIAf2\n"
"gLGWsFXvmg+1GKmqZ1RIJrRg1zsyTDTtXbVlaHHSCZV+oKHZ46cbgfEAo2YwZDAO\n"
"BgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUKrer\n"
"I8TiU+jR3cvMuSfRtJDhm6cwHwYDVR0jBBgwFoAUeu19bca3eJ2yOAGl6EqMsKQO\n"
"KowwCgYIKoZIzj0EAwIDSAAwRQIgbk0O1mwHyXokKIQRjUbrfQ9T486kB6sRfs9V\n"
"z/dfRS8CIQDxXODOVHjmvEsFSdf9oKp8U8L3RtM1lEROi1UtFX5OQA==\n"
"-----END CERTIFICATE-----\n";
  */  
    /*Read signer cert*/
    size_t signerCertSize = 0;
    status = tng_atcacert_max_signer_cert_size(&signerCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    tng_atcacert_max_signer_cert_size Failed \r\n");
        ret = atmel_ecc_translate_err(ret);
        return ret;
    }
    ATMEL_DEBUG_LOG("    signer Cert size is %d\r\n", signerCertSize);
    uint8_t signerCert[signerCertSize];
    status = tng_atcacert_read_signer_cert((uint8_t*) & signerCert, &signerCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    tng_atcacert_read_signer_cert Failed \r\n");
        ret = atmel_ecc_translate_err(ret);
        return ret;
    }

    /*Read device cert signer by the signer above*/
    size_t deviceCertSize = 0;
    status = tng_atcacert_max_device_cert_size(&deviceCertSize);
    if (ATCA_SUCCESS != status) {
        SYS_CONSOLE_PRINT("    tng_atcacert_max_signer_cert_size Failed \r\n");
        ret = atmel_ecc_translate_err(ret);
        return ret;
    }
    ATMEL_DEBUG_LOG("    device Cert size is %d\r\n", deviceCertSize);
    uint8_t deviceCert[deviceCertSize];
    status = tng_atcacert_read_device_cert((uint8_t*) & deviceCert, &deviceCertSize, (uint8_t*) & signerCert);
    if (ATCA_SUCCESS != status) {
        ret = atmel_ecc_translate_err(ret);
        SYS_CONSOLE_PRINT("    tng_atcacert_read_device_cert Failed (%x) \r\n", status);
        return ret;
    }
    /*Generate a PEM chain for device certificate.*/
    byte devPem[1024];
    byte signerPem[1024];
    XMEMSET(devPem, 0, 1024);
    XMEMSET(signerPem, 0, 1024);
    int devPemSz, signerPemSz;
    
    devPemSz = wc_DerToPem(deviceCert, deviceCertSize, devPem, sizeof(devPem), CERT_TYPE);
    if((devPemSz<=0)){
        SYS_CONSOLE_PRINT("    Failed converting device Cert to PEM (%d)\r\n",devPemSz);
        return devPemSz;
    }
    
    signerPemSz = wc_DerToPem(signerCert, signerCertSize, signerPem, sizeof(signerPem), CERT_TYPE);
    if((signerPemSz<=0)){
        SYS_CONSOLE_PRINT("    Failed converting signer Cert to PEM (%d)\r\n",signerPemSz);
        return signerPemSz;
    }
    
    char devCertChain[devPemSz+signerPemSz];
    
    strncat(devCertChain,(char*)devPem,devPemSz);
    strncat(devCertChain,(char*)signerPem,signerPemSz);
    ATMEL_DEBUG_LOG("    device Cert Chain : \r\n%s\r\n", devCertChain);
    
    ret=wolfSSL_CTX_use_certificate_chain_buffer(ctx,(const unsigned char*)devCertChain,strlen(devCertChain));
    //ret = wolfSSL_CTX_use_certificate_buffer(ctx, deviceCert, deviceCertSize, SSL_FILETYPE_ASN1);
    if (ret != SSL_SUCCESS) {
        SYS_CONSOLE_PRINT("    ATCA: error loading certificate from buffer (%d) \r\n", ret);
        ret=-1;
    }
    else ret=0;
    ATMEL_DEBUG_LOG("    device Cert committed\r\n");
    return ret;
}

int atcatls_set_callbacks(WOLFSSL_CTX *ctx) {
    ATMEL_ENTRY_LOG();
    int ret;
    wolfSSL_CTX_SetEccKeyGenCb(ctx, atcatls_create_key_cb);
    wolfSSL_CTX_SetEccVerifyCb(ctx, atcatls_verify_signature_cb);
    wolfSSL_CTX_SetEccSignCb(ctx, atcatls_sign_certificate_cb);
    wolfSSL_CTX_SetEccSharedSecretCb(ctx, atcatls_create_pms_cb);
#ifdef WOLFSSL_TNG_DEVCERT_REGISTER
    ret=atcatls_set_certificates(ctx);
    if(0!=ret){
        SYS_CONSOLE_PRINT("    atcatls_set_certificates failed. (%d) \r\n",ret);
    }
#endif
    return 0;
}

int atcatls_set_callback_ctx(WOLFSSL *ssl, void *user_ctx) {
    ATMEL_ENTRY_LOG();
    wolfSSL_SetEccKeyGenCtx(ssl, user_ctx);
    wolfSSL_SetEccVerifyCtx(ssl, user_ctx);
    wolfSSL_SetEccSignCtx(ssl, user_ctx);
    wolfSSL_SetEccSharedSecretCtx(ssl, user_ctx);
    return 0;
}

#endif /* HAVE_PK_CALLBACKS */

#endif

#endif /* WOLFSSL_ATMEL || WOLFSSL_ATECC508A || WOLFSSL_ATECC_PKCB */

