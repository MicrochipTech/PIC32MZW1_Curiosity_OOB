/*******************************************************************************
  User Configuration Header

  File Name:
    user.h

  Summary:
    Build-time configuration header for the user defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    It only provides macro definitions for build-time configuration options

*******************************************************************************/

#ifndef USER_H
#define USER_H
#include "configuration.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: User Configuration macros
// *****************************************************************************
// *****************************************************************************


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END
//#define WOLFSSL_SMALL_STACK
////#define WOLFSSL_ATMEL
////#define WOLFSSL_ATECC508A_NOSOFTECC 

//#define DEBUG_WOLFSSL_VERBOSE
//#define WOLFSSL_DEBUG_ERRORS_ONLY
#define WOLFSSL_MAX_ERROR_SZ 100
#define WOLFSSL_DER_TO_PEM
#define WOLFSSL_BASE64_ENCODE
//#define SHOW_SECRETS
#define HAVE_TLS_EXTENSIONS
#define HAVE_SUPPORTED_CURVES
#define HAVE_SNI

#define APP_TNG_SUPPORT
#if defined(APP_TNG_SUPPORT)
    #define WOLFSSL_ATECC608A
    #define WOLFSSL_ATECC_TNGTLS
    #define WOLFSSL_ATECC_ECDH_IOENC
    #define HAVE_PK_CALLBACKS
    #define WOLFSSL_ATECC508A_NOIDLE
#endif

#endif // USER_H
/*******************************************************************************
 End of File
*/
