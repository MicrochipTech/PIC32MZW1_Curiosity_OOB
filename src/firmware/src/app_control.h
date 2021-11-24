/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_control.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_CONTROL_Initialize" and "APP_CONTROL_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_CONTROL_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _APP_CONTROL_H
#define _APP_CONTROL_H

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
#include "time.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
    typedef enum {
        /* Application's state machine's initial state. */
        APP_CONTROL_STATE_INIT = 0,
        APP_CONTROL_STATE_MONITOR_CONNECTION,
        APP_CONTROL_STATE_MONITOR_SWITCH,
        APP_CONTROL_STATE_ADC_READ,
        APP_CONTROL_STATE_RTCC_READ
    } APP_CONTROL_STATES;

#define VT100TERM
#ifdef VT100TERM
#define TERM_GREEN "\x1B[32m"
#define TERM_RED   "\x1B[31m"
#define TERM_YELLOW "\x1B[33m"
#define TERM_CYAN "\x1B[36m"
#define TERM_WHITE "\x1B[47m"
#define TERM_RESET "\x1B[0m"
#define TERM_BG_RED "\x1B[41m" 
#define TERM_BOLD "\x1B[1m" 
#define TERM_UL "\x1B[4m"

#define TERM_CTRL_RST "\x1B\x63"
#define TERM_CTRL_CLRSCR "\x1B[2J"
#else 
#define TERM_GREEN 
#define TERM_RED   
#define TERM_YELLOW 
#define TERM_CYAN 
#define TERM_WHITE 
#define TERM_RESET 
#define TERM_BG_RED
#define TERM_BOLD 
#define TERM_UL 

#define TERM_CTRL_RST 
#define TERM_CTRL_CLRSCR 
#endif



#define APP_CTRL_MAX_SSID_LEN 16
#define APP_CTRL_MAX_WIFI_PASS_LEN 64
#define APP_CTRL_WIFI_SEC_MODE

#define APP_CTRL_MAX_BROKER_NAME_LEN 128
#define APP_CTRL_MAX_CLIENT_ID_LEN 128

#define APP_CTRL_OC_TIMER_PERIOD 65000

#define APP_CTRL_ADC_VREF                (3.3f)
#define APP_CTRL_ADC_MAX_COUNT           (4095)
#define APP_CTRL_ADC_AVG_COUNT           10

#define APP_ATCA_SERIAL_NUM_SIZE        (9)
#define APP_SERIAL_NUM_STR_LEN (APP_ATCA_SERIAL_NUM_SIZE * 2)
    
    typedef enum 
    {
        /* Requesting a Open Authentication types */
        WIFI_OPEN = 1,

        /* Requesting a WEP Authentication types */
        WIFI_WEP,

        /* Requesting a WPA/WPA2(Mixed) Authentication types */
        WIFI_WPAWPA2MIXED,

        /* Requesting a WPA2 Authentication types */
        WIFI_WPA2,

        /* Requesting a WPA2/WPA3(Mixed) Authentication types */
        WIFI_WPA2WPA3MIXED,

        /* Requesting a WPA3 Authentication types */
        WIFI_WPA3

    } WIFI_AUTH ;
    
    typedef struct {
        bool wifiCtrlValid;
        bool wifiCtrlChanged;
        bool wifiConnected;
        char SSID[APP_CTRL_MAX_SSID_LEN];
        char pass[APP_CTRL_MAX_WIFI_PASS_LEN];
        WIFI_AUTH authmode;
    } APP_CTRL_WIFI_DATA;

    typedef struct {
        bool mqttConfigValid;
        char mqttBroker[APP_CTRL_MAX_BROKER_NAME_LEN];
        char clientId[APP_CTRL_MAX_CLIENT_ID_LEN];
        bool conStat;
    } APP_CTRL_MQTT_DATA;
    
    typedef struct {
        bool bootSwitch;
        bool switchStatus;
    } APP_CTRL_SWITCH_DATA;
    
    typedef struct {
        bool dataReady;
        uint16_t adcCount;
        float temp;
    } APP_CTRL_ADC_DATA;
    
    typedef struct{
        uintptr_t assocHandle;
    }APP_RSSI_DATA;
    
    typedef struct {
        struct tm sys_time;
    }APP_RTCC_DATA;

    typedef struct {
        char devSerialStr[APP_SERIAL_NUM_STR_LEN + 1];
        bool serialNumValid;
        APP_CONTROL_STATES state;
        APP_CTRL_WIFI_DATA wifiCtrl;
        APP_CTRL_MQTT_DATA mqttCtrl;
        APP_CTRL_SWITCH_DATA switchData;
        APP_CTRL_ADC_DATA adcData;
        APP_RSSI_DATA rssiData;
        APP_RTCC_DATA rtccData;
    } APP_CONTROL_DATA;

    extern APP_CONTROL_DATA app_controlData;

    void APP_CONTROL_Initialize(void);
    void APP_CONTROL_Tasks(void);
    void softResetDevice(void);

#endif /* _APP_CONTROL_H */

    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

