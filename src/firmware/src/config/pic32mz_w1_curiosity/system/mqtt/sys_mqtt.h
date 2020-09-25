#ifndef SYS_MQTT_H    // Guards against multiple inclusion
#define SYS_MQTT_H

#include <stdlib.h>
#include "definitions.h"
#ifdef FREERTOS
#include "osal/osal_freertos.h"
#else
#include "osal/osal_impl_basic.h"
#endif
#include "tcpip/tcpip.h"
#include "third_party/paho.mqtt.embedded-c/MQTTClient-C/Platforms/MCHP_pic32mzw1.h" 
#include "third_party/paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.h" 
#include "system/appdebug/sys_appdebug.h"

extern SYS_MODULE_OBJ g_AppDebugHdl;

typedef enum {
    SYS_MQTT_STATUS_IDLE = 0, 					///< Idle
    SYS_MQTT_STATUS_LOWER_LAYER_DOWN, 			///< Lower Layer is DOWN
    SYS_MQTT_STATUS_SOCK_CLIENT_CONNECTING, 			///< Net Client connecting to Net Server
    SYS_MQTT_STATUS_SOCK_CONNECTED, 					///< Net Instance connected to the peer
	SYS_MQTT_STATUS_SOCK_OPEN_FAILED, 			///< Net Instance Failed to open socket
    SYS_MQTT_STATUS_MQTT_CONNECTED, 			///< Lower Layer is DOWN
    SYS_MQTT_STATUS_MQTT_DISCONNECTING, 				///< Net Instance in disconnected state
    SYS_MQTT_STATUS_MQTT_DISCONNECTED, 				///< Net Instance in disconnected state
    SYS_MQTT_STATUS_MQTT_CONN_FAILED,               ///< Lower Layer is DOWN
	SYS_MQTT_STATUS_WAIT_FOR_MQTT_CONACK,
	SYS_MQTT_STATUS_SEND_MQTT_CONN,
	SYS_MQTT_STATUS_WAIT_FOR_MQTT_SUBACK,
	SYS_MQTT_STATUS_WAIT_FOR_MQTT_PUBACK,
	SYS_MQTT_STATUS_WAIT_FOR_MQTT_UNSUBACK,
} SYS_MQTT_STATUS;

#define SYS_MQTT_TOPIC_NAME_MAX_LEN            256
#define SYS_MQTT_MAX_BROKER_NAME_LEN           256
#define SYS_MQTT_SUB_MAX_TOPICS                2
#define SYS_MQTT_MSG_MAX_LEN                   512
#define SYS_MQTT_CLIENT_ID_MAX_LEN             256

/* App Debug Print Flows */
#define MQTT_CFG         0x1
#define MQTT_DATA        0x2
#define MQTT_PAHO        0x4

#define SYS_MQTT_DEFAULT_NET_INTF              0
/**
 * @brief
	System NET Return values
    Identifies the return values for the Sys Net APIs.
*/
typedef enum {    
	SYS_MQTT_SUCCESS = 0,	///< Success
	SYS_MQTT_FAILURE = -1,    ///< Failure
	SYS_MQTT_SERVICE_DOWN = -2,	///< Sys NET Service Down
	SYS_MQTT_SEM_OPERATION_FAILURE = -5,	///< Sys NET Available Put Buffer not enough for xmitting the Data
	SYS_MQTT_INVALID_HANDLE = -6,	///< Sys NET Invalid Handle
} SYS_MQTT_RESULT;

typedef enum {
	SYS_MQTT_VENDOR_PAHO = 0,			//MQTT - Paho
} SYS_MQTT_Vendor_Type;


// *****************************************************************************
/* System Cloud MQTT Broker Configuration

  Summary:
    Used for passing on the configuration related to the MQTT Broker 

  Remarks:
    None.
*/
typedef struct {
	SYS_MQTT_Vendor_Type            eVendorType;	//to know which of the Configurations are valid
	char 		brokerName[SYS_MQTT_MAX_BROKER_NAME_LEN]; // MQTT Broker/ Server Name
	uint16_t	serverPort;	// MQTT Server Port
    uint16_t    keepAliveInterval;
	char		clientId[SYS_MQTT_CLIENT_ID_MAX_LEN];	// MQTT Client ID
	bool		tlsEnabled;	// TLS is Enabled
	bool		autoConnect;	// AutoConnect is Enabled
} SYS_MQTT_BrokerConfig;


// *****************************************************************************
/* System Cloud MQTT Subscribe Configuration

  Summary:
    Used for passing on the configuration related to the MQTT Subtopics the user 
	wants to subscribe to.

  Remarks:
    This Configuration is passed via the SYS_MQTT_Connect() function or the
	SYS_MQTT_CtrlMsg() function
*/
typedef struct {
    uint8_t entryValid;
	uint8_t	qos;	//Qos (0/ 1/ 2)
	char	topicName[SYS_MQTT_TOPIC_NAME_MAX_LEN];	//Name of the Topic Subscribing to
} SYS_MQTT_SubscribeConfig;


// *****************************************************************************
/* System Cloud MQTT Publish Message

  Summary:
    Used for Publishing message onto a  MQTT Subtopics.

  Remarks:
    This Message is passed via the SYS_MQTT_CtrlMsg() function
*/
typedef struct {
	uint8_t	qos;	//Qos (0/ 1/ 2)
	uint8_t	retain;		//Retain (0/1) - Message needs to be retained by the Broker till every subscriber receives it
	uint8_t	message[SYS_MQTT_MSG_MAX_LEN];	//Message to be Published
	uint16_t	messageLength;	//Message Length
	char	*topicName;	//Topic on which to Publish the message
	uint16_t	topicLength;	//Topic Length
} SYS_MQTT_PublishConfig;

typedef struct {
	uint8_t	qos;	//Qos (0/ 1/ 2)
	uint8_t	retain;		//Retain (0/1) - Message needs to be retained by the Broker till every subscriber receives it
	char	topicName[SYS_MQTT_TOPIC_NAME_MAX_LEN];	//Topic on which to Publish the message
	uint16_t	topicLength;	//Topic Length
} SYS_MQTT_PublishTopicCfg;

// *****************************************************************************
/* System Cloud Event Message Type

  Summary:
    Event Message Type which comes with the Callback SYS_MQTT_CtrlMsg() for performing various 
	MQTT/ AWS/ Azure Operations.

  Remarks:
    None.
*/
typedef enum {
	SYS_MQTT_EVENT_MSG_RCVD = 0,	//Message received on a topic subscribed to
	SYS_MQTT_EVENT_MSG_DISCONNECTED,	//MQTT Client for Disconnected
	SYS_MQTT_EVENT_MSG_CONNECTED,		//MQTT Client Connected
	SYS_MQTT_EVENT_MSG_SUBSCRIBED,	//MQTT Client Subscribed to a Grp
	SYS_MQTT_EVENT_MSG_UNSUBSCRIBED,	//MQTT Client UnSubscribed from a Grp
	SYS_MQTT_EVENT_MSG_PUBLISHED,	//MQTT Client Published to a Grp
	SYS_MQTT_EVENT_MSG_CONNACK_TO,		//MQTT Client ConnAck TimeOut
	SYS_MQTT_EVENT_MSG_SUBACK_TO,		//MQTT Client SubAck TimeOut
	SYS_MQTT_EVENT_MSG_PUBACK_TO,		//MQTT Client PubAck TimeOut
	SYS_MQTT_EVENT_MSG_UNSUBACK_TO,		//MQTT Client PubAck TimeOut
} SYS_MQTT_EVENT_TYPE;

// *****************************************************************************
/* Function:
    int32 (*SYS_MQTT_CALLBACK)(SYS_MQTT_EVENT_TYPE eEventType, 
								void *data, 
								uint16_t len, 
								void* cookie);

   Summary:
    Pointer to a cloud system service callback function.

   Description:
    This data type defines a pointer to a udp service callback function, thus
    defining the function signature.  Callback functions may be registered by
    mqtt clients of the cloud service via the Initialize call.

   Precondition:
    Is a part of the cloud service initialization using the SYS_MQTT_Connect
    function.

   Parameters:
	eEventType	- event (SYS_MQTT_EVENT_TYPE) - Message Received/ Got Disconnected
	data	- Data (if any) related to the Event
	len		- Length of the Data received
    cookie  	- A context value, returned untouched to the client when the
                 callback occurs.  It can be used to identify the instance of
                 the client who registered the callback.
	

   Returns:
    None.

  Example:
    <code>
    void CloudSrvcCallback(SYS_MQTT_EVENT_TYPE event, void *data, uint16_t len, void* cookie, )
	{
		switch(event)
		{
			case SYS_MQTT_EVENT_MSG_RCVD:
			{
				SYS_MQTT_EventData	*mqttEvent = (SYS_MQTT_EventData	*)data;
				SYS_CONSOLE_PRINT("CloudSrvcCallback(): Received message - %s on Topic %s", mqttEvent.message, mqttEvent.topicName);
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

  Remarks:
    None.
*/
typedef int32_t (*SYS_MQTT_CALLBACK)(SYS_MQTT_EVENT_TYPE eEventType, void *data, uint16_t len, void* cookie);

// *****************************************************************************
/* System Cloud Configuration

  Summary:
    Used for passing on the configuration related to the either MQTT Broker, or the Cloud Vendors 
	AWS/ Azure.

  Remarks:
    None.
*/
typedef struct {
	SYS_MQTT_BrokerConfig           sBrokerConfig;								//MQTT Broker Configuration
	uint8_t							subscribeCount;								//Number of Topis Subscribed to (0-SYS_MQTT_MAX_TOPICS)
	SYS_MQTT_SubscribeConfig        sSubscribeConfig[SYS_MQTT_SUB_MAX_TOPICS];	//Config for all the Topics Subscribed to
    bool                            bLwtEnabled;
    SYS_MQTT_PublishConfig          sLwtConfig;
} SYS_MQTT_Config;

extern const SYS_MQTT_Config 		g_sSysMqttConfig;

#ifdef SYS_MQTT_PAHO

#define SYS_MQTT_PAHO_MAX_TX_BUFF_LEN  512
#define SYS_MQTT_PAHO_MAX_RX_BUFF_LEN  512
typedef struct {
    Network sPahoNetwork;
    MQTTClient sPahoClient;
    uint8_t         subscribeCount;
 	SYS_MQTT_PublishConfig   sPubSubCfgInProgress;
	unsigned char   sendbuf[SYS_MQTT_PAHO_MAX_TX_BUFF_LEN];
    unsigned char   recvbuf[SYS_MQTT_PAHO_MAX_RX_BUFF_LEN];
} SYS_MQTT_PahoInfo;
#endif

typedef union {
#ifdef SYS_MQTT_PAHO
    SYS_MQTT_PahoInfo  sPahoInfo;
#endif    
} SYS_MQTT_VendorInfo;

typedef struct {
    uint32_t    startTime;
    uint32_t    timeOut;
}SYS_MQTT_TimerInfo;

typedef struct {
	SYS_MQTT_Config         sCfgInfo;
	SYS_MQTT_CALLBACK    	callback_fn;	/* callback registered with the service */
	void *					vCookie;		/* Cookie to Identify the Instance by the User */
    OSAL_SEM_HANDLE_TYPE    InstSemaphore;	/* Semaphore for Critical Section */
    SYS_MQTT_VendorInfo     uVendorInfo;    /* Info related to - Paho/ AWS/ Azure */
    SYS_MODULE_OBJ          netSrvcHdl;
	SYS_MQTT_STATUS         eStatus;		/* Current state of the service */
    SYS_MQTT_TimerInfo      timerInfo;
} SYS_MQTT_Handle;

// *****************************************************************************
/* Function:
	SYS_MODULE_OBJSYS_MODULE_OBJ *SYS_MQTT_Connect(SYS_MQTT_Config *cfg, 
													SYS_MQTT_CALLBACK fn cloudFn, 
													void *cookie);

   Summary:
        Initializes the System Cloud service.

   Description:
        This function initializes the instance of the System Cloud Service.

   Parameters:
       cfg    		- Configuration based on which the Cloud Service needs to Open

       cloudFn     	- Function pointer to the Callback to be called in case of an event
	   
	   cookie		- Cookie passed as one of the params in the Callback for the user to identify the service instance

   Returns:
        If successful, returns a valid handle to an object. Otherwise, it
        returns SYS_MODULE_OBJ_INVALID.

   Example:
        <code>

		SYS_MQTT_Config    	g_sCloudSrvcCfg;
		SYS_MODULE_OBJ 			g_cloudSrvcHandle;

		memset(&g_sCloudSrvcCfg, 0, sizeof(g_sCloudSrvcCfg));
		g_sCloudSrvcCfg.configBitmask |= SYS_MQTT_CONFIG_MASK_MQTT;
		strcpy(g_sCloudSrvcCfg.mqttConfig.brokerConfig.brokerName, "test.mosquitto.org", strlen("test.mosquitto.org"));
		g_sCloudSrvcCfg.mqttConfig.brokerConfig.serverPort = 1883;
		strcpy(g_sCloudSrvcCfg.mqttConfig.brokerConfig.clientId, "pic32mzw1", strlen("pic32maw1"));
		g_sCloudSrvcCfg.mqttConfig.brokerConfig.autoConnect = 1;
		g_sCloudSrvcCfg.mqttConfig.brokerConfig.tlsEnabled = 0;
		
		g_sCloudSrvcCfg.mqttConfig.subscribeCount = 1;
		strcpy(g_sCloudSrvcCfg.mqttConfig.subscribeConfig[0].topicName, "house/temperature/first_floor/kitchen", strlen("house/temperature/first_floor/kitchen"));
		g_sCloudSrvcCfg.mqttConfig.subscribeConfig[0].qos = 1;
		
		g_cloudSrvcHandle = SYS_MQTT_Connect(&g_sCloudSrvcCfg, cloudSrvcCallback, 0);                
        if (g_cloudSrvcHandle == SYS_MODULE_OBJ_INVALID)
        {
            // Handle error
        }
        </code>

  Remarks:
        This routine should be called only once when the user is configuring the Cloud service
*/

SYS_MODULE_OBJ SYS_MQTT_Connect(SYS_MQTT_Config *cfg, SYS_MQTT_CALLBACK fn, void *cookie);

// *****************************************************************************
/* System Cloud Control Message Type

  Summary:
    Control Message Type used with SYS_MQTT_CtrlMsg() for performing various 
	MQTT/ AWS/ Azure Operations.

  Remarks:
    None.
*/
typedef enum {
	SYS_MQTT_CTRL_MSG_TYPE_MQTT_SUBSCRIBE,          //MQTT - Subscribe to a Topic
	SYS_MQTT_CTRL_MSG_TYPE_MQTT_UNSUBSCRIBE,		//MQTT - Unsubscribe from a Topic
	SYS_MQTT_CTRL_MSG_TYPE_MQTT_CONNECT,			//MQTT - Connect to a Broker
	SYS_MQTT_CTRL_MSG_TYPE_MQTT_DISCONNECT,         //MQTT - Disconnect from the Broker
} SYS_MQTT_CtrlMsgType;

// *****************************************************************************
/* Function:
       int32_t	SYS_MQTT_CtrlMsg(SYS_MODULE_OBJ *hdl, 
								SYS_MQTT_CtrlMsgType eCtrlMsgType, 
								void *data, 
								uint16_t len)

  Summary:
      Returns success/ failure for the operation asked by the user:
		1. Connect
		2. Disconnect
		3. Subscribe
		4. Unsubscribe

  Description:
       This function is used for disconnecting/ connecting/ to the peer or Publish/ Subscribe to a Topic.

  Precondition:
       SYS_MQTT_Connect should have been called.

  Parameters:
       object  		- SYS Cloud object handle, returned from SYS_MQTT_Connect
	   
	   eCtrlMsgType - valid Msg Type - SYS_MQTT_CtrlMsgType
      
       data			- valid data buffer pointer based on the Msg Type
	   
	   len			- length of the data buffer the pointer is pointing to

  Returns:
		SYS_MQTT_SUCCESS - Indicates that the Request was catered to successfully
		SYS_MQTT_FAILURE - Indicates that the Request failed

  Example:
       <code>
	   SYS_MQTT_SubscribeConfig		sSubscribeCfg;
	   
	   memset(&sSubscribeCfg, 0, sizeof(sSubscribeCfg));
	   sSubscribeCfg.qos = 1;
	   strcpy(sSubscribeCfg.topicName, "house/temperature/first_floor/kitchen"));

       // Handle "objSysCloud" value must have been returned from SYS_MQTT_Connect.	   
		if( SYS_MQTT_CtrlMsg(objSysCloud, SYS_MQTT_CTRL_MSG_TYPE_MQTT_SUBSCRIBE, &sSubscribeCfg, sizeof(sSubscribeCfg)) == SYS_MQTT_SUCCESS)
		{
		}
		</code>

  Remarks:
       None.
  */
int32_t	SYS_MQTT_CtrlMsg(SYS_MODULE_OBJ hdl, SYS_MQTT_CtrlMsgType eCtrlMsgType, void *data, uint16_t len);


#define SYS_MQTT_EVENT_BASE	0
#define SYS_MQTT_AWS_EVENT_BASE	10
#define SYS_MQTT_AZURE_EVENT_BASE	20


// *****************************************************************************
/* System MQTT Event Structure

  Summary:
    MQTT Event Data which comes with the Callback SYS_MQTT_CtrlMsg() for performing various 
	MQTT.

  Remarks:
    None.
*/
typedef struct {
	char	topicName[SYS_MQTT_TOPIC_NAME_MAX_LEN];	//Topic on which the message was received
	uint8_t	message[SYS_MQTT_MSG_MAX_LEN];			//the message was received
	uint16_t	messageLen;									//Length of the message
} SYS_MQTT_EventData;

void SYS_MQTT_Task(SYS_MODULE_OBJ obj);
SYS_MODULE_OBJ SYS_MQTT_GetHandleFromPaho(Network* n);

// *****************************************************************************
/* Function:
       int32_t	SYS_MQTT_Publish(SYS_MODULE_OBJ *hdl, 
								SYS_MQTT_PublishTopicCfg  *psPubCfg, 
								char *message, 
								uint16_t message_len)

  Summary:
      Returns success/ failure for the publishing of message asked by the user

  Description:
       This function is used for Publishing to a Topic.

  Precondition:
       SYS_MQTT_Connect should have been called.

  Parameters:
       hdl  		- SYS_MQTT object handle, returned from SYS_MQTT_Connect
	   
       psPubCfg		- valid pointer to the Topic details on which to Publish
       
	   message		- Message to be published

	   message_len  - Message length
  
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

  Remarks:
       None.
  */
int32_t	SYS_MQTT_Publish(SYS_MODULE_OBJ obj, SYS_MQTT_PublishTopicCfg  *psPubCfg, char *message, uint16_t message_len);

// *****************************************************************************
/* Function:
       int32_t	SYS_MQTT_Subscribe(SYS_MODULE_OBJ hdl, 
       								SYS_MQTT_SubscribeConfig  *subConfig)

  Summary:
      Returns success/ failure for the subscribing to a Topic by the user

  Description:
       This function is used for Subscribing to a Topic.

  Precondition:
       SYS_MQTT_Connect should have been called.

  Parameters:
       hdl  		- SYS_MQTT object handle, returned from SYS_MQTT_Connect
	   
       subConfig	- valid pointer to the Topic details on which to Subscribe
       
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

  Remarks:
       None.
  */
int32_t	SYS_MQTT_Subscribe(SYS_MODULE_OBJ hdl, SYS_MQTT_SubscribeConfig  *subConfig);

// *****************************************************************************
/* Function:
       int32_t	SYS_MQTT_Unsubscribe(SYS_MODULE_OBJ hdl, 
       								SYS_MQTT_SubscribeConfig  *subConfig)

  Summary:
      Returns success/ failure for the unsubscribing to a Topic by the user

  Description:
       This function is used for Unsubscribing to a Topic.

  Precondition:
       SYS_MQTT_Connect should have been called.

  Parameters:
       hdl  		- SYS_MQTT object handle, returned from SYS_MQTT_Connect
	   
       topic		- Topic from which to unsubscribe
       
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

  Remarks:
       None.
  */
int32_t	SYS_MQTT_Unsubscribe(SYS_MODULE_OBJ hdl, char *subTopic);

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
       hdl  		- SYS_MQTT object handle, returned from SYS_MQTT_Connect
	   
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
SYS_MQTT_STATUS SYS_MQTT_GetStatus(SYS_MODULE_OBJ obj);
int32_t SYS_MQTT_Initialize();
void* SYS_MQTT_AllocHandle();
void SYS_MQTT_FreeHandle(void *handle);
#endif //SYS_MQTT_H
