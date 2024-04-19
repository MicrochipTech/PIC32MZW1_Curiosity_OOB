/*******************************************************************************
  PIC32MZW Driver Authentication Context Implementation

  File Name:
    wdrv_pic32mzw_authctx.c

  Summary:
    PIC32MZW wireless driver authentication context implementation.

  Description:
    This interface manages the authentication contexts which 'wrap' the state
      associated with authentication schemes.
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
#include <string.h>

#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_authctx.h"
#include "drv_pic32mzw1.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Authentication Context Global Data
// *****************************************************************************
// *****************************************************************************

DRV_PIC32MZW_11I_MASK DRV_PIC32MZW_Get11iMask
(
    WDRV_PIC32MZW_AUTH_TYPE authType,
    WDRV_PIC32MZW_AUTH_MOD_MASK authMod
);

//*******************************************************************************
/*
  Function:
    bool _DRV_PIC32MZW_WEPKeyIsValid
    (
        uint8_t idx,
        uint8_t *const pKey,
        uint8_t size
    )

  Summary:
    Checks if WEP key is valid.

  Description:
    Determines if the WEP key, index and size are valid.

  Precondition:
    None.

  Parameters:
    idx  - Key index.
    pKey - Pointer to key.
    size - Size of key.

  Returns:
    true or false indicating if WEP key information is valid.

*/

static bool _DRV_PIC32MZW_WEPKeyIsValid
(
    uint8_t idx,
    uint8_t *const pKey,
    uint8_t size
)
{
    /* Check index. Index values 1-4 is only allowed*/
    if ((idx < 1) || (idx > 4))
    {
        return false;
    }
    /* Check key. */
    if (NULL == pKey)
    {
        return false;
    }
    /* Check size. */
    if (
            (WDRV_PIC32MZW_WEP_40_KEY_STRING_SIZE != size)
        &&  (WDRV_PIC32MZW_WEP_104_KEY_STRING_SIZE != size)
    )
    {
        return false;
    }
    return true;
}

//*******************************************************************************
/*
  Function:
    bool _DRV_PIC32MZW_PersonalKeyIsValid
    (
        uint8_t *const pPassword,
        uint8_t size,
        DRV_PIC32MZW_11I_MASK dot11iInfo
    )

  Summary:
    Checks if personal key is valid.

  Description:
    Determines if the personal key and size are valid.

  Precondition:
    None.

  Parameters:
    pPassword  - Pointer to personal key.
    size       - Size of personal key.
    dot11iInfo - .11i information.

  Returns:
    true or false indicating if personal key information is valid.

*/

static bool _DRV_PIC32MZW_PersonalKeyIsValid
(
    uint8_t *const pPassword,
    uint8_t size,
    DRV_PIC32MZW_11I_MASK dot11iInfo
)
{
    /* Check password is present. */
    if (NULL == pPassword)
    {
        return false;
    }

    /* If password is to be used for SAE, we place the same upper limit on
     * length as for PSK passphrases. Note this is an implementation-specific
     * restriction, not an 802.11 (2016) restriction. */
    if (dot11iInfo & DRV_PIC32MZW_11I_SAE)
    {
        if (WDRV_PIC32MZW_MAX_PSK_PASSWORD_LEN < size)
        {
            return false;
        }
    }

    if (dot11iInfo & DRV_PIC32MZW_11I_PSK)
    {
        /* Determine if this is a password or direct PSK. */
        if (WDRV_PIC32MZW_PSK_LEN == size)
        {
            /* PSK. */
            while (size--)
            {
                char character = (char)(pPassword[size]);

                /* Each character must be in the range '0-9', 'A-F' or 'a-f'. */
                if (
                        (('0' > character) || ('9' < character))
                    &&  (('A' > character) || ('F' < character))
                    &&  (('a' > character) || ('f' < character))
                )
                {
                    return false;
                }
            }
        }
        else
        {
            /* Password. */
            /* Check password size. */
            if (
                    (WDRV_PIC32MZW_MAX_PSK_PASSWORD_LEN < size)
                ||  (WDRV_PIC32MZW_MIN_PSK_PASSWORD_LEN > size)
            )
            {
                return false;
            }

            /* Each character must be in the range 0x20 to 0x7e. */
            while (size--)
            {
                char character = (char)(pPassword[size]);

                if ((0x20 > character) || (0x7e < character))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_AuthCtxIsValid(const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx)

  Summary:
    Tests if an authentication context is valid.

  Description:
    Tests the elements of the authentication context to judge if their values are legal.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.

*/

bool WDRV_PIC32MZW_AuthCtxIsValid(const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return false;
    }

    switch (pAuthCtx->authType)
    {
        /* Nothing to check for Open authentication. */
        case WDRV_PIC32MZW_AUTH_TYPE_OPEN:
        {
            break;
        }

        /* Check WEP authentication. */
        case WDRV_PIC32MZW_AUTH_TYPE_WEP:
        {
            if (false == _DRV_PIC32MZW_WEPKeyIsValid(
                    pAuthCtx->authInfo.WEP.idx,
                    (uint8_t *const)(pAuthCtx->authInfo.WEP.key),
                    pAuthCtx->authInfo.WEP.size
            ))
            {
                return false;
            }
            break;
        }

        /* Check Personal authentication. */
        case WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL:
        case WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL:
#ifdef WDRV_PIC32MZW_WPA3_PERSONAL_SUPPORT
        case WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL:
        case WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL:
#endif
        {
            if (false == _DRV_PIC32MZW_PersonalKeyIsValid(
                    (uint8_t *const)(pAuthCtx->authInfo.personal.password),
                    pAuthCtx->authInfo.personal.size,
                    DRV_PIC32MZW_Get11iMask(pAuthCtx->authType, pAuthCtx->authMod)
            ))
            {
                return false;
            }
            break;
        }
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
        case WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_ENTERPRISE:
        case WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE:
        case WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE:
        case WDRV_PIC32MZW_AUTH_TYPE_WPA3_ENTERPRISE:
        {
            int size = 0, serverDomainLen = 0;
            /* validate enterprise related parameters */
            if (WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID == pAuthCtx->authInfo.enterprise.phase1.tlsCtxHandle)
            {
                return false;
            }
            size = strlen(pAuthCtx->authInfo.enterprise.phase1.identity);
            if ((0 == size) || (size > WDRV_PIC32MZW_ENT_AUTH_IDENTITY_LEN_MAX))
            {
                return false;
            }
            serverDomainLen = strlen(pAuthCtx->authInfo.enterprise.phase1.serverDomainName);
            if (serverDomainLen > WDRV_PIC32MZW_ENT_AUTH_SERVER_DOMAIN_LEN_MAX)
            {
                return false;
            }

            
            if (WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTTLSv0_MSCHAPv2 == pAuthCtx->authInfo.enterprise.auth1xMethod)
            {
                /* username and password is mandatory for MSCHAPv2 authentication */
                size = strlen(pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.username);
                if (size > WDRV_PIC32MZW_ENT_AUTH_USERNAME_LEN_MAX)
                {
                    return false;
                }
                size = strlen(pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.password);
                if (size > WDRV_PIC32MZW_ENT_AUTH_PASSWORD_LEN_MAX)
                {
                    return false;
                }
            }
#ifndef WDRV_PIC32MZW_AUTH_WPA3_ENTERPRISE_FQDN_OPTIONAL
            if ((WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE == pAuthCtx->authType) ||
                    (WDRV_PIC32MZW_AUTH_TYPE_WPA3_ENTERPRISE == pAuthCtx->authType))
            {
                /* Server domain is mandatory for WPA2WPA3 and WPA3 enterprise */
                if (0 == serverDomainLen)
                {
                    return false;
                }
            }
#endif
            break;
        }

#endif
        default:
        {
            /* Unknown authentication scheme. */
            return false;
        }
    }

    return true;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetDefaults
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx
    )

  Summary:
    Configures an authentication context into a default state.

  Description:
    Ensures that each element of the structure is configured into a default state.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetDefaults
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure authentication type is a known invalid type. */
    pAuthCtx->authType = WDRV_PIC32MZW_AUTH_TYPE_DEFAULT;

    /* Initialise auth modifiers to 0. */
    pAuthCtx->authMod = WDRV_PIC32MZW_AUTH_MOD_NONE;

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetOpen
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx
    )

  Summary:
    Configure an authentication context for Open authentication.

  Description:
    The auth type and information are configured appropriately for Open
      authentication.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetOpen
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Set authentication type to Open. */
    pAuthCtx->authType = WDRV_PIC32MZW_AUTH_TYPE_OPEN;

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetWEP
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
        uint8_t idx,
        uint8_t *pKey,
        uint8_t size
    )

  Summary:
    Configure an authentication context for WEP authentication.

  Description:
    The auth type and information are configured appropriately for WEP
      authentication.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetWEP
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    uint8_t idx,
    uint8_t *const pKey,
    uint8_t size
)
{
    /* Ensure authentication context is valid. */
    if ((NULL == pAuthCtx) || (NULL == pKey))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the index and key are valid. */
    if (false == _DRV_PIC32MZW_WEPKeyIsValid(idx, pKey, size))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Set authentication type to WEP. */
    pAuthCtx->authType = WDRV_PIC32MZW_AUTH_TYPE_WEP;

    /* Initialise Shared Key authentication to disabled.                     */
    /* The Application may enable Shared Key later if desired via            */
    /* WDRV_PIC32MZW_AuthCtxSharedKey.                                       */
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_SHARED_KEY;

    /* Set key index and size. */
    pAuthCtx->authInfo.WEP.idx  = idx;
    pAuthCtx->authInfo.WEP.size = size;

    /* Copy WEP key and ensure zero terminated. */
    memcpy(&pAuthCtx->authInfo.WEP.key, pKey, size);
    pAuthCtx->authInfo.WEP.key[size] = '\0';

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetPersonal
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
        uint8_t *pPassword,
        uint8_t size,
        WDRV_PIC32MZW_AUTH_TYPE authType
    )

  Summary:
    Configure an authentication context for WPA personal authentication.

  Description:
    The auth type and information are configured appropriately for personal
      authentication with the password or PSK provided. The Management Frame
      Protection configuration is initialised to WDRV_PIC32MZW_AUTH_MFP_ENABLED.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetPersonal
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    uint8_t *const pPassword,
    uint8_t size,
    WDRV_PIC32MZW_AUTH_TYPE authType
)
{
    DRV_PIC32MZW_11I_MASK dot11iInfo;

    /* Ensure authentication context is valid. */
    if ((NULL == pAuthCtx) || (NULL == pPassword))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    if (WDRV_PIC32MZW_AUTH_TYPE_DEFAULT == authType)
    {
#ifdef WDRV_PIC32MZW_WPA3_PERSONAL_SUPPORT
        /* Set authentication type to WPA2/WPA3 transition mode. */
        authType = WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL;
#else
        /* Set authentication type to WPA2. */
        authType = WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL;
#endif
    }

    dot11iInfo = DRV_PIC32MZW_Get11iMask(authType, WDRV_PIC32MZW_AUTH_MOD_NONE);

    /* Ensure the requested auth type is valid for Personal authentication. */
    if (!(dot11iInfo & (DRV_PIC32MZW_11I_PSK | DRV_PIC32MZW_11I_SAE)))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Ensure the password is valid. */
    if (false == _DRV_PIC32MZW_PersonalKeyIsValid(pPassword, size, dot11iInfo))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Set authentication type. */
    pAuthCtx->authType = authType;

    /* Initialise the MFP configuration to WDRV_PIC32MZW_AUTH_MFP_ENABLED.   */
    /* The Application may change the configuration later if desired via     */
    /* WDRV_PIC32MZW_AuthCtxConfigureMfp.                                    */
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_REQ;
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_OFF;

    /* Initialise the TD configuration to not enabled.                       */
    /* The Application may change the configuration later if desired via     */
    /* WDRV_PIC32MZW_AuthCtxApTransitionDisable or                           */
    /* WDRV_PIC32MZW_AuthCtxStaTransitionDisable.                            */
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_AP_TD;
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_STA_TD;

    /* Copy the key and zero out unused parts of the buffer. */
    pAuthCtx->authInfo.personal.size = size;
    memset( pAuthCtx->authInfo.personal.password,
            0,
            sizeof(pAuthCtx->authInfo.personal.password));
    memcpy(pAuthCtx->authInfo.personal.password, pPassword, size);

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxConfigureMfp
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
        WDRV_PIC32MZW_AUTH_MFP_CONFIG config
    )

  Summary:
    Set the Management Frame Protection configuration of an authentication
      context.

  Description:
    The authentication context is updated with the Management Frame Protection
      configuration specified in the config parameter.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.
*/
WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxConfigureMfp
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    WDRV_PIC32MZW_AUTH_MFP_CONFIG config
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Update the authentication context. */
    switch (config)
    {
        case WDRV_PIC32MZW_AUTH_MFP_ENABLED:
        {
            pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_REQ;
            pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_OFF;
            break;
        }
        case WDRV_PIC32MZW_AUTH_MFP_REQUIRED:
        {
            pAuthCtx->authMod |= WDRV_PIC32MZW_AUTH_MOD_MFP_REQ;
            pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_OFF;
            break;
        }
        case WDRV_PIC32MZW_AUTH_MFP_DISABLED:
        {
            pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_REQ;
            pAuthCtx->authMod |= WDRV_PIC32MZW_AUTH_MOD_MFP_OFF;
            break;
        }
        default:
        {
            return WDRV_PIC32MZW_STATUS_INVALID_ARG;
        }
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSharedKey
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
        bool enable
    )

  Summary:
    Enable or disable Shared Key authentication in an authentication context.

  Description:
    The authentication context is updated to enable or disable Shared Key
      authentication.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.
*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSharedKey
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    bool enable
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Update the authentication context. */
    if (true == enable)
    {
        pAuthCtx->authMod |= WDRV_PIC32MZW_AUTH_MOD_SHARED_KEY;
    }
    else
    {
        pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_SHARED_KEY;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxApTransitionDisable
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
        bool tdOn,
    )

  Summary:
    Turn on/off the transition disable feature in AP mode.

  Description:
    The authentication context is updated to turn on/off the transmission of a
      transition disable element in AP mode.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.
*/
WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxApTransitionDisable
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    bool enableTD
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Update the authentication context. */
    if (true == enableTD)
    {
        pAuthCtx->authMod |= WDRV_PIC32MZW_AUTH_MOD_AP_TD;
    }
    else
    {
        pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_AP_TD;
    }

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxStaTransitionDisable
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    )

  Summary:
    Disable transition security algorithms in STA mode.

  Description:
    The authentication context is updated to disable transition security
      algorithms in STA mode.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.
*/
WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxStaTransitionDisable
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Update the authentication context. */
    pAuthCtx->authMod |= WDRV_PIC32MZW_AUTH_MOD_STA_TD;

    return WDRV_PIC32MZW_STATUS_OK;
}

#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetEnterpriseTLS
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
        const char *const pIdentity,
        WDRV_PIC32MZW_TLS_CONTEXT_HANDLE tlsCtxHandle,
        const char *const pServerDomain,
        WDRV_PIC32MZW_AUTH_TYPE authType
    )

  Summary:
    Configure an authentication context for WPA(2)-Enterprise authentication
    using enterprise auth method == WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTLS.

  Description:
    The type and state information are configured appropriately for WPA-Enterprise
    authentication.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.

*/
WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetEnterpriseTLS
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    const char *const pIdentity,
    WDRV_PIC32MZW_TLS_CONTEXT_HANDLE tlsCtxHandle,
    const char *const pServerDomain,
    WDRV_PIC32MZW_AUTH_TYPE authType
)
{
    DRV_PIC32MZW_11I_MASK dot11iInfo;
    int identityLen = 0;
    int serverDomainLen = 0;

    /* Ensure authentication context is valid. */
    if ((NULL == pAuthCtx) || (NULL == pIdentity) ||
        (WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID == tlsCtxHandle))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    identityLen = strlen(pIdentity);
    if ((0 == identityLen) || (identityLen > WDRV_PIC32MZW_ENT_AUTH_IDENTITY_LEN_MAX))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    if (WDRV_PIC32MZW_AUTH_TYPE_DEFAULT == authType)
    {
        /* Set authentication type to WPA2/WPA3 transition mode. */
        authType = WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE;
    }

    dot11iInfo = DRV_PIC32MZW_Get11iMask(authType, WDRV_PIC32MZW_AUTH_MOD_NONE);

    /* Ensure the requested auth type is valid */
    if (!(dot11iInfo & DRV_PIC32MZW_11I_1X))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    if (NULL != pServerDomain)
    {
        serverDomainLen = strlen(pServerDomain);
        if (serverDomainLen > WDRV_PIC32MZW_ENT_AUTH_SERVER_DOMAIN_LEN_MAX)
        {
            return WDRV_PIC32MZW_STATUS_INVALID_ARG;
        }
    }
#ifndef WDRV_PIC32MZW_AUTH_WPA3_ENTERPRISE_FQDN_OPTIONAL
    if ((WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE == authType) ||
            (WDRV_PIC32MZW_AUTH_TYPE_WPA3_ENTERPRISE == authType))
    {
        /* Server domain is mandatory for WPA2WPA3 and WPA3 enterprise */
        if ((NULL == pServerDomain) || (0 == serverDomainLen))
        {
            return WDRV_PIC32MZW_STATUS_INVALID_ARG;
        }
    }
#endif
    /* Set authentication type */
    pAuthCtx->authType = authType;

    /* Initialize the MFP configuration to WDRV_PIC32MZW_AUTH_MFP_ENABLED.   */
    /* The Application may change the configuration later if desired via     */
    /* WDRV_PIC32MZW_AuthCtxConfigureMfp.                                    */
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_REQ;
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_OFF;

    /* Initialise the TD configuration to not enabled.                       */
    /* The Application may change the configuration later if desired via     */
    /* WDRV_PIC32MZW_AuthCtxApTransitionDisable or                           */
    /* WDRV_PIC32MZW_AuthCtxStaTransitionDisable.                            */
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_AP_TD;
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_STA_TD;

    /* Set EAP method type */
    pAuthCtx->authInfo.enterprise.auth1xMethod = WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTLS;

    /* Set (domain-)username */
    memset(&pAuthCtx->authInfo.enterprise.phase1.identity, 0, WDRV_PIC32MZW_ENT_AUTH_IDENTITY_LEN_MAX+1);
    memcpy(&pAuthCtx->authInfo.enterprise.phase1.identity, pIdentity, identityLen);

    /* Set server domain name for validation of server cert's SAN(subject alternative name) or CN(common name) */
    memset(&pAuthCtx->authInfo.enterprise.phase1.serverDomainName, 0, WDRV_PIC32MZW_ENT_AUTH_SERVER_DOMAIN_LEN_MAX+1);
    if (NULL != pServerDomain)
    {
        memcpy(&pAuthCtx->authInfo.enterprise.phase1.serverDomainName, pServerDomain, serverDomainLen);
    }

    /* Copy the context handle */
    pAuthCtx->authInfo.enterprise.phase1.tlsCtxHandle = tlsCtxHandle;

    return WDRV_PIC32MZW_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetEnterpriseTTLSMSCHAPv2
    (
        WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
        const char *const pIdentity,
        WDRV_PIC32MZW_TLS_CONTEXT_HANDLE tlsCtxHandle,
        const char *const pServerDomain,
        const char *const pUserName,
        const char *const pPassword,
        WDRV_PIC32MZW_AUTH_TYPE authType
    )

  Summary:
    Configure an authentication context for WPA(2)-Enterprise authentication
    using enterprise method == WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTTLSv0_MSCHAPv2.

  Description:
    The type and state information are configured appropriately for WPA-Enterprise
    authentication.

  Remarks:
    See wdrv_pic32mzw_authctx.h for usage information.

*/
WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetEnterpriseTTLSMSCHAPv2
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    const char *const pIdentity,
    WDRV_PIC32MZW_TLS_CONTEXT_HANDLE tlsCtxHandle,
    const char *const pServerDomain,
    const char *const pUserName,
    const char *const pPassword,
    WDRV_PIC32MZW_AUTH_TYPE authType
)
{
    DRV_PIC32MZW_11I_MASK dot11iInfo;
    int identityLen = 0;
    int serverDomainLen = 0;
    int userNameLen = 0;
    int PasswordLen = 0;

    /* Ensure authentication context is valid. */
    if ((NULL == pAuthCtx) || (NULL == pIdentity) ||
        (WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID == tlsCtxHandle))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    identityLen = strlen(pIdentity);
    if ((0 == identityLen) || (identityLen > WDRV_PIC32MZW_ENT_AUTH_IDENTITY_LEN_MAX))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    if (WDRV_PIC32MZW_AUTH_TYPE_DEFAULT == authType)
    {
        /* Set authentication type to WPA2/WPA3 transition mode. */
        authType = WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE;
    }

    dot11iInfo = DRV_PIC32MZW_Get11iMask(authType, WDRV_PIC32MZW_AUTH_MOD_NONE);

    /* Ensure the requested auth type is valid */
    if (!(dot11iInfo & DRV_PIC32MZW_11I_1X))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    if (NULL != pServerDomain)
    {
        serverDomainLen = strlen(pServerDomain);
        if (serverDomainLen > WDRV_PIC32MZW_ENT_AUTH_SERVER_DOMAIN_LEN_MAX)
        {
            return WDRV_PIC32MZW_STATUS_INVALID_ARG;
        }
    }
#ifndef WDRV_PIC32MZW_AUTH_WPA3_ENTERPRISE_FQDN_OPTIONAL
    if ((WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE == authType) ||
            (WDRV_PIC32MZW_AUTH_TYPE_WPA3_ENTERPRISE == authType))
    {
        /* Server domain is mandatory for WPA2WPA3 and WPA3 enterprise */
        if ((NULL == pServerDomain) || (0 == serverDomainLen))
        {
            return WDRV_PIC32MZW_STATUS_INVALID_ARG;
        }
    }
#endif
    /* username and password is mandatory for mschapv2 authentication */
    if ((NULL == pUserName) || (NULL == pPassword))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }
    userNameLen = strlen(pUserName);
    if ((0 == userNameLen) || (userNameLen > WDRV_PIC32MZW_ENT_AUTH_USERNAME_LEN_MAX))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }
    PasswordLen = strlen(pPassword);
    if ((0 == PasswordLen) || (PasswordLen > WDRV_PIC32MZW_ENT_AUTH_PASSWORD_LEN_MAX))
    {
        return WDRV_PIC32MZW_STATUS_INVALID_ARG;
    }

    /* Set authentication type */
    pAuthCtx->authType = authType;

    /* Initialize the MFP configuration to WDRV_PIC32MZW_AUTH_MFP_ENABLED.   */
    /* The Application may change the configuration later if desired via     */
    /* WDRV_PIC32MZW_AuthCtxConfigureMfp.                                    */
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_REQ;
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_MFP_OFF;

    /* Initialise the TD configuration to not enabled.                       */
    /* The Application may change the configuration later if desired via     */
    /* WDRV_PIC32MZW_AuthCtxApTransitionDisable or                           */
    /* WDRV_PIC32MZW_AuthCtxStaTransitionDisable.                            */
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_AP_TD;
    pAuthCtx->authMod &= ~WDRV_PIC32MZW_AUTH_MOD_STA_TD;


    /*********** configure phase 1 ***********/
    /* Set (domain-)username */
    memset(&pAuthCtx->authInfo.enterprise.phase1.identity, 0, WDRV_PIC32MZW_ENT_AUTH_IDENTITY_LEN_MAX+1);
    memcpy(&pAuthCtx->authInfo.enterprise.phase1.identity, pIdentity, identityLen);

    /* Set server domain name for validation of server cert's SAN(subject alternative name) or CN(common name) */
    memset(&pAuthCtx->authInfo.enterprise.phase1.serverDomainName, 0, WDRV_PIC32MZW_ENT_AUTH_SERVER_DOMAIN_LEN_MAX+1);
    if (NULL != pServerDomain)
    {
        memcpy(&pAuthCtx->authInfo.enterprise.phase1.serverDomainName, pServerDomain, serverDomainLen);
    }

    /* Copy the context handle */
    pAuthCtx->authInfo.enterprise.phase1.tlsCtxHandle = tlsCtxHandle;

    /******* configure phase 2 ************/
    /* Set EAP method type */
    pAuthCtx->authInfo.enterprise.auth1xMethod = WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTTLSv0_MSCHAPv2;

    /* Set MSCHAPv2 username */
    memset(&pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.username, 0, WDRV_PIC32MZW_ENT_AUTH_USERNAME_LEN_MAX+1);
    memcpy(&pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.username, pUserName, userNameLen);

    /* Set MSCHAPv2 password */
    memset(&pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.password, 0, WDRV_PIC32MZW_ENT_AUTH_PASSWORD_LEN_MAX+1);
    memcpy(&pAuthCtx->authInfo.enterprise.phase2.credentials.mschapv2.password, pPassword, PasswordLen);

    return WDRV_PIC32MZW_STATUS_OK;
}
#endif
