/*******************************************************************************
  Wi-Fi System Service Implementation


  File Name
    sys_wifi.h

  Summary
    Wi-Fi system service interface.

  Description
    This file defines the interface to the Wi-Fi system service.  This
    system service provides a simple APIs to enable PIC32MZW1 Wi-Fi 
    Functionality.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020-2021 released Microchip Technology Inc.  All rights reserved.

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

#ifndef _SYS_WIFI_H 
#define _SYS_WIFI_H 

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

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types and Constants
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* System Wi-Fi service Authentication types

  Summary:
    Identifies the type of Authentication requested.

  Description:
    Identifies the type of Authentication requested.

  Remarks:
    None.
*/

typedef enum 
{
    /* Requesting a Open Authentication types */
    SYS_WIFI_OPEN = 1,

    /* Requesting a WEP Authentication types */
    SYS_WIFI_WEP,

    /* Requesting a WPA/WPA2(Mixed) Authentication types */
    SYS_WIFI_WPAWPA2MIXED,

    /* Requesting a WPA2 Authentication types */
    SYS_WIFI_WPA2,

    /* Requesting a WPA2/WPA3(Mixed) Authentication types */
    SYS_WIFI_WPA2WPA3MIXED,

    /* Requesting a WPA3 Authentication types */
    SYS_WIFI_WPA3,
} SYS_WIFI_AUTH ;

typedef enum{

    /* Requesting a TLS mode within Enterprise Authentication mode types */    
    SYS_WIFI_ENTERPRISE_TLS,
    
    /* Requesting a TTLS mode within Enterprise Authentication mode types */           
    SYS_WIFI_ENTERPRISE_TTLS,   
            
}SYS_WIFI_ENTERPRISE_METHOD;

// *****************************************************************************
/* System Wi-Fi service control message types

  Summary:
    Identifies the control message for which the client has called 
    the SYS_WIFI_CtrlMsg().

  Description:
    Identifies the control message for which the client has called 
    the SYS_WIFI_CtrlMsg().

  Remarks:
   The different control messages which can be invoked by the client. 
*/

typedef enum 
{
    /* Control message type for requesting a Wi-Fi Configuration 
       set(for connect) */ 
    SYS_WIFI_CONNECT = 0,

    /*Control message type for requesting a Wi-Fi device disconnect */
    SYS_WIFI_DISCONNECT,
    
    /*Control message type for auto reconnect failure */
    SYS_WIFI_AUTO_CONNECT_FAIL,
 
    /* Control message type for requesting a Wi-Fi configuration information */
    SYS_WIFI_GETWIFICONFIG,

    
    /* Control message type for registering a Wi-Fi system service 
       client callback */
    SYS_WIFI_REGCALLBACK,

            
    /*Control message type for requesting a Wi-Fi driver handle */
    SYS_WIFI_GETDRVHANDLE,
    /*Control message type for requesting a Assoc handle */
    SYS_WIFI_GETDRVASSOCHANDLE,

} SYS_WIFI_CTRLMSG ;

// *****************************************************************************
/* System Wi-Fi service operating  modes

  Summary:
    Identifies the Wi-Fi operating mode.

  Description:
    Identifies the Wi-Fi operating mode.

  Remarks:
   Client need to manually reboot device after switching mode.
   For example, changing operating mode to STA to AP or AP to STA.
*/

typedef enum 
{
    /* Requesting a operating mode a station */
    SYS_WIFI_STA = 0
} SYS_WIFI_MODE ;


// *****************************************************************************
/* System Wi-Fi service station mode configuration structure.

  Summary:
    Configuration of station parameters.

  Description:
    Configuration of station parameters.

  Remarks:
   None.
*/
typedef struct 
{
    /* Wi-Fi station mode SSID */
    uint8_t ssid[33];

    /* Wi-Fi station mode passphrase */
    uint8_t psk[64];

    /* Wi-Fi station mode authentication type */
    SYS_WIFI_AUTH authType;

     /* Wi-Fi station mode channel number.
        values of channel:  
        0 - scan and connect to all the channels
        1 to 13 - - scan and connect to specified channel */
    uint8_t channel;

    /* Wi-Fi station mode auto connect flag. 
      value 0- Don't connect to AP, wait for client request.
      value 1- Connect to AP immediately */
    bool autoConnect;
    
    /* Wi-Fi station mode IP address */
    IPV4_ADDR ipAddr;


} SYS_WIFI_STA_CONFIG;



// *****************************************************************************
/* System Wi-Fi service device configuration structure.

  Summary:
    Configuration of device configuration parameters.

  Description:
    Configuration of device configuration parameters.

  Remarks:
   None.
*/

typedef struct 
{
    /* Operating mode of the device */
    SYS_WIFI_MODE mode;

    /* Flag to identify if configuration needs to be saved in NVM.
       0 - Do not save configuration in NVM.
       1 - Save configuration in NVM. */
    uint8_t saveConfig;

    /* Country Code configuration */
    uint8_t countryCode[6];


    /* Wi-Fi station mode configuration structure */
    SYS_WIFI_STA_CONFIG staConfig;


}SYS_WIFI_CONFIG;


// *****************************************************************************
/* System Wi-Fi service Status .

  Summary:
    Result of a Wi-Fi service client interface get status 
    operation(SYS_WIFI_GetStatus()).

  Description:
    Result of a Wi-Fi service client interface get status 
    operation(SYS_WIFI_GetStatus()).

  Remarks:
   None.
*/


typedef enum
{
    /* Wi-Fi system service is in init status */
    SYS_WIFI_STATUS_INIT = 1,

    /* Wi-Fi system service is in driver open status */
    SYS_WIFI_STATUS_WDRV_OPEN_REQ,

    /* Wi-Fi system service is in auto connect wait status */
    SYS_WIFI_STATUS_AUTOCONNECT_WAIT,

    /* Wi-Fi system service is in wait for TCPIP stack init status */
    SYS_WIFI_STATUS_TCPIP_WAIT_FOR_TCPIP_INIT,

    /* Wi-Fi system service is in Wi-Fi connect request status */
    SYS_WIFI_STATUS_CONNECT_REQ,

    /* Wi-Fi system service is in station mode IP address received status*/        
    SYS_WIFI_STATUS_STA_IP_RECIEVED,

 
    /* Wi-Fi system service is in TCPIP ready status, waiting for client request.*/
    SYS_WIFI_STATUS_TCPIP_READY,
    
   /* Wi-Fi system service is in TCPIP error status */
    SYS_WIFI_STATUS_TCPIP_ERROR,
            
    /* Wi-Fi system service is in config error status */
    SYS_WIFI_STATUS_CONFIG_ERROR,

    /* Wi-Fi system service is in connection error status */
    SYS_WIFI_STATUS_CONNECT_ERROR,

    /* Wi-Fi system service is in not in valid status */
    SYS_WIFI_STATUS_NONE =255
} SYS_WIFI_STATUS;


// *****************************************************************************
/* System Wi-Fi service Result.

  Summary:
    Result of a Wi-Fi system service client interface operation.

  Description:
    Identifies the result of  Wi-Fi service operations

  Remarks:
   None.
*/

typedef enum{
    
    /* Operation completed with success */
    SYS_WIFI_SUCCESS =  0,

    /* Operation Failed.*/
    SYS_WIFI_FAILURE,

    /* Wi-Fi service un-initialize */
    SYS_WIFI_SERVICE_UNINITIALIZE,

    /*Wi-Fi configuration request failed */
    SYS_WIFI_CONFIG_FAILURE,

    //Wi-Fi Connect request failed
    SYS_WIFI_CONNECT_FAILURE,

    //Wi-Fi Save request failed
    SYS_WIFI_SAVE_FAILURE,

    //Operation request object is invalid
    SYS_WIFI_OBJ_INVALID=255

}SYS_WIFI_RESULT;

// *****************************************************************************
/* Function:
    typedef void (*SYS_WIFI_CALLBACK )(uint32_t event, void * data,void *cookie )

   Summary:
    Pointer to a Wi-Fi system service callback function.

   Description:
    This data type defines a pointer to a Wi-Fi service callback function.
    Callback functions can be registered by client at initialization or using 
    control message type.

   Precondition:
    The Wi-Fi service must have been initialized using the SYS_WIFI_Initialize
    function if client registering callback using control message.

   Parameters:
    event     - A event value, event can be any  of SYS_WIFI_CTRLMSG types.
    data      - Wi-Fi service Data.
    cookie    - Client register cookie.

   Returns:
    None.

  Example:
    <code>
    #include "app.h"
    #include "system/console/sys_console.h"
    #include "system/wifi/sys_wifi.h"
    #include "tcpip/tcpip.h"
    #include "definitions.h"

    //User can refer the application "wireless_apps_pic32mzw1_wfi32e01\apps\wifi_easy_config" for more information on how to implement callback.
    APP_DATA appData;
    void WiFiServCallback (uint32_t event, void * data,void *cookie )
    {
        IPV4_ADDR *IPAddr; 
        switch(event)
        {
            case SYS_WIFI_CONNECT:
            {
                //In STA mode, Wi-Fi service share IP address provided by AP in the callback
                IPAddr = (IPV4_ADDR *)data;
                SYS_CONSOLE_PRINT("IP address obtained = %d.%d.%d.%d \\r\\n",IPAddr->v[0], IPAddr->v[1], IPAddr->v[2], IPAddr->v[3]);

                //In AP mode, Wi-Fi service share MAC address and IP address of the connected STA in the callback
                SYS_WIFI_STA_APP_INFO    *StaConnInfo = (SYS_WIFI_STA_APP_INFO *)data;
                SYS_CONSOLE_PRINT("STA Connected to AP. Got IP address = %d.%d.%d.%d \r\n", 
                        StaConnInfo->ipAddr.v[0], StaConnInfo->ipAddr.v[1], StaConnInfo->ipAddr.v[2], StaConnInfo->ipAddr.v[3]);                
                SYS_CONSOLE_PRINT("STA Connected to AP. Got MAC address = %x:%x:%x:%x:%x:%x \r\n", 
                        StaConnInfo->macAddr[0], StaConnInfo->macAddr[1], StaConnInfo->macAddr[2],
                        StaConnInfo->macAddr[3], StaConnInfo->macAddr[4], StaConnInfo->macAddr[5]);
                
                break;
            }
            case SYS_WIFI_DISCONNECT:
            {
                SYS_CONSOLE_PRINT("Device DISCONNECTED \\r\\n");
                break;
            }
            case SYS_WIFI_PROVCONFIG:
            {
                SYS_CONSOLE_PRINT("Received the Provisioning data \\r\\n");
                break;
            }
        }
    }
    void APP_Initialize(void) {
    appData.state = APP_STATE_INIT;
   }
    
    void APP_Tasks(void) {

    switch (appData.state) {
        case APP_STATE_INIT:
        {
            SYS_WIFI_CtrlMsg(sysObj.syswifi,SYS_WIFI_REGCALLBACK,WiFiServCallback,sizeof(uint8_t *));
            appData.state=APP_STATE_SERVICE_TASKS;
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {

            break;
        }
        default:
        {
            break;
        }
    }
    }

    </code>

  Remarks:
    None.
*/

typedef void (*SYS_WIFI_CALLBACK )(uint32_t event, void * data,void *cookie );

// *****************************************************************************
// *****************************************************************************
// Section: Initialization functions
// *****************************************************************************
// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ SYS_WIFI_Initialize
    ( 
        SYS_WIFI_CONFIG *config,
        SYS_WIFI_CALLBACK callback,
        void *cookie
    )

   Summary:
        Initializes the System Wi-Fi module.

   Description:
        Wi-Fi service supports only one single instance of Wi-Fi.

   Parameters:
       config    - Wi-Fi device configuration structure.
       callback  - The client callback function pointer.
       cookie    - The pointer which will be passed to the customer application 
                   when the customer callback function is invoked.

   Returns:
        If successful, returns a valid handle to an object. Otherwise, it
        returns SYS_MODULE_OBJ_INVALID.

   Example:
        <code>
        #define WIFI_DEV_SSID  "DEMO_AP"
        #define WIFI_DEV_PSK   "password"

        SYS_WIFI_CONFIG    wifiSrvcConfig;

        // Set mode as STA
        wifiSrvcConfig.mode = SYS_WIFI_STA;
        
        // Disable saving wifi configuration
        wifiSrvcConfig.saveConfig = false;

        // Set the auth type to SYS_WIFI_WPA2 
        wifiSrvcConfig.staConfig.authType = SYS_WIFI_WPA2;

        // Enable all the channels(0)
        wifiSrvcConfig.staConfig.channel = 0;

        // Device doesn't wait for user request
        wifiSrvcConfig.staConfig.autoConnect = 1;
        
        // Set SSID
        memcpy(wifiSrvcConfig.staConfig.ssid,WIFI_DEV_SSID,sizeof(WIFI_DEV_SSID));

        // Set PSK
        memcpy(wifiSrvcConfig.staConfig.psk,WIFI_DEV_PSK,sizeof(WIFI_DEV_PSK));

        sysObj.syswifi = SYS_WIFI_Initialize(&wifiSrvcConfig, WiFiServCallback, 0);                
        if (sysObj.syswifi == SYS_MODULE_OBJ_INVALID)
        {
            // Handle error
        }
        </code>


  Remarks:
        This routine can only be called once during system initialization.
        If the Wi-Fi system service is enabled using MHC, then auto generated 
        code will take care of system wi-fi initialization.
*/

SYS_MODULE_OBJ SYS_WIFI_Initialize( SYS_WIFI_CONFIG *config,SYS_WIFI_CALLBACK callback,void *cookie);

// *****************************************************************************
/* Function:
   SYS_WIFI_RESULT SYS_WIFI_Deinitialize (SYS_MODULE_OBJ object)

  Summary:
       Deinitializes the module instance of the system Wi-Fi service

  Description:
       This function deinitializes the module instance disabling its
       operation. Resets all of the internal data structures and fields 
       to the default settings.

  Precondition:
       The SYS_WIFI_Initialize function should have been called before calling
       this function.

  Parameters:
       object   - SYS WIFI object handle, returned from SYS_WIFI_Initialize

  Returns:
       return SYS_WIFI_RESULT 

  Example:
        <code>
        
         if (SYS_WIFI_SUCCESS == SYS_WIFI_Deinitialize (sysObj.syswifi))
        {
            // when the SYS WIFI is De-initialized.
        }
        </code>

  Remarks:
       Deinitialize should be called if the WiFi service is no longer 
       going to be used.
*/

SYS_WIFI_RESULT SYS_WIFI_Deinitialize (SYS_MODULE_OBJ object) ;

// *****************************************************************************
// *****************************************************************************
// Section: Status functions
// *****************************************************************************
// *****************************************************************************
/* Function:
   uint8_t SYS_WIFI_GetStatus ( SYS_MODULE_OBJ object)

  Summary:
        Returns Wi-Fi system service status.

  Description:
    This function returns the current status of the System Wi-Fi service.
	
    This function help user to perform synchronize functionality with Wi-Fi service.

  Precondition:
       The SYS_WIFI_Initialize function should have been called before calling
       this function.

  Parameters:
       object   - SYS WIFI object handle, returned from SYS_WIFI_Initialize

  Returns:
       return SYS_WIFI_STATUS if client provided object is valid, else return SYS_WIFI_OBJ_INVALID.

  Example:
        <code>
		
        //Wi-Fi system service is in TCPIP ready status, waiting for client request.
        if (SYS_WIFI_STATUS_TCPIP_READY == SYS_WIFI_GetStatus (sysObj.syswifi))
        {
            // when the SYS WIFI module in TCPIP ready STATUS
        }
        </code>

  Remarks:
    None
*/

uint8_t SYS_WIFI_GetStatus ( SYS_MODULE_OBJ object) ;

// *****************************************************************************
// *****************************************************************************
// Section: Setup functions
// *****************************************************************************
// *****************************************************************************
/* Function:
   uint8_t SYS_WIFI_Tasks ( SYS_MODULE_OBJ object)

  Summary:
    Maintains the Wi-Fi System tasks and functionalities. 

  Description:
    This function is used to run the various tasks and functionalities 
    of Wi-Fi system service.

  Precondition:
       The SYS_WIFI_Initialize function should have been called before calling
       this function.

  Parameters:
       object   - SYS WIFI object handle, returned from SYS_WIFI_Initialize

  Returns:
       return SYS_WIFI_STATUS if client provided object is valid,
        else return SYS_WIFI_OBJ_INVALID.

  Example:
        <code>

         if (SYS_WIFI_OBJ_INVALID != SYS_WIFI_Tasks (sysObj.syswifi))
        {
            
        }

        </code>

  Remarks:
    If the Wi-Fi system service is enabled using MHC, then auto generated code 
    will take care of system task execution.
*/

uint8_t SYS_WIFI_Tasks (SYS_MODULE_OBJ object);

// *****************************************************************************
/* Function:
   SYS_WIFI_RESULT SYS_WIFI_CtrlMsg 
   (
         SYS_MODULE_OBJ object,
         uint32_t event,
         void *buffer,
         uint32_t length 
   )

  Summary:
    Returns success/ failure for the connect/disconnect/scan operation asked by client. 

  Description:
    This function is used to make control message request
    (connect,disconnect,scan,register callback) to Wi-Fi system service.

  Precondition:
       The SYS_WIFI_Initialize function should have been called before calling
       this function.

  Parameters:
       object   - SYS WIFI object handle, returned from SYS_WIFI_Initialize
       event    - A event value, event can be any of SYS_WIFI_CTRLMSG types
       buffer   - Control message data input.
       length   - size of buffer data


  Returns:
       return SYS_WIFI_RESULT.

  Example:
        <code>

        Details of SYS_WIFI_CONNECT:
            SYS_WIFI_CONFIG    wifiSrvcConfig;
            SYS_MODULE_OBJ     WiFiServHandle;

            // Set mode as STA 
            wifiSrvcConfig.mode = SYS_WIFI_STA;

            // Disable saving wifi configuration 
            wifiSrvcConfig.saveConfig = false;   

            // Set the auth type to SYS_WIFI_WPA2
            wifiSrvcConfig.staConfig.authType = SYS_WIFI_WPA2;

            // Enable all the channels(0)
            wifiSrvcConfig.staConfig.channel = 0;

            // Device doesn't wait for user request
            wifiSrvcConfig.staConfig.autoConnect = 1;

            // Set SSID 
            memcpy(wifiSrvcConfig.staConfig.ssid, WIFI_DEV_SSID, sizeof(WIFI_DEV_SSID));

            // Set PSK
            memcpy(wifiSrvcConfig.staConfig.psk, WIFI_DEV_PSK, sizeof(WIFI_DEV_PSK));

            // sysObj.syswifi return from SYS_WIFI_Initialize() 
            if (SYS_WIFI_OBJ_INVALID != SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_CONNECT, &wifiSrvcConfig, sizeof(SYS_WIFI_CONFIG)))
            {
            
            }


        Details of SYS_WIFI_GETDRVHANDLE:

            // Get Wi-Fi Driver handle using control message request.
            DRV_HANDLE myWifiDrvHandle;
            if(SYS_WIFI_SUCCESS == SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_GETDRVHANDLE, &myWifiDrvHandle, sizeof(DRV_HANDLE)))
            {
                  //Received the handle
            }

        Details of SYS_WIFI_REGCALLBACK:

            // Client can register multiple callback.Number of supported 
            // callback registration is a MHC configuration.
            SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_REGCALLBACK, WiFiServCallback, sizeof(uint8_t *));

        Details of SYS_WIFI_GETWIFICONFIG:
            
            // Get Wi-Fi Configuration using control message request.
            // The information of configuration is updated in the wifiSrvcConfig.
            
            SYS_WIFI_CONFIG wifiSrvcConfig;
            if(SYS_WIFI_SUCCESS == SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_GETWIFICONFIG, &wifiSrvcConfig, sizeof(SYS_WIFI_CONFIG)))
            {
                  //Received the wifiSrvcConfig data
            } 

            Details of SYS_WIFI_DISCONNECT:
                //In STA mode, device disconnect request using control message. 
                SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_DISCONNECT, NULL, 0);

                // In AP mode, How to disconnect connected STA 
                // When STA connect to AP mode,application received the callback(SYS_WIFI_CONNECT) with connect STA IP and MAC address.
                // User same MAC address for disconnect request.
                SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_DISCONNECT, macAddr, 6);

        </code>

  Remarks:
    None
*/
SYS_WIFI_RESULT SYS_WIFI_CtrlMsg (SYS_MODULE_OBJ object,uint32_t id,void *buffer,uint32_t length );

// *****************************************************************************
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif /* _SYS_WIFI_H  */


