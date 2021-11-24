/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony MQTT application.

  Description:
    This file contains the source code for the MPLAB Harmony MQTT application.
    It implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/
/*******************************************************************************
Copyright (C) 2021 released Microchip Technology Inc.  All rights reserved.

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "system/mqtt/sys_mqtt.h"
// *****************************************************************************
// *****************************************************************************
// Section: Declarations
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
volatile APP_DATA g_appData;

SYS_MODULE_OBJ g_sSysMqttHandle = SYS_MODULE_OBJ_INVALID;
SYS_MQTT_Config g_sTmpSysMqttCfg;

//#define APP_CFG_WITH_MQTT_API

// *****************************************************************************
// *****************************************************************************
// Section: Local data
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/******************************************************************************
  Function:
    int32_t MqttCallback(SYS_MQTT_EVENT_TYPE eEventType, void *data, uint16_t len, void* cookie)

  Remarks:
    Callback function registered with the SYS_MQTT_Connect() API. For more details 
	check https://microchip-mplab-harmony.github.io/wireless/system/mqtt/docs/interface.html
 */
int32_t MqttCallback(SYS_MQTT_EVENT_TYPE eEventType, void *data, uint16_t len, void* cookie) {
    switch (eEventType) {
        case SYS_MQTT_EVENT_MSG_RCVD:
        {
			/* Message received on Subscribed Topic */
            /*
            SYS_MQTT_PublishConfig	*psMsg = (SYS_MQTT_PublishConfig	*)data;
            psMsg->message[psMsg->messageLength] = 0;
            psMsg->topicName[psMsg->topicLength] = 0;
            SYS_CONSOLE_PRINT("\nMqttCallback(): Msg received on Topic: %s ; Msg: %s\r\n", 
                psMsg->topicName, psMsg->message);
             */
        }
            break;

        case SYS_MQTT_EVENT_MSG_DISCONNECTED:
        {
        }
            break;

        case SYS_MQTT_EVENT_MSG_CONNECTED:
        {
			SYS_CONSOLE_PRINT("\nMqttCallback(): Connected\r\n");
            /*
			char message[32] = {0};
			strcpy(message, "Hello");
			APP_MQTT_PublishMsg(message);
             */
        }
            break;

        case SYS_MQTT_EVENT_MSG_SUBSCRIBED:
        {
            /*
            SYS_MQTT_SubscribeConfig	*psMqttSubCfg = (SYS_MQTT_SubscribeConfig	*)data;
            SYS_CONSOLE_PRINT("\nMqttCallback(): Subscribed to Topic '%s'\r\n", psMqttSubCfg->topicName);
             */
        }
            break;

        case SYS_MQTT_EVENT_MSG_UNSUBSCRIBED:
        {
			/* MQTT Topic Unsubscribed; Now the Client will not receive any messages for this Topic */
        }
            break;

        case SYS_MQTT_EVENT_MSG_PUBLISHED:
        {
			/* MQTT Client Msg Published */
        }
            break;

        case SYS_MQTT_EVENT_MSG_CONNACK_TO:
        {
			/* MQTT Client ConnAck TimeOut; User will need to reconnect again */
        }
            break;

        case SYS_MQTT_EVENT_MSG_SUBACK_TO:
        {
			/* MQTT Client SubAck TimeOut; User will need to subscribe again */
        }
            break;

        case SYS_MQTT_EVENT_MSG_PUBACK_TO:
        {
			/* MQTT Client PubAck TimeOut; User will need to publish again */
        }
            break;

        case SYS_MQTT_EVENT_MSG_UNSUBACK_TO:
        {
			/* MQTT Client UnSubAck TimeOut; User will need to Unsubscribe again */
        }
            break;

    }
    return SYS_MQTT_SUCCESS;
}

/*******************************************************************************
  Function:
    void APP_MQTT_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_MQTT_Initialize(void) {

	/*
	** For more details check https://microchip-mplab-harmony.github.io/wireless/system/mqtt/docs/interface.html
	*/
#ifdef APP_CFG_WITH_MQTT_API

	/* In case the user does not want to use the configuration given in the MHC */
	
    SYS_MQTT_Config *psMqttCfg;

    memset(&g_sTmpSysMqttCfg, 0, sizeof (g_sTmpSysMqttCfg));
    psMqttCfg = &g_sTmpSysMqttCfg;
    psMqttCfg->sBrokerConfig.autoConnect = true;
    psMqttCfg->sBrokerConfig.tlsEnabled = false;
    strcpy(psMqttCfg->sBrokerConfig.brokerName, "test.mosquitto.org");
    psMqttCfg->sBrokerConfig.serverPort = 1883;
    psMqttCfg->sBrokerConfig.cleanSession = false;
    psMqttCfg->sBrokerConfig.keepAliveInterval = 60;
    psMqttCfg->subscribeCount = 0; 
    g_sSysMqttHandle = SYS_MQTT_Connect(&g_sTmpSysMqttCfg, MqttCallback, NULL);
#else    
    g_sSysMqttHandle = SYS_MQTT_Connect(NULL, /* NULL value means that the MHC configuration should be used for this connection */
										MqttCallback, 
										NULL);
#endif    
}

/******************************************************************************
  Function:
    void APP_MQTT_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_MQTT_Tasks(void) {

    SYS_MQTT_Task(g_sSysMqttHandle);
}

/******************************************************************************
  Function:
    int32_t APP_MQTT_GetStatus ( void *)

  Remarks:
    See prototype in app.h.
 */
int32_t APP_MQTT_GetStatus(void *p) {

    return SYS_MQTT_GetStatus(g_sSysMqttHandle);
}


/*******************************************************************************
 End of File
 */
