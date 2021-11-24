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
/*******************************************************************************
Copyright (C) 2020-21 released Microchip Technology Inc. All rights reserved.

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

#ifdef WDRV_PIC32MZW_WPA3_SUPPORT
    /* WPA3 SAE transition mode. (CCMP, IGTK can be BIP or none) */
    WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL,

    /* WPA3 SAE authentication. (CCMP, IGTK is BIP) */
    WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL,
#endif

    /* Authentication types with this value or above are not recognised. */
    WDRV_PIC32MZW_AUTH_TYPE_MAX
} WDRV_PIC32MZW_AUTH_TYPE;

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
     * This modifier can be set/cleared by WDRV_PIC32MZW_AuthCtxConfigureMfp. */
    WDRV_PIC32MZW_AUTH_MOD_MFP_REQ      = 0x01,
    /* If set, this modifier causes management frame protection to be disabled.
     * It is relevant to the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL
     * This modifier is ignored if WDRV_PIC32MZW_AUTH_MOD_MFP_REQ is set.
     * This modifier can be set/cleared by WDRV_PIC32MZW_AuthCtxConfigureMfp. */
    WDRV_PIC32MZW_AUTH_MOD_MFP_OFF      = 0x02,
    /* If set, this modifier allows the device (as supplicant) to attempt
     * Shared Key authentication in the event that Open System authentication
     * is rejected by the authenticator.
     * It is relevant to the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WEP
     * This modifier can be set/cleared by WDRV_PIC32MZW_AuthCtxSharedKey. */
    WDRV_PIC32MZW_AUTH_MOD_SHARED_KEY   = 0x04,
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
     * This configuration is not compatible with other auth types. */
    WDRV_PIC32MZW_AUTH_MFP_ENABLED,
    /* Management Frame Protection required.
     * This is an optional configuration for the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL
     * This configuration is not compatible with other auth types. */
    WDRV_PIC32MZW_AUTH_MFP_REQUIRED,
    /* Management Frame Protection disabled.
     * This is an optional configuration for the following auth types:
     *      WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL
     *      WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL
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
      WDRV_PIC32MZW_AuthCtxSetPersonal.
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

#endif /* _WDRV_PIC32MZW_AUTHCTX_H */
