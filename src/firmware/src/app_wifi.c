/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_wifi.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************


#include <string.h>
#include "app_wifi.h"
#include "app_control.h"

typedef struct {
    APP_WIFI_STATES state;
    DRV_HANDLE wdrvHandle;
    volatile bool isRegDomainSet;
    volatile bool isConnected;
} APP_WIFI_DATA;

APP_WIFI_DATA app_wifiData;
void WiFiServCallback(uint32_t event, void * data, void *cookie) 
{
    switch (event) 
    {
        case SYS_WIFI_CONNECT:
        {
            app_wifiData.isConnected = true;
            app_controlData.wifiCtrl.wifiConnected = true;
            SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_GETDRVASSOCHANDLE, 
                    &app_controlData.rssiData.assocHandle, 4); 
            break;
        }
        case SYS_WIFI_DISCONNECT:
        {
            //SYS_CONSOLE_PRINT("Device DISCONNECTED from AP\r\n");
            app_wifiData.isConnected = false;
            app_controlData.wifiCtrl.wifiConnected = false;
            break;
        }
    }
}

void APP_WIFI_Initialize(void) {
    /* Place the App state machine in its initial state. */
    app_wifiData.state = APP_WIFI_STATE_INIT;
    app_wifiData.isConnected = false;
    app_controlData.wifiCtrl.wifiConnected = false;
}
static SYS_WIFI_CONFIG wifiConfig;

void APP_WIFI_Tasks(void) {
    int ret =0;      

 switch (app_wifiData.state) {
        case APP_WIFI_STATE_INIT:
            ret=SYS_WIFI_CtrlMsg(sysObj.syswifi,SYS_WIFI_REGCALLBACK,WiFiServCallback,sizeof(uint8_t*));
            if (SYS_WIFI_SUCCESS!=ret){
                app_wifiData.state=APP_WIFI_ERROR;
                SYS_CONSOLE_MESSAGE("APP_WIFI: " TERM_RED "Failed registering Wi-Fi control message\r\n" TERM_RESET);
            }
            app_wifiData.state=APP_WIFI_CONFIG;
        break;
     case APP_WIFI_CONFIG:
            if (true == app_controlData.wifiCtrl.wifiCtrlValid) {
                wifiConfig.mode=SYS_WIFI_STA;
                wifiConfig.saveConfig=0;
                strncpy((char*)&wifiConfig.countryCode,WIFI_DEFAULT_REG_DOMAIN,strlen(WIFI_DEFAULT_REG_DOMAIN)+1);
                strncpy((char*)&wifiConfig.staConfig.ssid,app_controlData.wifiCtrl.SSID,APP_CTRL_MAX_SSID_LEN);
                strncpy((char*)&wifiConfig.staConfig.psk,app_controlData.wifiCtrl.pass,APP_CTRL_MAX_WIFI_PASS_LEN);
                wifiConfig.staConfig.authType=app_controlData.wifiCtrl.authmode;
                wifiConfig.staConfig.autoConnect=1;
                wifiConfig.staConfig.channel=0;
                        
                ret=SYS_WIFI_CtrlMsg(sysObj.syswifi,SYS_WIFI_CONNECT,&wifiConfig,sizeof(SYS_WIFI_CONFIG));
                if (SYS_WIFI_SUCCESS!=ret){
                    app_wifiData.state=APP_WIFI_ERROR;
                    SYS_CONSOLE_MESSAGE("APP_WIFI: " TERM_RED "Error requesting Wi-Fi connection\r\n" TERM_RESET);
                }
                app_wifiData.state=APP_WIFI_CONNECT;
            } else {
                break;
            }
         break;
     case APP_WIFI_CONNECT:
         if (true==app_wifiData.isConnected){
             app_wifiData.state=APP_WIFI_IDLE;
         }
         break;
     case APP_WIFI_IDLE:
         break;
     case APP_WIFI_ERROR:
         break;
            
            
            
                        
 }
}
/*******************************************************************************
 End of File
 */
 