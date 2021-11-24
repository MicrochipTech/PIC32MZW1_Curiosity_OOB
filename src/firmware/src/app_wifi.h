/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_wifi.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_WIFI_Initialize" and "APP_WIFI_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_WIFI_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _APP_WIFI_H
#define _APP_WIFI_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "FreeRTOS.h"
#include "task.h"
#include "definitions.h"


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
    // DOM-IGNORE-END
    

    
extern char* WIFI_AUTH_STRING[];


#define DEFAULT_SSID "MCHP.IOT"
#define DEFAULT_SSID_PSK "microchip"
#define DEFAULT_AUTH_MODE (WIFI_AUTH)WIFI_WPAWPA2MIXED
#define DEFAULT_AUTH_MODE_STRING "WPAWPA2MIXED"
#define DEFAULT_AUTH_MODE_NUM "2"

#define WIFI_DEFAULT_REG_DOMAIN "USA"

    typedef enum {
        /* Application's state machine's initial state. */
        APP_WIFI_STATE_INIT = 0,
        APP_WIFI_ERROR,
        APP_WIFI_CONFIG,
        APP_WIFI_CONNECT,
        APP_WIFI_IDLE,
    } APP_WIFI_STATES;

    void APP_WIFI_Initialize(void);
    void APP_WIFI_Tasks(void);

#endif /* _APP_WIFI_H */

    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END
/*******************************************************************************
 End of File
 */

