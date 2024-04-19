/*******************************************************************************
  PIC32MZW Driver Authentication Context Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_pic32mzw_authctx.h

  Summary:
    PIC32MZW wireless driver authentication context header file.

  Description:
    This interface manages the authentication contexts which 'wrap' the state
      associated with authentication schemes.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

#ifndef _WDRV_PIC32MZW_AUTHCTX_H
#define _WDRV_PIC32MZW_AUTHCTX_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_pic32mzw_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Authentication Context Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Authentication Types

  Summary:
    List of possible authentication types.

  Description:
    This type defines the possible authentication types that can be requested
    in AP mode or STA mode.

  Remarks:
    None.
*/

typedef enum
{
    /* This type may be used in helper function
     * WDRV_PIC32MZW_AuthCtxSetPersonal, in which case it is translated into
     * the best practice auth type. Other uses of this type are invalid. */
    WDRV_PIC32MZW_AUTH_TYPE_DEFAULT,

    /* No encryption. */
    WDRV_PIC32MZW_AUTH_TYPE_OPEN,

    /* WEP encryption. */
    WDRV_PIC32MZW_AUTH_TYPE_WEP,

    /* WPA2 mixed mode (AP) / compatibility mode (STA) with PSK.
     * (As an AP GTK is TKIP, as a STA GTK is chosen by AP;
     * PTK can be CCMP or TKIP) */
    WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL,

    /* WPA2-only authentication with PSK. (PTK and GTK are both CCMP).       */
    /* Note that a WPA2-only STA will not connect to a WPA2 mixed mode AP.   */
    WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL,

#ifdef WDRV_PIC32MZW_WPA3_PERSONAL_SUPPORT
    /* WPA3 SAE transition mode. (CCMP, IGTK can be BIP or none) */
    WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL,

    /* WPA3 SAE authentication. (CCMP, IGTK is BIP) */
    WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL,
#endif

#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
    /* WPA2/WPA enterprise mixed mode authentication type */
    WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_ENTERPRISE,

    /* WPA2 enterprise authentication type */
    WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE,

    /* WPA3 enterprise transition mode authentication type */
    WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE,

    /* WPA3 enterprise authentication type */
    WDRV_PIC32MZW_AUTH_TYPE_WPA3_ENTERPRISE,
#endif

    /* Authentication types with this value or above are not recognised. */
    WDRV_PIC32MZW_AUTH_TYPE_MAX
} WDRV_PIC32MZW_AUTH_TYPE;

#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
/*  802.1X Enterprise authentication methods
 *
  Summary:
    List of possible EAP methods supported by WPA-Enterprise authentication.

  Description:
    This type defines the possible EAP methods supported by WPA-Enterprise
    authentication in STA mode.

  Remarks:
    None.
*/

typedef enum
{
    /* phase1 - EAP-TLS - Extensible Authentication Protocol - Transport Layer Security
       phase2 - None */
    WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTLS = 1,
    /* phase1 - EAP-TTLSv0 - Extensible Authentication Protocol Tunneled Transport Layer Security
       Authenticated Protocol Version 0
       phase 2 - TTLSv0/MSCHPAv2 - Microsoft PPP CHAP Extensions, Version 2 */
    WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTTLSv0_MSCHAPv2
} WDRV_PIC32MZW_AUTH_1X_METHOD;
#endif

// *****************************************************************************
/*  Authentication Modifiers

  Summary:
    List of possible authentication modifiers.

  Description:
    This type defines the possible modifications that can be made to the
    authentication types in WDRV_PIC32MZW_AUTH_TYPE.

  Remarks:
    Not all modifiers are relevant to all auth types. When an auth context is
    applied, modifiers are ignored if they are not relevant to the auth type.
*/

typedef enum
{
    /* No modifiers set; the default behaviour for each auth type applies. */
    WDRV_PIC32MZW_AUTH_MOD_NONE         = 0,
    /* If set, this modifier causes management frame protection to be required.
     * It is relevant to the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE
     * This modifier can be set/cleared by WDRV_PIC32MZW_AuthCtxConfigureMfp. */
    WDRV_PIC32MZW_AUTH_MOD_MFP_REQ      = 0x01,
    /* If set, this modifier causes management frame protection to be disabled.
     * It is relevant to the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_ENTERPRISE
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE
     * This modifier is ignored if WDRV_PIC32MZW_AUTH_MOD_MFP_REQ is set.
     * This modifier can be set/cleared by WDRV_PIC32MZW_AuthCtxConfigureMfp. */
    WDRV_PIC32MZW_AUTH_MOD_MFP_OFF      = 0x02,
    /* If set, this modifier allows the device, as supplicant, to attempt
     * Shared Key authentication in the event that Open System authentication
     * is rejected by the authenticator.
     * It is relevant to the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WEP
     * This modifier can be set/cleared by WDRV_PIC32MZW_AuthCtxSharedKey. */
    WDRV_PIC32MZW_AUTH_MOD_SHARED_KEY   = 0x04,
    /* If set, this modifier causes the device, as authenticator, to include a
     * transition disable element. This instructs peer STAs to disable any
     * transition mode protocols.
     * It is relevant to the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL
     * This modifier can be set/cleared by
     *      WDRV_PIC32MZW_AuthCtxApTransitionDisable. */
    WDRV_PIC32MZW_AUTH_MOD_AP_TD        = 0x08,
    /* If set, this modifier causes the device, as supplicant, to disable any
     * transition mode protocols.
     * It is relevant to the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE (so long as MFP is enabled)
     *      WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_ENTERPRISE (so long as MFP is enabled)
     * This modifier can be set by WDRV_PIC32MZW_AuthCtxStaTransitionDisable. */
    WDRV_PIC32MZW_AUTH_MOD_STA_TD       = 0x10,
} WDRV_PIC32MZW_AUTH_MOD_MASK;

// *****************************************************************************
/*  MFP Configurations

  Summary:
    List of possible configurations for Management Frame Protection.

  Description:
    This type defines the possible configurations that can be specified in
    WDRV_PIC32MZW_AuthCtxConfigureMfp.

  Remarks:
    Not all MFP configurations are compatible with all auth types. When an auth
    context is applied, the MFP configuration is ignored if it is not
    compatible with the auth type.
*/

typedef enum
{
    /* Management Frame Protection enabled but not required.
     * This is the default configuration for the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_ENTERPRISE
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE
     * This configuration is not compatible with other auth types. */
    WDRV_PIC32MZW_AUTH_MFP_ENABLED,
    /* Management Frame Protection required.
     * This is an optional configuration for the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_ENTERPRISE
     * This configuration is not compatible with other auth types. */
    WDRV_PIC32MZW_AUTH_MFP_REQUIRED,
    /* Management Frame Protection disabled.
     * This is an optional configuration for the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_ENTERPRISE
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_ENTERPRISE
     * This configuration is not compatible with other auth types. */
    WDRV_PIC32MZW_AUTH_MFP_DISABLED,
} WDRV_PIC32MZW_AUTH_MFP_CONFIG;

// *****************************************************************************
/*  Authentication Context

  Summary:
    Context structure containing information about authentication types.

  Description:
    The context contains the type of authentication as well as any state
      information.

  Remarks:
    None.
*/

typedef struct
{
    /* Authentication type of context. */
    WDRV_PIC32MZW_AUTH_TYPE authType;

    /* Authentication modifiers. */
    WDRV_PIC32MZW_AUTH_MOD_MASK authMod;

    /* Union of data structures for each authentication type. */
    union
    {
        /* WEP authentication state. */
        struct
        {
            /* The WEP key index in the range 1-4. */
            uint8_t idx;
            /* The WEP key size is 10 for WEP_40 and 26 for WEP_104. */
            uint8_t size;
            /* The WEP key. */
            uint8_t key[WDRV_PIC32MZW_WEP_104_KEY_STRING_SIZE+1];
        } WEP;

        /* WPA-Personal authentication state. */
        struct
        {
            /* The size of the password or PSK, in characters/bytes          */
            /* Size must be in the range 8 to 63 for a WPA(2) password.      */
            /* Size must be 64 for a WPA(2) PSK.                             */
            uint8_t size;
            /* The password or PSK. */
            uint8_t password[WDRV_PIC32MZW_PSK_LEN];
        } personal;
#ifdef WDRV_PIC32MZW_ENTERPRISE_SUPPORT
         /* 802.1x authentication state. */
        struct
        {
            /* EAP method configured for 802.1X authentication */
            WDRV_PIC32MZW_AUTH_1X_METHOD   auth1xMethod;
            struct
            {
                /* Specifies the EAP identity name - [Domain][UserName] or [Username][Domain] */
                char identity[WDRV_PIC32MZW_ENT_AUTH_IDENTITY_LEN_MAX+1];
                /* WOLFSSL_CTX handle */
                WDRV_PIC32MZW_TLS_CONTEXT_HANDLE tlsCtxHandle;
                /* Server domain name against which either server certificate's subject alternative
                 * name(SAN) or common name(CN) shall be matched for successful enterprise connection */
                char serverDomainName[WDRV_PIC32MZW_ENT_AUTH_SERVER_DOMAIN_LEN_MAX + 1];
            } phase1;
            struct
            {
                union
                {
                    struct
                    {
                        /* username for mschapv2 authentication */
                        char username[WDRV_PIC32MZW_ENT_AUTH_USERNAME_LEN_MAX + 1];
                        /* password for mschapv2 authentication */
                        char password[WDRV_PIC32MZW_ENT_AUTH_PASSWORD_LEN_MAX + 1];
                    } mschapv2;
                } credentials;
            } phase2;
        } enterprise;
#endif
    } authInfo;
} WDRV_PIC32MZW_AUTH_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Section: PIC32MZW Driver Authentication Context Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    bool WDRV_PIC32MZW_AuthCtxIsValid(const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx)

  Summary:
    Tests if an authentication context is valid.

  Description:
    Tests the elements of the authentication context to judge if their values are legal.

  Precondition:
    None.

  Parameters:
    pAuthCtx  - Pointer to an authentication context.

  Returns:
    true or false indicating if context is valid.

  Remarks:
    None.

*/

bool WDRV_PIC32MZW_AuthCtxIsValid(const WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx);

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

  Precondition:
    None.

  Parameters:
    pAuthCtx - Pointer to an authentication context.

  Returns:
    WDRV_PIC32MZW_STATUS_OK          - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG - The parameters were incorrect.

  Remarks:
    A default context is not valid until it is configured.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetDefaults
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx
);

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

  Precondition:
    None.

  Parameters:
    pAuthCtx - Pointer to an authentication context.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetOpen
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx
);

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

  Precondition:
    None.

  Parameters:
    pAuthCtx - Pointer to an authentication context.
    idx      - WEP index.
    pKey     - Pointer to WEP key.
    size     - Size of WEP key.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetWEP
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    uint8_t idx,
    uint8_t *const pKey,
    uint8_t size
);

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

  Precondition:
    None.

  Parameters:
    pAuthCtx    - Pointer to an authentication context.
    pPassword   - Pointer to password (or 64-character PSK).
    size        - Size of password (or 64 for PSK).
    authType    - Authentication type (or WDRV_PIC32MZW_AUTH_TYPE_DEFAULT).

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetPersonal
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    uint8_t *const pPassword,
    uint8_t size,
    WDRV_PIC32MZW_AUTH_TYPE authType
);

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

  Precondition:
    None.

  Parameters:
    pAuthCtx    - Pointer to an authentication context.
    config      - The required Management Frame Protection configuration.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been updated.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    Not all MFP configurations are compatible with all auth types. When an auth
      context is applied, the MFP configuration is ignored if it is not
      compatible with the auth type.
    The MFP configuration is initialised to WDRV_PIC32MZW_AUTH_MFP_ENABLED by
      WDRV_PIC32MZW_AuthCtxSetPersonal and WDRV_PIC32MZW_AuthCtxSetEnterpriseTLS.
*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxConfigureMfp
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    WDRV_PIC32MZW_AUTH_MFP_CONFIG config
);

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

  Precondition:
    None.

  Parameters:
    pAuthCtx    - Pointer to an authentication context.
    enable      - True to enable Shared Key authentication, false to disable.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been updated.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    Shared Key authentication is initialised as disabled by
      WDRV_PIC32MZW_AuthCtxSetWEP.
    Shared Key is not available when the device is an authenticator.
    Shared Key authentication is only attempted if Open System authentication
      fails (i.e. not supported by the authenticator).
*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSharedKey
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    bool enable
);

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

  Precondition:
    None.

  Parameters:
    pAuthCtx    - Pointer to an authentication context.
    enableTD    - True to turn on transition disable, false to turn it off.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been updated.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    The transition disable element instructs STAs to disable transition
    security algorithms (i.e. AKM suites 2 and 6) from their network profile
    for this ESS.
*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxApTransitionDisable
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    bool tdOn
);

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

  Precondition:
    None.

  Parameters:
    pAuthCtx    - Pointer to an authentication context.

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been updated.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    The transition security algorithms to be disabled are the TKIP cipher and
      AKM suites 1, 2 and 6.
*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxStaTransitionDisable
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx
);


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
    Configure an authentication context for WPA-Enterprise authentication
    using enterprise authentication method == WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTLS.

  Description:
    The type and state information are configured appropriately for WPA-Enterprise
    authentication. The Management Frame Protection configuration is initialized
    to WDRV_PIC32MZW_AUTH_MFP_ENABLED

  Precondition:
    Wolfssl TLS context handle is created and all the required certs and keys are loaded,
    peer server certificate validation is enabled using the appropriate wolfssl APIs.

   Below is the example code for reference:
   <code>
    WDRV_PIC32MZW_TLS_CONTEXT_HANDLE APP_Create_TLS_Context(
        const uint8_t *const pCAcert,
        uint16_t u16CAcertLen,
        int caCertFormat,
        const uint8_t *const pCert,
        uint16_t u16CertLen,
        int privCertFormat,
        const uint8_t *const pPriKey,
        uint16_t u16PriKeyLen,
        int privKeyFormat)
    {
        WOLFSSL_CTX *pTlsCtx = NULL;

        if ((NULL == pCAcert) || (NULL == pCert) || (NULL == pPriKey))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }
        if ((u16CAcertLen == 0) || (u16CertLen == 0) || (u16PriKeyLen == 0 ))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        // Validate cert and key formats
        if (!((WOLFSSL_FILETYPE_PEM == caCertFormat) || (WOLFSSL_FILETYPE_ASN1 == caCertFormat)))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }
        if (!((WOLFSSL_FILETYPE_PEM == privCertFormat) || (WOLFSSL_FILETYPE_ASN1 == privCertFormat)))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }
        if (!((WOLFSSL_FILETYPE_PEM == privKeyFormat) || (WOLFSSL_FILETYPE_ASN1 == privKeyFormat)))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        // Create wolfssl context with TLS v1.2
        pTlsCtx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
        if (NULL == pTlsCtx)
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        // Load CA certificate into WOLFSSL_CTX for validating peer
        if (SSL_SUCCESS != wolfSSL_CTX_load_verify_buffer(pTlsCtx, pCAcert, u16CAcertLen, caCertFormat))
        {
            wolfSSL_CTX_free(pTlsCtx);
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }
        // Verify the certificate received from the server during the handshake
        wolfSSL_CTX_set_verify(pTlsCtx, WOLFSSL_VERIFY_PEER, 0);


        // Load client certificate into WOLFSSL_CTX
        if (SSL_SUCCESS != wolfSSL_CTX_use_certificate_buffer(pTlsCtx, pCert, u16CertLen, privCertFormat))
        {
            wolfSSL_CTX_free(pTlsCtx);
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        // Load client key into WOLFSSL_CTX
        if (SSL_SUCCESS != wolfSSL_CTX_use_PrivateKey_buffer(pTlsCtx, pPriKey, u16PriKeyLen, privKeyFormat))
        {
            wolfSSL_CTX_free(pTlsCtx);
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        return (WDRV_PIC32MZW_TLS_CONTEXT_HANDLE) pTlsCtx;
    }
   </code>

  Parameters:
    pAuthCtx         - Pointer to an authentication context.
    authType         - Authentication type
    pIdentity        - Pointer to EAP Identity(user and domain name).
    tlsCtxHandle     - Wolfssl TLS Context handle.
    pServerDomain    - Server domain name against which either server certificate's
                       subject alternative name(SAN) or common name(CN) shall be
                       matched for successful enterprise connection

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_PIC32MZW_STATUS WDRV_PIC32MZW_AuthCtxSetEnterpriseTLS
(
    WDRV_PIC32MZW_AUTH_CONTEXT *const pAuthCtx,
    const char *const pIdentity,
    WDRV_PIC32MZW_TLS_CONTEXT_HANDLE tlsCtxHandle,
    const char *const pServerDomain,
    WDRV_PIC32MZW_AUTH_TYPE authType
);


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
        WDRV_PIC32MZW_AUTH_TYPE authType,
        WDRV_PIC32MZW_AUTH_1X_METHOD auth1xMethod
    )

  Summary:
    Configure an authentication context for WPA-Enterprise authentication
    using enterprise authentication method == WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTTLSv0_MSCHAPv2.

  Description:
    The type and state information are configured appropriately for WPA-Enterprise
    authentication. The Management Frame Protection configuration is initialized
    to WDRV_PIC32MZW_AUTH_MFP_ENABLED

  Precondition:
    Wolfssl TLS context handle is created and all the required certs and keys are loaded,
    peer server certificate validation is enabled using the appropriate wolfssl APIs.

   Below is the example code for reference:
   <code>
    WDRV_PIC32MZW_TLS_CONTEXT_HANDLE APP_Create_TLS_Context(
        const uint8_t *const pCAcert,
        uint16_t u16CAcertLen,
        int caCertFormat,
        const uint8_t *const pCert,
        uint16_t u16CertLen,
        int privCertFormat,
        const uint8_t *const pPriKey,
        uint16_t u16PriKeyLen,
        int privKeyFormat)
    {
        WOLFSSL_CTX *pTlsCtx = NULL;

        if ((NULL == pCAcert) || (NULL == pCert) || (NULL == pPriKey))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }
        if ((u16CAcertLen == 0) || (u16CertLen == 0) || (u16PriKeyLen == 0 ))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        // Validate cert and key formats
        if (!((WOLFSSL_FILETYPE_PEM == caCertFormat) || (WOLFSSL_FILETYPE_ASN1 == caCertFormat)))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }
        if (!((WOLFSSL_FILETYPE_PEM == privCertFormat) || (WOLFSSL_FILETYPE_ASN1 == privCertFormat)))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }
        if (!((WOLFSSL_FILETYPE_PEM == privKeyFormat) || (WOLFSSL_FILETYPE_ASN1 == privKeyFormat)))
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        // Create wolfssl context with TLS v1.2
        pTlsCtx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
        if (NULL == pTlsCtx)
        {
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        // Load CA certificate into WOLFSSL_CTX for validating peer
        if (SSL_SUCCESS != wolfSSL_CTX_load_verify_buffer(pTlsCtx, pCAcert, u16CAcertLen, caCertFormat))
        {
            wolfSSL_CTX_free(pTlsCtx);
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }
        // Verify the certificate received from the server during the handshake
        wolfSSL_CTX_set_verify(pTlsCtx, WOLFSSL_VERIFY_PEER, 0);


        // Load client certificate into WOLFSSL_CTX
        if (SSL_SUCCESS != wolfSSL_CTX_use_certificate_buffer(pTlsCtx, pCert, u16CertLen, privCertFormat))
        {
            wolfSSL_CTX_free(pTlsCtx);
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        // Load client key into WOLFSSL_CTX
        if (SSL_SUCCESS != wolfSSL_CTX_use_PrivateKey_buffer(pTlsCtx, pPriKey, u16PriKeyLen, privKeyFormat))
        {
            wolfSSL_CTX_free(pTlsCtx);
            return WDRV_PIC32MZW_TLS_CONTEXT_HANDLE_INVALID;
        }

        return (WDRV_PIC32MZW_TLS_CONTEXT_HANDLE) pTlsCtx;
    }
   </code>

  Parameters:
    pAuthCtx         - Pointer to an authentication context.
    authType         - Authentication type
    pIdentity        - Pointer to EAP Identity(user and domain name).
    tlsCtxHandle     - Wolfssl TLS Context handle.
    pServerDomain    - Server domain name against which either server certificate's
                       subject alternative name(SAN) or common name(CN) shall be
                       matched for successful enterprise connection
    pUserName        - User name for phase2 authentication if auth1xMethod is
                       WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTTLSv0_MSCHAPv2.
    pPassword        - Password for phase2 authentication if auth1xMethod is
                       WDRV_PIC32MZW_AUTH_1X_METHOD_EAPTTLSv0_MSCHAPv2

  Returns:
    WDRV_PIC32MZW_STATUS_OK             - The context has been configured.
    WDRV_PIC32MZW_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    None.

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
);
#endif

#endif /* _WDRV_PIC32MZW_AUTHCTX_H */
