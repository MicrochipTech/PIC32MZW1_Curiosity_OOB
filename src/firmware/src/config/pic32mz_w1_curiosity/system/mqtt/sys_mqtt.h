/*******************************************************************************
  MQTT System Service Interface Header File

  Company
    Microchip Technology Inc.

  File Name
    sys_mqtt.h

  Summary
    MQTT system service interface.

  Description
    This file defines the interface to the Net system service.  This
    system service provides a simple APIs to enable PIC32MZW1 MQTT 
	Functionality.

  Remarks:
    None
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

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

#ifndef SYS_MQTT_H    // Guards against multiple inclusion
#define SYS_MQTT_H

#include <stdlib.h>
#include "definitions.h"
#ifdef SYS_MQTT_ENABLE_DEBUG_PRINT
#include "system/appdebug/sys_appdebug.h"
#endif

extern SYS_MODULE_OBJ g_AppDebugHdl;

// *****************************************************************************
// *****************************************************************************
// Section: Data Types and Constants
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************

// *****************************************************************************
/* System MQTT Instance Status

  Summary:
    Identifies the current status of the Sys Mqtt Instance.

  Remarks:
    None.
*/
typedef enum {
	// Idle
    SYS_MQTT_STATUS_IDLE = 0, 					
    
	// Lower Layer is DOWN
	SYS_MQTT_STATUS_LOWER_LAYER_DOWN, 			
	
	// Net Client connecting to Net Server
    SYS_MQTT_STATUS_SOCK_CLIENT_CONNECTING, 	
	
	// Net Instance connected to the peer
    SYS_MQTT_STATUS_SOCK_CONNECTED, 			
	
	// Net Instance Failed to open socket
	SYS_MQTT_STATUS_SOCK_OPEN_FAILED, 			
	
	// Lower Layer is DOWN
    SYS_MQTT_STATUS_MQTT_CONNECTED, 			
	
	// Net Instance in disconnected state
    SYS_MQTT_STATUS_MQTT_DISCONNECTING, 		
	
	// Net Instance in disconnected state
    SYS_MQTT_STATUS_MQTT_DISCONNECTED, 			
	
	// Lower Layer is DOWN
    SYS_MQTT_STATUS_MQTT_CONN_FAILED,           

	// Wait for Connect Ack from Broker
	SYS_MQTT_STATUS_WAIT_FOR_MQTT_CONACK,

	// Send Mqtt Connect to Broker
	SYS_MQTT_STATUS_SEND_MQTT_CONN,

	// Wait for Subscribe Ack from Broker
	SYS_MQTT_STATUS_WAIT_FOR_MQTT_SUBACK,

	// Wait for Publish Ack from Broker
	SYS_MQTT_STATUS_WAIT_FOR_MQTT_PUBACK,

	// Wait for Unsibscribe Ack from Broker
	SYS_MQTT_STATUS_WAIT_FOR_MQTT_UNSUBACK,
} SYS_MQTT_STATUS;

#define SYS_MQTT_TOPIC_NAME_MAX_LEN            128
#define SYS_MQTT_MAX_BROKER_NAME_LEN           256
#define SYS_MQTT_USER_NAME_MAX_LEN             128
#define SYS_MQTT_PASSWORD_MAX_LEN              128
#define SYS_MQTT_SUB_MAX_TOPICS                2
#define SYS_MQTT_MSG_MAX_LEN                   512
#define SYS_MQTT_CLIENT_ID_MAX_LEN             256

#define MQTT_CFG         0x1
#define MQTT_DATA        0x2
#define MQTT_PAHO        0x4

#define SYS_MQTT_DEFAULT_NET_INTF              0

// *****************************************************************************
/* System MQTT Return values

  Summary:
    Identifies the return values for the Sys Mqtt APIs.

  Remarks:
    None.
*/
typedef enum {    
	// Success
	SYS_MQTT_SUCCESS = 0,	
	
	// Failure
	SYS_MQTT_FAILURE = -1,    
	
	// Sys NET Service Down
	SYS_MQTT_SERVICE_DOWN = -2,	

	// Sys NET Available Put Buffer not enough for xmitting the Data
	SYS_MQTT_SEM_OPERATION_FAILURE = -5,	
	
	// Sys NET Invalid Handle
	SYS_MQTT_INVALID_HANDLE = -6,	
} SYS_MQTT_RESULT;

typedef enum {
	SYS_MQTT_VENDOR_PAHO = 0,			//MQTT - Paho
} SYS_MQTT_Vendor_Type;


// *****************************************************************************
/* System MQTT Broker Configuration

  Summary:
    Used for passing on the configuration related to the MQTT Broker 

  Remarks:
    None.
*/
typedef struct {
	//to know which of the Configurations are valid
	SYS_MQTT_Vendor_Type            eVendorType;	
	
	// MQTT Broker/ Server Name
	char 		brokerName[SYS_MQTT_MAX_BROKER_NAME_LEN]; 
	
	// MQTT Server Port
	uint16_t	serverPort;	
	
	// Keep Alive Interval for the Mqtt Session
    uint16_t    keepAliveInterval;
	
	// MQTT Client ID
	char		clientId[SYS_MQTT_CLIENT_ID_MAX_LEN];	
	
	// MQTT Username
	char		username[SYS_MQTT_USER_NAME_MAX_LEN];	
	
	// MQTT password
	char		password[SYS_MQTT_PASSWORD_MAX_LEN];	
	
	// TLS is Enabled
	bool		tlsEnabled;	
	
	// AutoConnect is Enabled
	bool		autoConnect;	
} SYS_MQTT_BrokerConfig;


// *****************************************************************************
/* System MQTT Subscribe Configuration

  Summary:
    Used for passing on the configuration related to the MQTT Subtopics the user 
	wants to subscribe to.

  Remarks:
    This Configuration is passed via the SYS_MQTT_Connect() function or the
	SYS_MQTT_CtrlMsg() function
*/
typedef struct {
    uint8_t entryValid;
	
	//Qos (0/ 1/ 2)
	uint8_t	qos;	
	
	//Name of the Topic Subscribing to
	char	topicName[SYS_MQTT_TOPIC_NAME_MAX_LEN];	
} SYS_MQTT_SubscribeConfig;


// *****************************************************************************
/* System MQTT Read Published Message

  Summary:
    Used for Reading the message that has been received on a topic subscribed to.
	The structure is also used for passing on the LWT config when connecting to MQTT Broker.

  Remarks:
    This Message is passed to the Application via the SYS_MQTT_CALLBACK() function
*/
typedef struct {
	//Qos (0/ 1/ 2)
	uint8_t	qos;	
	
	//Retain (0/1) - Message needs to be retained by the Broker till every subscriber receives it
	uint8_t	retain;		
	
	//Message to be Published
	uint8_t	message[SYS_MQTT_MSG_MAX_LEN];	
	
	//Message Length
	uint16_t	messageLength;	
	
	//Topic on which to Publish the message
	char	*topicName;	
	
	//Topic Length
	uint16_t	topicLength;	
} SYS_MQTT_PublishConfig;

// *****************************************************************************
/* System MQTT Read Published Message

  Summary:
    Used for publishing a message on a topic. It contains the config related to the Topic

  Remarks:
    This Message is passed from the Application to the MQTT servuce via the SYS_MQTT_Publish() function
*/
typedef struct {
	//Qos (0/ 1/ 2)
	uint8_t	qos;	
	
	//Retain (0/1) - Message needs to be retained by the Broker till every subscriber receives it
	uint8_t	retain;		
	
	//Topic on which to Publish the message
	char	topicName[SYS_MQTT_TOPIC_NAME_MAX_LEN];	
	
	//Topic Length
	uint16_t	topicLength;	
} SYS_MQTT_PublishTopicCfg;

// *****************************************************************************
/* System Mqtt Event Message Type

  Summary:
    Event Message Type which comes with the Callback SYS_MQTT_CALLBACK() 
	informing the user of the event that has occured.

  Remarks:
    None.
*/
typedef enum {
	//Message received on a topic subscribed to
	SYS_MQTT_EVENT_MSG_RCVD = 0,	
	
	//MQTT Client for Disconnected
	SYS_MQTT_EVENT_MSG_DISCONNECTED,	
	
	//MQTT Client Connected
	SYS_MQTT_EVENT_MSG_CONNECTED,	

	//MQTT Client Subscribed to a Grp	
	SYS_MQTT_EVENT_MSG_SUBSCRIBED,	
	
	//MQTT Client UnSubscribed from a Grp
	SYS_MQTT_EVENT_MSG_UNSUBSCRIBED,

	//MQTT Client Published to a Grp	
	SYS_MQTT_EVENT_MSG_PUBLISHED,	
	
	//MQTT Client ConnAck TimeOut
	SYS_MQTT_EVENT_MSG_CONNACK_TO,		
	
	//MQTT Client SubAck TimeOut
	SYS_MQTT_EVENT_MSG_SUBACK_TO,		
	
	//MQTT Client PubAck TimeOut
	SYS_MQTT_EVENT_MSG_PUBACK_TO,		
	
	//MQTT Client PubAck TimeOut
	SYS_MQTT_EVENT_MSG_UNSUBACK_TO,		
} SYS_MQTT_EVENT_TYPE;

// *****************************************************************************
/* System MQTT Instance Configuration

  Summary:
    Used for passing on the configuration related to the either MQTT Broker, 
	or the Cloud Vendors AWS/ Azure, etc.

  Remarks:
    None.
*/
typedef struct {
	//MQTT Broker Configuration
	SYS_MQTT_BrokerConfig           sBrokerConfig;								
	
	//Number of Topis Subscribed to (0-SYS_MQTT_MAX_TOPICS)
	uint8_t							subscribeCount;			
	
	//Config for all the Topics Subscribed to
	SYS_MQTT_SubscribeConfig        sSubscribeConfig[SYS_MQTT_SUB_MAX_TOPICS];	
	
	//If last will and testament(LWT) is enabled or not
    bool                            bLwtEnabled;
	
	// LWT Configuration
    SYS_MQTT_PublishConfig          sLwtConfig;
} SYS_MQTT_Config;

extern const SYS_MQTT_Config 		g_sSysMqttConfig;

// *****************************************************************************
// *****************************************************************************
// Section: Initialization functions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
       int32_t SYS_MQTT_Initialize()

  Summary:
      Returns success/ failure for initialization of data structures of the MQTT service

  Description:
       This function is used for initializing the data structures of the MQTT service and is called from within the System Task.

  Returns:
		SYS_NET_SUCCESS - Indicates the data structures were initialized successfully
		SYS_NET_FAILURE - Indicates that it failed to initialize the data structures.

  Example:
       <code>
		if( SYS_MQTT_Initialize() == SYS_MQTT_SUCCESS)
		{
		}
       </code>

  Remarks:
		If the MQTT system service is enabled using MHC, then auto generated code will take care of its initialization.
  */
int32_t SYS_MQTT_Initialize();

// *****************************************************************************
/* Function:
       void SYS_MQTT_Deinitialize()

  Summary:
      Deinitialization of data structures of the MQTT service

  Description:
       This function is used for freeing the allocated data structures for the MQTT service.

  Example:
       <code>
		SYS_MQTT_Deinitialize()
       </code>

  Remarks:
		None
	   */
int32_t SYS_MQTT_Deinitialize();


// *****************************************************************************
// *****************************************************************************
// Section: Status functions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
       SYS_MQTT_STATUS SYS_MQTT_GetStatus ( SYS_MODULE_OBJ object )

  Summary:
      Returns System MQTT instance status.

  Description:
       This function returns the current status of the System MQTT instance.

  Precondition:
       SYS_MQTT_Connect should have been called before calling this function

  Parameters:
       object  - SYS MQTT object handle, returned from SYS_MQTT_Connect

  Returns:
		SYS_MQTT_STATUS
		
  Example:
       <code>
       // Handle "objSysMqtt" value must have been returned from SYS_MQTT_Connect.
       if (SYS_MQTT_GetStatus (objSysMqtt) == SYS_MQTT_STATUS_WAIT_FOR_MQTT_CONACK)
       {
           // MQTT system service is initialized, and Waiting for the Connect Ack 
		   // from the Broker for the Connect Packet sent by DUT to it.
       }
       </code>

  Remarks:
       None.
  */
SYS_MQTT_STATUS SYS_MQTT_GetStatus(SYS_MODULE_OBJ obj);

// *****************************************************************************
// *****************************************************************************
// Section: Data Exchange functions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
    int32_t SYS_MQTT_Publish(SYS_MODULE_OBJ obj, 
			SYS_MQTT_PublishTopicCfg  *psPubCfg, char *message, uint16_t message_len);

  Summary:
      Returns success/ failure for the publishing of message on a topic by the user.

   Description:
		This function is used for Publishing a message on a Topic.
  
  Precondition:
       SYS_MQTT_Connect should have been called before calling this function

  Parameters:
       obj  - SYS MQTT object handle, returned from SYS_MQTT_Connect <br>
	   psPubCfg		- valid pointer to the Topic details on which to Publish <br>
	   message		- Message to be published <br>
	   message_len  - Message length <br>
	   	     
   Returns:
		SYS_MQTT_SUCCESS - Indicates that the Request was catered to successfully
		SYS_MQTT_FAILURE - Indicates that the Request failed

   Example:
       <code>
	   SYS_MQTT_PublishTopicCfg		sTopicCfg;
	   
	   memset(&sTopicCfg, 0, sizeof(sTopicCfg));
	   sTopicCfg.qos = 1;
	   sTopicCfg.retain = 1;
	   strcpy(sTopicCfg.topicName, "house/temperature/first_floor/kitchen");
	   sTopicCfg.topicLength = strlen("house/temperature/first_floor/kitchen");

       // Handle "objSysMqtt" value must have been returned from SYS_MQTT_Connect.	   
		if( SYS_MQTT_Publish(objSysMqtt, &sPublishCfg, "80.17", strlen("80.17")) == SYS_MQTT_SUCCESS)
		{
		}
		</code>

*/
int32_t	SYS_MQTT_Publish(SYS_MODULE_OBJ obj, SYS_MQTT_PublishTopicCfg  *psPubCfg, char *message, uint16_t message_len);


// *****************************************************************************
/* Function:
    int32_t SYS_MQTT_CALLBACK(SYS_MQTT_EVENT_TYPE eEventType, void *data, uint16_t len, void* cookie);

  Summary:
      Pointer to a MQTT system service callback function.

   Description:
		This data type defines a pointer to a Mqtt service callback function, 
		thus defining the function signature.  Callback functions may be registered 
		by mqtt clients of the Mqtt service via the SYS_MQTT_Connect call.
  
  Precondition:
       Is a part of the Mqtt service Setup using the SYS_MQTT_Connect function

  Parameters:
	eEventType	- event (SYS_MQTT_EVENT_TYPE) - Message Received/ Got Disconnected <br>
	data	- Data (if any) related to the Event <br>
	len		- Length of the Data received <br>
    cookie  	- A context value, returned untouched to the client when the 
					callback occurs.  It can be used to identify the instance of 
					the client who registered the callback.
	   	     
   Returns:
    None.

   Example:
       <code>
    void MqttSrvcCallback(SYS_MQTT_EVENT_TYPE event, void *data, uint16_t len, void* cookie, )
	{
		switch(event)
		{
			case SYS_MQTT_EVENT_MSG_RCVD:
			{
				SYS_MQTT_PublishConfig	*psMsg = (SYS_MQTT_PublishConfig	*)data;
				psMsg->message[psMsg->messageLength] = 0;
				psMsg->topicName[psMsg->topicLength] = 0;
				SYS_CONSOLE_PRINT("\nMqttCallback(): Msg received on Topic: %s ; Msg: %s\r\n", 
					psMsg->topicName, psMsg->message);
				break;
			}

			case SYS_MQTT_EVENT_MSG_DISCONNECT:
			{
				SYS_CONSOLE_PRINT("CloudSrvcCallback(): MQTT DOWN");
				break;
			}
		}
	}
		</code>

*/
typedef int32_t (*SYS_MQTT_CALLBACK)(SYS_MQTT_EVENT_TYPE eEventType, void *data, uint16_t len, void* cookie);

// *****************************************************************************
// *****************************************************************************
// Section: Setup functions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
	SYS_MODULE_OBJSYS_MODULE_OBJ* SYS_MQTT_Connect(SYS_MQTT_Config *cfg, 
													SYS_MQTT_CALLBACK fn MqttFn, 
													void *cookie);

   Summary:
        Connects to the configured MQTT Broker.

   Description:
        This function opens a new instance and connects to the configured 
		MQTT Broker.

   Parameters:
       cfg    		- Configuration based on which the Cloud Service needs to Open<br>

       MqttFn     	- Function pointer to the Callback to be called in case of 
						an event<br>
	   
	   cookie		- Cookie passed as one of the params in the Callback for the user 
						to identify the service instance<br>

   Returns:
        If successful, returns a valid handle to an object. Otherwise, it
        returns SYS_MODULE_OBJ_INVALID.

   Example:
        <code>

		SYS_MQTT_Config    	g_sMqttSrvcCfg;
		SYS_MODULE_OBJ 			g_MqttSrvcHandle;

		memset(&g_sMqttSrvcCfg, 0, sizeof(g_sMqttSrvcCfg));
		
		g_sMattSrvcCfg.configBitmask |= SYS_MQTT_CONFIG_MASK_MQTT;
		
		strcpy(g_sMqttSrvcCfg.mqttConfig.brokerConfig.brokerName, 
				"test.mosquitto.org", strlen("test.mosquitto.org"));
		
		g_sMqttSrvcCfg.mqttConfig.brokerConfig.serverPort = 1883;
		
		strcpy(g_sMqttSrvcCfg.mqttConfig.brokerConfig.clientId, 
			"pic32mzw1", strlen("pic32maw1"));
		
		g_sMqttSrvcCfg.mqttConfig.brokerConfig.autoConnect = 1;
		
		g_sMqttSrvcCfg.mqttConfig.brokerConfig.tlsEnabled = 0;
		
		g_sMqttSrvcCfg.mqttConfig.subscribeCount = 1;
		
		strcpy(g_sMqttSrvcCfg.mqttConfig.subscribeConfig[0].topicName, 
				"house/temperature/first_floor/kitchen", 
				strlen("house/temperature/first_floor/kitchen"));
		
		g_sMqttSrvcCfg.mqttConfig.subscribeConfig[0].qos = 1;
		
		g_MqttSrvcHandle = SYS_MQTT_Connect(&g_sMqttSrvcCfg, MqttSrvcCallback, 0);                
		if (g_MqttSrvcHandle == SYS_MODULE_OBJ_INVALID)
        {
            // Handle error
        }
        </code>

  Remarks:
        This routine should be called only once when the user is configuring 
		the Mqtt service
*/

SYS_MODULE_OBJ SYS_MQTT_Connect(SYS_MQTT_Config *cfg, SYS_MQTT_CALLBACK fn, void *cookie);

// *****************************************************************************
/* Function:
       void SYS_MQTT_Disconnect(SYS_MODULE_OBJ obj)

  Summary:
      Disconnects from the MQTT Server

  Description:
       This function is used for disconnecting from the MQTT Server.

  Precondition:
       SYS_MQTT_Connect should have been called.

  Parameters:
       obj  		- SYS_MQTT object handle, returned from SYS_MQTT_Connect
	   
  Returns:
		None

  Example:
       <code>

       // Handle "objSysMqtt" value must have been returned from SYS_MQTT_Connect.	   
		SYS_MQTT_Disconnect(objSysMqtt);
		</code>

  Remarks:
       None.
  */
void SYS_MQTT_Disconnect(SYS_MODULE_OBJ obj);

/* Function:
    void SYS_MQTT_Task(SYS_MODULE_OBJ obj)

  Summary:
      Executes the MQTT Service State Machine

   Description:
		This function ensures that the MQTT service is able to execute its 
		state machine to process any messages and invoke the user callback 
		for any events.
  
  Precondition:
       SYS_MQTT_Connect should have been called before calling this function

  Parameters:
       obj  - SYS MQTT object handle, returned from SYS_MQTT_Connect

   Returns:
        None

   Example:
        <code>
		// Handle "objSysMqtt" value must have been returned from SYS_MQTT_Connect.
		while(1)
		{
			...
			SYS_MQTT_Task(objSysMqtt);
			...
		}
        </code>

*/
void SYS_MQTT_Task(SYS_MODULE_OBJ obj);


// *****************************************************************************
/* Function:
    int32_t SYS_MQTT_Subscribe(SYS_MODULE_OBJ obj, 
				SYS_MQTT_SubscribeConfig  *subConfig);

  Summary:
      Returns success/ failure for the subscribing to a Topic by the user.

   Description:
		This function is used for subscribing to a Topic.
  
  Precondition:
       SYS_MQTT_Connect should have been called before calling this function

  Parameters:
       obj  - SYS MQTT object handle, returned from SYS_MQTT_Connect <br>
	   subConfig		- valid pointer to the Topic details on which to Subscribe <br>
	   	     
   Returns:
		SYS_MQTT_SUCCESS - Indicates that the Request was catered to successfully
		SYS_MQTT_FAILURE - Indicates that the Request failed

   Example:
       <code>
	   SYS_MQTT_SubscribeConfig		sSubscribeCfg;
	   
	   memset(&sSubscribeCfg, 0, sizeof(sSubscribeCfg));
	   sSubscribeCfg.qos = 1;
	   strcpy(sSubscribeCfg.topicName, "house/temperature/first_floor/kitchen");

       // Handle "objSysMqtt" value must have been returned from SYS_MQTT_Connect.	   
		if( SYS_MQTT_Subscribe(objSysMqtt, &sSubscribeCfg) == SYS_MQTT_SUCCESS)
		{
		}
		</code>

*/
int32_t	SYS_MQTT_Subscribe(SYS_MODULE_OBJ obj, SYS_MQTT_SubscribeConfig  *subConfig);

// *****************************************************************************
/* Function:
    int32_t SYS_MQTT_Unsubscribe(SYS_MODULE_OBJ obj, char  *subTopic);

  Summary:
      Returns success/ failure for the unsubscribing to a Topic by the user.

   Description:
		This function is used for Unsubscribing from a Topic.
  
  Precondition:
       SYS_MQTT_Connect should have been called before calling this function

  Parameters:
       obj  - SYS MQTT object handle, returned from SYS_MQTT_Connect <br>
	   subtopic		- Topic from which to unsubscribe <br>
	   	     
   Returns:
		SYS_MQTT_SUCCESS - Indicates that the Request was catered to successfully
		SYS_MQTT_FAILURE - Indicates that the Request failed

   Example:
       <code>
       // Handle "objSysMqtt" value must have been returned from SYS_MQTT_Connect.	   
		if( SYS_MQTT_Unsubscribe(objSysMqtt, "house/temperature/first_floor/kitchen") == SYS_MQTT_SUCCESS)
		{
		}
		</code>

*/
int32_t	SYS_MQTT_Unsubscribe(SYS_MODULE_OBJ obj, char *subTopic);

#ifndef SYS_MQTT_ENABLE_DEBUG_PRINT
#define SYS_MQTTDEBUG_DBG_PRINT(obj, flow, fmt, ...)
#define SYS_MQTTDEBUG_INFO_PRINT(obj, flow, fmt, ...)
#define SYS_MQTTDEBUG_FN_ENTER_PRINT(obj, flow)
#define SYS_MQTTDEBUG_FN_EXIT_PRINT(obj, flow)
#define SYS_MQTTDEBUG_ERR_PRINT(obj, flow, fmt, ...) SYS_CONSOLE_Print(SYS_CONSOLE_DEFAULT_INSTANCE, fmt, ##__VA_ARGS__)
#else
#define SYS_MQTTDEBUG_DBG_PRINT      SYS_APPDEBUG_DBG_PRINT
#define SYS_MQTTDEBUG_INFO_PRINT     SYS_APPDEBUG_INFO_PRINT
#define SYS_MQTTDEBUG_FN_ENTER_PRINT     SYS_APPDEBUG_FN_ENTER_PRINT
#define SYS_MQTTDEBUG_FN_EXIT_PRINT      SYS_APPDEBUG_FN_EXIT_PRINT
#define SYS_MQTTDEBUG_ERR_PRINT      SYS_APPDEBUG_ERR_PRINT
#endif

#endif //SYS_MQTT_H
