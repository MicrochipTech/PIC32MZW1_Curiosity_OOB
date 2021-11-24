/*******************************************************************************
  Wi-Fi Provision System Service Implementation

  File Name
    sys_wifiprov.h

  Summary
    Wi-Fi Provisioning system service interface.

  Description
    This file defines the interface to the Wi-Fi Provisioning system service.This
    system service provides a simple APIs to enable PIC32MZW1 Wi-Fi Provisioning 
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



#ifndef _SYS_WIFIPROV_H 
#define _SYS_WIFIPROV_H 

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
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* System Wi-Fi Provisioning service Authentication types

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
    SYS_WIFIPROV_OPEN = 1,

    /* Requesting a WEP Authentication types */
    SYS_WIFIPROV_WEP,

    /* Requesting a WPA/WPA2(Mixed) Authentication types */
    SYS_WIFIPROV_WPAWPA2MIXED,     

    /* Requesting a WPA2 Authentication types */
    SYS_WIFIPROV_WPA2,

    /* Requesting a WPA2/WPA3(Mixed) Authentication types */
    SYS_WIFIPROV_WPA2WPA3MIXED,

    /* Requesting a WPA3 Authentication types */
    SYS_WIFIPROV_WPA3

} SYS_WIFIPROV_AUTH ;

// *****************************************************************************
/* System Wi-Fi Provisioning service control message types

  Summary:
    Identifies the control message for which the client has called 
    the SYS_WIFIPROV_CtrlMsg().

  Description:
    Identifies the control message for which the client has called 
    the SYS_WIFIPROV_CtrlMsg().

  Remarks:
   The different control messages which can be invoked by the client.
*/

typedef enum 
{
    /* Requesting a Wi-Fi Configuration set(for connect) */
    SYS_WIFIPROV_SETCONFIG = 0,

    /* Requesting a Wi-Fi configuration get */
    SYS_WIFIPROV_GETCONFIG,

    /* Updating Wi-Fi Connect status for enabling Wi-Fi Provisioning service */
    SYS_WIFIPROV_CONNECT,        

} SYS_WIFIPROV_CTRLMSG ;

// *****************************************************************************
/* System Wi-Fi Provisioning service operating  modes

  Summary:
    Identifies the Wi-Fi operating mode.

  Description:
    Identifies the Wi-Fi operating mode.

  Remarks:
   Client need to manually reboot device after switching mode.
   For example changing operating mode from STA to AP or AP to STA.
*/

typedef enum 
{
    /* Requesting a operating mode as a station */
    SYS_WIFIPROV_STA = 0,
    
} SYS_WIFIPROV_MODE ;

// *****************************************************************************
/* System Wi-Fi Provisioning service station mode configuration structure.

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
    SYS_WIFIPROV_AUTH authType;

    /* Wi-Fi station mode channel number.
       values of channel:  
       0 - scan and connect to all the channels
       1 to 13 - - scan and connect to specified channel */
    uint8_t channel;

    /* Wi-Fi station mode auto connect flag. 
       value 0- Don't connect to AP, wait for client request.
       value 1- Connect to AP */
    bool autoConnect;

} SYS_WIFIPROV_STA_CONFIG;


// *****************************************************************************
/* System Wi-Fi Provisioning service device configuration structure.

  Summary:
    Configuration of device configuration parameters.

  Description:
    Configuration of device configuration parameters.

  Remarks:
   None.
*/

typedef struct 
{
    /* Operating mode of device */
    SYS_WIFIPROV_MODE mode;

    /* Flag to identify if configuration needs to be saved in NVM.
       0 - Do not save configuration in NVM.
       1 - Save configuration in NVM. */
    uint8_t saveConfig;

    /* Country Code configuration */
    uint8_t countryCode[6];

    /* Wi-Fi station mode configuration */
    SYS_WIFIPROV_STA_CONFIG staConfig;

}SYS_WIFIPROV_CONFIG;

// *****************************************************************************
/* System Wi-Fi service Provisioning status .

  Summary:
    Result of a Wi-Fi Provisioning system service client interface get
    operation(SYS_WIFIPROV_GetStatus()).

  Description:
    Result of a Wi-Fi Provisioning system service client interface get
    operation(SYS_WIFIPROV_GetStatus()).

  Remarks:
   None.
*/
typedef enum
{
    /* Wi-Fi Provisioning system service is in client request state */
    SYS_WIFIPROV_STATUS_WAITFORREQ,

    /*Wi-Fi Provisioning system service is in invalid state */
    SYS_WIFIPROV_STATUS_NONE =255

} SYS_WIFIPROV_STATUS;

// *****************************************************************************
/* System Wi-Fi Provisioning Result.

  Summary:
    Result of a Wi-Fi Provisioning system service client interface operation.

  Description:
    Identifies the result of  Wi-Fi Provisioning service operations

  Remarks:
   None.
*/

typedef enum{
    
    /* Operation completed with success */
    SYS_WIFIPROV_SUCCESS = 0,

    /* Operation failed. */
    SYS_WIFIPROV_FAILURE,

    /* Operation request object is invalid */
    SYS_WIFIPROV_OBJ_INVALID=255

}SYS_WIFIPROV_RESULT;

// *****************************************************************************
/* Function:
    typedef void (*SYS_WIFIPROV_CALLBACK )
    (
        uint32_t event, 
        void * data,
        void *cookie 
    )

   Summary:
    Pointer to a Wi-Fi Provisioning system service callback function.

   Description:
    This data type defines a pointer to a Wi-Fi Provisioning service callback function.
    Callback functions can be registered by client at initializing.

   Precondition:
    None

   Parameters:
    event    - A event value, event can be any  of SYS_WIFIPROV_CTRLMSG types.
    data     - Wi-Fi Provisioning service Data.
    cookie   - Client register cookie.

   Returns:
    None.

  Example:
    <code>

    void WiFiProvServCallback (uint32_t event, void * data,void *cookie )
    {
        switch(event)
        {
            case SYS_WIFIPROV_SETCONFIG:
            {
                SYS_WIFIPROV_CONFIG* wifiProvConfig = (SYS_WIFIPROV_CONFIG *) data;
                // Provisioning service  updated data 
                SYS_CONSOLE_PRINT("%s:%d Device mode=%d\\r\\n",__func__,__LINE__,wifiProvConfig->mode);
                break;
            }

            case SYS_WIFIPROV_GETCONFIG:
            {
                SYS_WIFIPROV_CONFIG* wifiProvConfig = (SYS_WIFIPROV_CONFIG *) data;
                // client requested get Wi-Fi Configuration 
                SYS_CONSOLE_PRINT("%s:%d Device mode=%d\\r\\n",__func__,__LINE__,wifiProvConfig->mode);
                break;
           }
        }
    }
    </code>

  Remarks:
    None.
*/
typedef void (*SYS_WIFIPROV_CALLBACK )(uint32_t event, void * data,void *cookie );

// *****************************************************************************
// *****************************************************************************
// Section: Initialization functions
// *****************************************************************************
// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ SYS_WIFIPROV_Initialize
    (
        SYS_WIFIPROV_CONFIG *config,
        SYS_WIFIPROV_CALLBACK callback,
        void *cookie
    )

   Summary:
        Initializes the System Wi-Fi Provisioning module.

   Description:
        Wi-Fi Provisioning service supports only single instance.

   Parameters:
       config    - Wi-Fi Provisioning device configuration structure.
       callback  - The client callback function pointer.
       cookie    - The pointer which will be passed to the client application 
                   when the client callback function is invoked.

   Returns:
        If successful, returns a valid handle to an object. Otherwise, it
        returns SYS_MODULE_OBJ_INVALID.

   Example:
        <code>
        #define WIFI_DEV_SSID  "DEMO_AP"
        #define WIFI_DEV_PSK   "password"

        SYS_WIFIPROV_CONFIG    wifiProvConfig;
        SYS_MODULE_OBJ         wifiProvServHandle;

        // Set mode as STA 
        wifiProvConfig.mode = SYS_WIFI_STA;

        // Disable saving wifi configuration
        wifiProvConfig.saveConfig = false;

        //Set the auth type to SYS_WIFI_WPA2
        wifiProvConfig.staConfig.authType = SYS_WIFI_WPA2;

        // Enable all the channels(0)
        wifiProvConfig.staConfig.channel = 0;
        
        // Device doesn't wait for user request.
        wifiProvConfig.staConfig.autoConnect = 1;
        
        // Set SSID
        memcpy(wifiProvConfig.staConfig.ssid,WIFI_DEV_SSID,sizeof(WIFI_DEV_SSID));

        // Set PSK 
        memcpy(wifiProvConfig.staConfig.psk,WIFI_DEV_PSK,sizeof(WIFI_DEV_PSK));

        wifiProvServHandle = SYS_WIFIPROV_Initialize(&wifiProvConfig, WiFiProvServCallback, 0);
        if (wifiProvServHandle == SYS_MODULE_OBJ_INVALID)
        {
            // Handle error
        }
        </code>


  Remarks:
        Client can auto enable the Provisioning service functionality by selecting MHC configuration option of Wi-Fi Service. 
*/

SYS_MODULE_OBJ SYS_WIFIPROV_Initialize(SYS_WIFIPROV_CONFIG *config,SYS_WIFIPROV_CALLBACK callback,void *cookie);

// *****************************************************************************
/* Function:
   SYS_WIFIPROV_RESULT SYS_WIFIPROV_Deinitialize (SYS_MODULE_OBJ object)

  Summary:
       Deinitializes the module instance of the SYS WIFIPROV module

  Description:
       This function deinitializes the module instance disabling its
       operation. Resets all of the internal data structures and fields 
       to the default settings.

  Precondition:
       The SYS_WIFIPROV_Initialize function should have been called before calling
       this function.

  Parameters:
       object   - SYS WIFIPROV object handle, returned from SYS_WIFIPROV_Initialize

  Returns:
       return SYS_WIFIPROV_RESULT 

  Example:
        <code>
        
         if (SYS_WIFI_SUCCESS == SYS_WIFIPROV_Deinitialize (wifiProvServHandle))
        {
            // when the SYS WIFI is De-initialized.
        }
        </code>

  Remarks:
        Deinitialize should be called if the WiFi Provisioning service is no longer going to be used.
*/

SYS_WIFIPROV_RESULT SYS_WIFIPROV_Deinitialize (SYS_MODULE_OBJ object) ;

// *****************************************************************************
// *****************************************************************************
// Section: Status functions
// *****************************************************************************
// *****************************************************************************
/* Function:
   uint8_t SYS_WIFIPROV_GetStatus ( SYS_MODULE_OBJ object)

  Summary:
        Returns System Wi-Fi Provisioning service status.

  Description:
    This function returns the current status of the System Wi-Fi Provisioning service.

  Precondition:
       The SYS_WIFIPROV_Initialize function should have been called before calling
       this function.

  Parameters:
       object   - SYS WIFIPROV object handle, returned from SYS_WIFIPROV_Initialize

  Returns:
       return SYS_WIFIPROV_STATUS if client provided object is valid, else return SYS_WIFIPROV_OBJ_INVALID.

  Example:
        <code>
        
         if (SYS_WIFIPROV_STATE_WAITFORREQ == SYS_WIFIPROV_GetStatus (wifiProvServHandle))
        {
            // when the SYS WIFI Provisioning module in wait for client request
        }
        </code>

  Remarks:
    None
*/
uint8_t SYS_WIFIPROV_GetStatus ( SYS_MODULE_OBJ object) ;

// *****************************************************************************
// *****************************************************************************
// Section: Setup functions
// *****************************************************************************
// *****************************************************************************
/* Function:
   uint8_t SYS_WIFIPROV_Tasks ( SYS_MODULE_OBJ object)

  Summary:
    Maintains the Wi-Fi Provisioning System tasks and functionalities. 

  Description:
    This function is used to run the various tasks and functionalities of Wi-Fi Provisioning system service.

  Precondition:
       The SYS_WIFIPROV_Initialize function should have been called before calling
       this function.

  Parameters:
       object   - SYS WIFI Provisioning object handle, returned from SYS_WIFIPROV_Initialize

  Returns:
       return SYS_WIFIPROV_STATUS if client provided object is valid, else return SYS_WIFIPROV_OBJ_INVALID.

  Example:
        <code>
        
         if (SYS_WIFIPROV_OBJ_INVALID != SYS_WIFIPROV_Tasks (wifiProvServHandle))
        {
            
        }
        </code>

  Remarks:
    None
*/

uint8_t SYS_WIFIPROV_Tasks (SYS_MODULE_OBJ object);

// *****************************************************************************
/* Function:
   SYS_WIFIPROV_RESULT SYS_WIFIPROV_CtrlMsg (SYS_MODULE_OBJ object,uint32_t event,void *buffer,uint32_t length )

  Summary:
    Request Wi-Fi  Provisioning system service control request interface

  Description:
    This function is used to make control request to Wi-Fi Provisioning system service.

  Precondition:
       The SYS_WIFIPROV_Initialize function should have been called before calling
       this function.

  Parameters:
       object   - SYS WIFIPROV object handle, returned from SYS_WIFIPROV_Initialize
       event    - A event value, event can be any  of SYS_WIFIPROV_CTRLMSG types
       buffer   - Control message data input.
       length   - size of buffer data


  Returns:
       return SYS_WIFIPROV_RESULT.

  Example:
        <code>
        Details of SYS_WIFIPROV_SETCONFIG:

            SYS_WIFIPROV_CONFIG    wifiProvConfig;
            SYS_MODULE_OBJ         wifiProvServHandle;

            // Set mode as STA 
            wifiProvConfig.mode = SYS_WIFI_STA;

            // Disable saving wifi configuration 
            wifiProvConfig.saveConfig = false;

            // Set the auth type to SYS_WIFI_WPA2 
            wifiProvConfig.staConfig.authType = SYS_WIFI_WPA2;

            // Enable all the channels(0)
            wifiProvConfig.staConfig.channel = 0;

            // Device doesn't wait for user request
            wifiProvConfig.staConfig.autoConnect = 1;

            // Set SSID 
            memcpy(wifiProvConfig.staConfig.ssid,WIFI_DEV_SSID,sizeof(WIFI_DEV_SSID));

            // Set PSK 
            memcpy(wifiProvConfig.staConfig.psk,WIFI_DEV_PSK,sizeof(WIFI_DEV_PSK));
            if (SYS_WIFIPROV_OBJ_INVALID != SYS_WIFIPROV_CtrlMsg (wifiProvServHandle,SYS_WIFIPROV_SETCONFIG,&wifiProvConfig,sizeof(SYS_WIFIPROV_CONFIG)))
            {
                // When Wi-Fi Provisioning Configuration need to be updated 
            }

        Details of SYS_WIFIPROV_GETCONFIG:
            SYS_WIFIPROV_CtrlMsg (wifiProvServHandle,SYS_WIFIPROV_GETCONFIG,NULL,0);

        Details of SYS_WIFIPROV_CONNECT:
            // Updating Wi-Fi Connected state to Provisioning service 
            bool wifiProvConnectState = true;
            SYS_WIFIPROV_CtrlMsg (wifiProvServHandle,SYS_WIFIPROV_CONNECT,&wifiProvConnectState,sizeof(wifiProvConnectState));

            // Updating Wi-Fi disconnected state to Provisioning service 
            bool wifiProvConnectState = false;
            SYS_WIFIPROV_CtrlMsg (wifiProvServHandle,SYS_WIFIPROV_CONNECT,&wifiProvConnectState,sizeof(wifiProvConnectState));


        </code>

  Remarks:
    None
*/

SYS_WIFIPROV_RESULT SYS_WIFIPROV_CtrlMsg (SYS_MODULE_OBJ object,uint32_t event,void *buffer,uint32_t length );

// *****************************************************************************

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //_SYS_WIFIPROV_H

