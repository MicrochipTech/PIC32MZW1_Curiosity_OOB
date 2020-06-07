#include "configuration.h"
#include "system/appdebug/sys_appdebug.h"

#ifdef SYS_MQTT_PAHO
#include "../sys_mqtt_paho.h"
#include "third_party/paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.h" 

#define MQTTPacket_willOptions_initializer { {'M', 'Q', 'T', 'W'}, 0, {NULL, {0, NULL}}, {NULL, {0, NULL}}, 0, 0 }

#define MQTTPacket_connectData_initializer { {'M', 'Q', 'T', 'C'}, 0, 4, {NULL, {0, NULL}}, 60, 1, 0, \
		MQTTPacket_willOptions_initializer, {NULL, {0, NULL}}, {NULL, {0, NULL}} }

#define SYS_MQTT_MAX_NUM_OF_INSTANCES  1
extern SYS_MQTT_Handle         g_asSysMqttHandle[SYS_MQTT_MAX_NUM_OF_INSTANCES];
MQTTMessage g_sMqttMsg;

extern uint8_t g_OmitPacketType;
#define SYS_MQTT_DBG_OMIT_PKT_TYPE_KEEPALIVE    1
#define SYS_MQTT_DBG_OMIT_PKT_TYPE_CONNACK      2
#define SYS_MQTT_DBG_OMIT_PKT_TYPE_PUBACK       3
#define SYS_MQTT_DBG_OMIT_PKT_TYPE_SUBACK       4
#define SYS_MQTT_DBG_OMIT_PKT_TYPE_UNSUBACK     5

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void TcpClientCallback(uint32_t event, void *data, void* cookie)
{
    SYS_MQTT_Handle  	*hdl = ( SYS_MQTT_Handle  	*)cookie;
     
    switch(event)
    {
        case SYS_NET_EVNT_CONNECTED:
        {
            /* TCP Socket got connected */
            SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_CFG, "Status UP\r\n");
            if((hdl->eStatus == SYS_MQTT_STATUS_MQTT_DISCONNECTED) && 
                    (hdl->sCfgInfo.sBrokerConfig.autoConnect == false))
                break;
            hdl->eStatus = SYS_MQTT_STATUS_SOCK_CONNECTED;
        }
        break;

        case SYS_NET_EVNT_DISCONNECTED:
        {
            /* TCP Socket got disconnected */
            SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_CFG, "Status DOWN\r\n");
            hdl->eStatus = SYS_MQTT_STATUS_MQTT_DISCONNECTING;
        }
        break;
    }
}

#define SYS_MQTT_PERIOIDC_TIMEOUT   5 // 5 Sec
#define SYS_MQTT_TIMEOUT_CONST (SYS_MQTT_PERIOIDC_TIMEOUT * SYS_TMR_TickCounterFrequencyGet())

void SYS_MQTT_StartTimer(SYS_MQTT_Handle *hdl, uint32_t timerInfo)
{
    hdl->timerInfo.startTime = SYS_TMR_TickCountGet();
    hdl->timerInfo.timeOut = timerInfo;
}

bool SYS_MQTT_TimerExpired(SYS_MQTT_Handle *hdl)
{
    if(hdl->timerInfo.startTime == 0)
        return false;
    
    return (SYS_TMR_TickCountGet() - hdl->timerInfo.startTime > hdl->timerInfo.timeOut);
}

void SYS_MQTT_ResetTimer(SYS_MQTT_Handle *hdl)
{
    hdl->timerInfo.startTime = 0;
}

void SYS_MQTT_ProcessTimeout(SYS_MQTT_Handle *hdl, SYS_MQTT_EVENT_TYPE cbEvent, SYS_MQTT_STATUS nextStatus)
{
    if(SYS_MQTT_TimerExpired(hdl) == true)
    {
        if(hdl->callback_fn)
            hdl->callback_fn(cbEvent,
                        NULL,
                        0,
                        hdl->vCookie);                    
        SYS_MQTT_ResetTimer(hdl);
        hdl->eStatus = nextStatus;
    }
}

void SYS_MQTT_messageCallback(MessageData* data)
{
	SYS_MQTT_PublishConfig	sMsg;

    SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_CFG, "Topic = %s\r\n", (char *)data->topicName->lenstring.data);
	memset(&sMsg, 0, sizeof(sMsg));
	memcpy((char *)sMsg.message, (char *)data->message->payload, (uint32_t)data->message->payloadlen);
	sMsg.topicName = (char *)data->topicName->lenstring.data;
	sMsg.messageLength = (uint32_t)data->message->payloadlen;
    sMsg.topicLength = data->topicName->lenstring.len;
    
    /* Sending the Published message to the Application */
	if(g_asSysMqttHandle[0].callback_fn)
		g_asSysMqttHandle[0].callback_fn(SYS_MQTT_EVENT_MSG_RCVD,
						&sMsg,
						sizeof(sMsg),
						g_asSysMqttHandle[0].vCookie);
}

SYS_MODULE_OBJ SYS_MQTT_PAHO_Open(SYS_MQTT_Config *cfg, 
													SYS_MQTT_CALLBACK fn, 
													void *cookie)
{
    SYS_MQTT_Handle  	*hdl = NULL;    	
    SYS_APPDEBUG_FN_ENTER_PRINT(g_AppDebugHdl, MQTT_CFG);
    
	/* 
	** Allocate Handle - hdl 
	*/
	hdl = SYS_MQTT_AllocHandle();
    if(hdl == NULL)
    {
        SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_CFG, "Failed to allocate Handle\r\n");
        return SYS_MODULE_OBJ_INVALID;        
    }
    
	/* 
	** Create Semaphore for ensuring the SYS NET APIs are re-entrant 
	*/
    if(OSAL_SEM_Create(&hdl->InstSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1) != OSAL_RESULT_TRUE)
    {
        /* 
		** Free Handle 
		*/		
        SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_CFG, "Failed to create Semaphore\r\n");
        SYS_MQTT_FreeHandle(hdl);
        return SYS_MODULE_OBJ_INVALID;        
    }
	
	/* 
	** Copy the config info and the fn ptr into the Handle 
	*/
	if(cfg == NULL)
	{
		memcpy(&hdl->sCfgInfo, &g_sSysMqttConfig, sizeof(SYS_MQTT_Config));
	}
	else
	{
		memcpy(&hdl->sCfgInfo, cfg, sizeof(SYS_MQTT_Config));
	}
    hdl->vCookie = cookie;
    hdl->callback_fn = fn;
    hdl->netSrvcHdl = SYS_MODULE_OBJ_INVALID;
    
    hdl->eStatus = SYS_MQTT_STATUS_LOWER_LAYER_DOWN;

    SYS_APPDEBUG_FN_EXIT_PRINT(g_AppDebugHdl, MQTT_CFG);
    return (SYS_MODULE_OBJ)hdl; 				
}

void SYS_MQTT_Paho_Task(SYS_MODULE_OBJ obj)
{
    static uint32_t connCbSent = 0;
	SYS_MQTT_Handle	*hdl = (SYS_MQTT_Handle *)obj;

    SYS_APPDEBUG_FN_ENTER_PRINT(g_AppDebugHdl, MQTT_CFG);
    
    if(obj == SYS_MODULE_OBJ_INVALID)
    {
        SYS_APPDEBUG_INFO_PRINT(g_AppDebugHdl, MQTT_CFG, "Handle Invalid\r\n");
        return;
    }
    
    SYS_NET_Task(hdl->netSrvcHdl);

	switch(hdl->eStatus)
	{
		case SYS_MQTT_STATUS_LOWER_LAYER_DOWN:
		{
            SYS_NET_Config sSysNetCfg; 
            
            /* Open a TCP Socket via the NET Service */
            memset(&sSysNetCfg, 0, sizeof(sSysNetCfg));
        
            sSysNetCfg.mode = SYS_NET_MODE_CLIENT;
            sSysNetCfg.ip_prot = SYS_NET_IP_PROT_TCP;
            sSysNetCfg.enable_reconnect = hdl->sCfgInfo.sBrokerConfig.autoConnect;
            sSysNetCfg.enable_tls = hdl->sCfgInfo.sBrokerConfig.tlsEnabled;
            strcpy(sSysNetCfg.host_name, hdl->sCfgInfo.sBrokerConfig.brokerName);
            sSysNetCfg.port = hdl->sCfgInfo.sBrokerConfig.serverPort;

            SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_CFG, "Host Name = %s \r\n", 
                    hdl->sCfgInfo.sBrokerConfig.brokerName);
            
            hdl->netSrvcHdl = SYS_NET_Open(&sSysNetCfg, TcpClientCallback, hdl);
            if(hdl->netSrvcHdl != SYS_MODULE_OBJ_INVALID)
            {
                SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_CFG, "TCPIP Socket Opened\r\n");
                hdl->eStatus = SYS_MQTT_STATUS_SOCK_CLIENT_CONNECTING;
                return ; 				
            }
            else
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_CFG, "TCPIP Socket Open FAILED\r\n");
                hdl->eStatus = SYS_MQTT_STATUS_SOCK_OPEN_FAILED;
                return ; 				    
            }
		}
		break;

        case SYS_MQTT_STATUS_SOCK_CONNECTED:
        {
        	int rc=0;
            char buffer[80];
            memset(buffer, 0, sizeof(buffer));

            /* Open the MQTT Connection */
            NetworkInit(&(hdl->uVendorInfo.sPahoInfo.sPahoNetwork));
            MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
            MQTTClientInit(&(hdl->uVendorInfo.sPahoInfo.sPahoClient), 
                            &(hdl->uVendorInfo.sPahoInfo.sPahoNetwork), 
                            5000, 
                            hdl->uVendorInfo.sPahoInfo.sendbuf, 
                            SYS_MQTT_PAHO_MAX_TX_BUFF_LEN, 
                            hdl->uVendorInfo.sPahoInfo.recvbuf, 
                            SYS_MQTT_PAHO_MAX_RX_BUFF_LEN);

            connectData.MQTTVersion = 4; //use protocol version 3.1.1
            connectData.clientID.cstring = (char *) &(hdl->sCfgInfo.sBrokerConfig.clientId);
            connectData.keepAliveInterval = hdl->sCfgInfo.sBrokerConfig.keepAliveInterval;
            if(hdl->sCfgInfo.bLwtEnabled)
            {
                connectData.willFlag = 1;
                connectData.will.message.cstring = (char *)&hdl->sCfgInfo.sLwtConfig.message; 
                connectData.will.topicName.cstring = hdl->sCfgInfo.sLwtConfig.topicName;
                connectData.will.qos = hdl->sCfgInfo.sLwtConfig.qos;
                connectData.will.retained = hdl->sCfgInfo.sLwtConfig.retain;
            }
            if ((rc = MQTTConnect(&(hdl->uVendorInfo.sPahoInfo.sPahoClient), &connectData)) != 0)
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_CFG, "MQTTConnect() failed (%d)\r\n", rc);
                hdl->eStatus = SYS_MQTT_STATUS_MQTT_CONN_FAILED;
                return;
            }
            SYS_MQTT_StartTimer(hdl, SYS_MQTT_TIMEOUT_CONST);
            hdl->eStatus = SYS_MQTT_STATUS_WAIT_FOR_MQTT_CONACK;
            
        }
        break;
        
		case SYS_MQTT_STATUS_WAIT_FOR_MQTT_CONACK:
		{
            /* Wait for the MQTT Connection Ack from the MQTT Server */
            int rc = MQTTWaitForConnect(&(hdl->uVendorInfo.sPahoInfo.sPahoClient));
            if(g_OmitPacketType == SYS_MQTT_DBG_OMIT_PKT_TYPE_CONNACK)
                rc = FAILURE; //This is test stub 
            if (rc == SUCCESS)
            {
                SYS_MQTT_ResetTimer(hdl);
            	hdl->eStatus = SYS_MQTT_STATUS_MQTT_CONNECTED;
                hdl->uVendorInfo.sPahoInfo.subscribeCount = 0;
                SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_CFG, "MQTT Connected\r\n");
                
                /* Check if the Application configured a Topic to 
                 * Subscribe to while opening the MQTT Service */
                if(hdl->sCfgInfo.subscribeCount)
                {
                    SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_DATA, "Subscribing to Topic = \r\n", hdl->sCfgInfo.subscribeCount, hdl->sCfgInfo.sSubscribeConfig[0].topicName);
                    if ((rc = MQTTSubscribe(&(hdl->uVendorInfo.sPahoInfo.sPahoClient), 
                                            hdl->sCfgInfo.sSubscribeConfig[0].topicName, 
                                            (enum QoS )(hdl->sCfgInfo.sSubscribeConfig[0].qos),
                                            SYS_MQTT_messageCallback)) != 0)
                    {
                        SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "MQTTSubscribe() failed (%d)\r\n", rc);
                        return ;
                    }
                    SYS_MQTT_StartTimer(hdl, SYS_MQTT_TIMEOUT_CONST);
                    hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.qos = hdl->sCfgInfo.sSubscribeConfig[0].qos;
                    hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.topicName = hdl->sCfgInfo.sSubscribeConfig[0].topicName;
                    hdl->eStatus = SYS_MQTT_STATUS_WAIT_FOR_MQTT_SUBACK;                    
                }
                else
                {
                    connCbSent = 1;
                    if(hdl->callback_fn)
                        hdl->callback_fn(SYS_MQTT_EVENT_MSG_CONNECTED,
                                    NULL,
                                    0,
                                    hdl->vCookie);
                }
            }
            else
            {
                SYS_MQTT_ProcessTimeout(hdl, SYS_MQTT_EVENT_MSG_CONNACK_TO, SYS_MQTT_STATUS_MQTT_CONN_FAILED);
            }
		}
		break;
        
        case SYS_MQTT_STATUS_WAIT_FOR_MQTT_SUBACK:
        {
            /* Wait for the MQTT Subscribe Ack */
            int rc = MQTTWaitForSubscribeAck(&(hdl->uVendorInfo.sPahoInfo.sPahoClient), 
											hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.topicName,
											SYS_MQTT_messageCallback);
            if(g_OmitPacketType == SYS_MQTT_DBG_OMIT_PKT_TYPE_SUBACK)
                rc = FAILURE; //This is test stub 
            if (rc == SUCCESS)
            {
				SYS_MQTT_SubscribeConfig	sMqttSubCfg;
                SYS_MQTT_ResetTimer(hdl);
				memset(&sMqttSubCfg, 0, sizeof(sMqttSubCfg));
				strcpy(sMqttSubCfg.topicName, hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.topicName);

                SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_DATA, "Suback received for Topic (%s)\r\n", hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.topicName);
                
            	hdl->eStatus = SYS_MQTT_STATUS_MQTT_CONNECTED;
                if(connCbSent == 0)
                {
                    connCbSent = 1;
                    if(hdl->callback_fn)
                        hdl->callback_fn(SYS_MQTT_EVENT_MSG_CONNECTED,
                                    NULL,
                                    0,
                                    hdl->vCookie);
                }
                
                /* Let the Application know that we have received the Sub Ack  */
                if(hdl->callback_fn)
	                hdl->callback_fn(SYS_MQTT_EVENT_MSG_SUBSCRIBED,
                                &sMqttSubCfg,
                                sizeof(sMqttSubCfg),
                                hdl->vCookie);
                memset(&hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress, 0,
                        sizeof(hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress));
            }
            else
            {
                SYS_MQTT_ProcessTimeout(hdl, SYS_MQTT_EVENT_MSG_SUBACK_TO, SYS_MQTT_STATUS_MQTT_CONNECTED);
            }
        }
        break;
	
        case SYS_MQTT_STATUS_WAIT_FOR_MQTT_UNSUBACK:
        {
            /* Unsubscribe to a Topic */
            int rc = MQTTWaitForUnsubscribeAck(&(hdl->uVendorInfo.sPahoInfo.sPahoClient), 
											hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.topicName);
            if(g_OmitPacketType == SYS_MQTT_DBG_OMIT_PKT_TYPE_UNSUBACK)
                rc = FAILURE; //This is test stub 
            if (rc == SUCCESS)
            {
				SYS_MQTT_SubscribeConfig	sMqttSubCfg;
                SYS_MQTT_ResetTimer(hdl);
				memset(&sMqttSubCfg, 0, sizeof(sMqttSubCfg));
				strcpy(sMqttSubCfg.topicName, hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.topicName);
            	hdl->eStatus = SYS_MQTT_STATUS_MQTT_CONNECTED;

                SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_DATA, "Unsuback received for Topic (%s)\r\n", hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.topicName);

                /* Let the Applciation know that we have unsubscribed to the Topic */
                if(hdl->callback_fn)
	                hdl->callback_fn(SYS_MQTT_EVENT_MSG_UNSUBSCRIBED,
                                &sMqttSubCfg,
                                sizeof(sMqttSubCfg),
                                hdl->vCookie);
            }
            else
            {
                SYS_MQTT_ProcessTimeout(hdl, SYS_MQTT_EVENT_MSG_UNSUBACK_TO, SYS_MQTT_STATUS_MQTT_CONNECTED);
            }
        }
        break;
	
        case SYS_MQTT_STATUS_WAIT_FOR_MQTT_PUBACK:
		{
            /* Wait for the MQTT Publish Ack */
            int rc = MQTTWaitForPublishAck(&(hdl->uVendorInfo.sPahoInfo.sPahoClient), 
											&g_sMqttMsg );
            if(g_OmitPacketType == SYS_MQTT_DBG_OMIT_PKT_TYPE_PUBACK)
                rc = FAILURE; //This is test stub 
            if (rc == SUCCESS)
            {
                SYS_MQTT_ResetTimer(hdl);
            	hdl->eStatus = SYS_MQTT_STATUS_MQTT_CONNECTED;
                SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_DATA, "Puback received\r\n", g_sMqttMsg.payload);

                /* Tell the Application that we have received the PUBACK */
                if(hdl->callback_fn)
	                hdl->callback_fn(SYS_MQTT_EVENT_MSG_PUBLISHED,
                                NULL,
                                0,
                                hdl->vCookie);
            }
            else
            {
                SYS_MQTT_ProcessTimeout(hdl, SYS_MQTT_EVENT_MSG_PUBACK_TO, SYS_MQTT_STATUS_MQTT_CONNECTED);
            }
		}
        break;

		case SYS_MQTT_STATUS_MQTT_CONNECTED:
		{
            /* Wait for any message on the Subscribed Topics */
            int rc = MQTTWaitForPublish(&(hdl->uVendorInfo.sPahoInfo.sPahoClient));
            if (rc == SUCCESS)
            {
            }
		}
		break;

		case SYS_MQTT_STATUS_MQTT_DISCONNECTING:
		{
            int i = 0;
            for(i = 0; i < SYS_MQTT_SUB_MAX_TOPICS; i++)
            {
                /* Special Case of Subscribe Topic Config which came in open()*/
                if((i  == 0) && (hdl->sCfgInfo.subscribeCount))
                    continue;    
                
                if(hdl->sCfgInfo.sSubscribeConfig[i].entryValid == 0)
                    continue;                

                /* TBR: Should we Unsubscribe first or just Disconnect */
                hdl->sCfgInfo.sSubscribeConfig[i].entryValid = 0;
            }
            
            hdl->eStatus = SYS_MQTT_STATUS_MQTT_DISCONNECTED;
            if(hdl->callback_fn)
              hdl->callback_fn(SYS_MQTT_EVENT_MSG_DISCONNECTED,
                          NULL,
                          0,
                          hdl->vCookie);
            connCbSent = 0;
		}
		break;

		case SYS_MQTT_STATUS_IDLE:
		case SYS_MQTT_STATUS_SOCK_CLIENT_CONNECTING:
		case SYS_MQTT_STATUS_SOCK_OPEN_FAILED:
		case SYS_MQTT_STATUS_SEND_MQTT_CONN:
        case SYS_MQTT_STATUS_MQTT_DISCONNECTED:
        case SYS_MQTT_STATUS_MQTT_CONN_FAILED:
		{
		}
		break;
	}
    SYS_APPDEBUG_FN_EXIT_PRINT(g_AppDebugHdl, MQTT_CFG);
}

int32_t	SYS_MQTT_Paho_CtrlMsg(SYS_MODULE_OBJ obj, SYS_MQTT_CtrlMsgType eCtrlMsgType, void *data, uint16_t len)
{
	SYS_MQTT_Handle	*hdl = (SYS_MQTT_Handle *)obj;

    if(obj == SYS_MODULE_OBJ_INVALID)
        return SYS_MQTT_FAILURE;

    SYS_APPDEBUG_FN_ENTER_PRINT(g_AppDebugHdl, MQTT_DATA);
    
    switch(eCtrlMsgType)
    {
        case SYS_MQTT_CTRL_MSG_TYPE_MQTT_SUBSCRIBE:
        {
            int i = 0;
        	int rc=0;
            SYS_MQTT_SubscribeConfig  *psMqttSubCfg = (SYS_MQTT_SubscribeConfig  *)data;
            
            /* Find out if we are in Connected State or not */
            if(hdl->eStatus != SYS_MQTT_STATUS_MQTT_CONNECTED)
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "Failed: MQTT Not Connected\r\n");
                return SYS_MQTT_FAILURE;
            }

            /* Find out if we have already reached the MAX Topics Supported */
            for(i = 0; i < SYS_MQTT_SUB_MAX_TOPICS; i++)
            {
                if(hdl->sCfgInfo.sSubscribeConfig[i].entryValid == 0)
                    break;
            }
            
            /* Return Failure in case already reached the MAX Topics Supported */
            if(i == SYS_MQTT_SUB_MAX_TOPICS)
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "Failed: Max Topics already subscribed\r\n");
                return SYS_MQTT_FAILURE;
            }
            
            strcpy((char *)&hdl->sCfgInfo.sSubscribeConfig[i].topicName, psMqttSubCfg->topicName);
            hdl->sCfgInfo.sSubscribeConfig[i].qos = psMqttSubCfg->qos;
			hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.topicName = (char *)&hdl->sCfgInfo.sSubscribeConfig[i].topicName;

            SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_DATA, "Subscribing to %s on Index = %d\r\n", psMqttSubCfg->topicName, i);
            
            /* Subscribe to the Topic */
            if ((rc = MQTTSubscribe(&(hdl->uVendorInfo.sPahoInfo.sPahoClient), 
                                    hdl->sCfgInfo.sSubscribeConfig[i].topicName,
                                    (enum QoS )(psMqttSubCfg->qos),
                                    SYS_MQTT_messageCallback)) != 0)
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "MQTTSubscribe() Failed (%d)\r\n", rc);
                return SYS_MQTT_FAILURE;
            }
            SYS_MQTT_StartTimer(hdl, SYS_MQTT_TIMEOUT_CONST);
            hdl->sCfgInfo.sSubscribeConfig[i].entryValid = 1;
			hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.qos = psMqttSubCfg->qos;
            hdl->eStatus = SYS_MQTT_STATUS_WAIT_FOR_MQTT_SUBACK;
        }
        break;
        
        case SYS_MQTT_CTRL_MSG_TYPE_MQTT_UNSUBSCRIBE:
        {
            int rc = FAILURE;
            int i = 0;
            
            /* Find out if we are in Connected State or not */
            if(hdl->eStatus != SYS_MQTT_STATUS_MQTT_CONNECTED)
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "Failed: MQTT Not Connected\r\n");
                return SYS_MQTT_FAILURE;
            }

            /* Find out if Topic to Unsubscribe is in our List or not */
            for(i = 0; i < SYS_MQTT_SUB_MAX_TOPICS; i++)
            {
                if(hdl->sCfgInfo.sSubscribeConfig[i].entryValid == 0)
                    continue;
                
                if(strcmp(hdl->sCfgInfo.sSubscribeConfig[i].topicName, (char  *)data) == 0)
                {
                    hdl->sCfgInfo.sSubscribeConfig[i].entryValid = 0;
                    if(i == 0)
                        hdl->sCfgInfo.subscribeCount = 0;
                    break;
                }
            }

            if(i == SYS_MQTT_SUB_MAX_TOPICS)
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "Topic (%s) to Unsubscribe not in List\r\n", (char  *)data);
                return SYS_MQTT_FAILURE;
            }

            /* Unsubscribe from the Topic */
            rc = MQTTUnsubscribe(&(hdl->uVendorInfo.sPahoInfo.sPahoClient),
                                hdl->sCfgInfo.sSubscribeConfig[i].topicName);
            if(rc != 0)
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "MQTTUnsubscribe failed (%d)\r\n", rc);
                return SYS_MQTT_FAILURE;
            }
            SYS_MQTT_StartTimer(hdl, SYS_MQTT_TIMEOUT_CONST);
			hdl->uVendorInfo.sPahoInfo.sPubSubCfgInProgress.topicName = (char *)&hdl->sCfgInfo.sSubscribeConfig[i].topicName;
            hdl->eStatus = SYS_MQTT_STATUS_WAIT_FOR_MQTT_UNSUBACK;
            SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_DATA, "Unsubscribed to Topic %s)\r\n", (char *)data);
        }
        break;
        
        case SYS_MQTT_CTRL_MSG_TYPE_MQTT_CONNECT:
        {
            int rc = FAILURE;
            SYS_NET_Config sSysNetCfg; 

            if(hdl->eStatus != SYS_MQTT_STATUS_MQTT_DISCONNECTED)
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "Instance not in DISCONNECTED State (%d)\r\n", hdl->eStatus);
                return SYS_MQTT_FAILURE;
            }
            
            if(data)
            {
                SYS_MQTT_Config *cfg = (SYS_MQTT_Config *)data;
                memcpy(&hdl->sCfgInfo, cfg, sizeof(SYS_MQTT_Config));
            }
            memset(&sSysNetCfg, 0, sizeof(sSysNetCfg));
        
            sSysNetCfg.mode = SYS_NET_MODE_CLIENT;
            sSysNetCfg.ip_prot = SYS_NET_IP_PROT_TCP;
            sSysNetCfg.enable_reconnect = hdl->sCfgInfo.sBrokerConfig.autoConnect;
            sSysNetCfg.enable_tls = hdl->sCfgInfo.sBrokerConfig.tlsEnabled;
            strcpy(sSysNetCfg.host_name, hdl->sCfgInfo.sBrokerConfig.brokerName);
            sSysNetCfg.port = hdl->sCfgInfo.sBrokerConfig.serverPort;

            if ((rc = SYS_NET_CtrlMsg(hdl->netSrvcHdl,
                            SYS_NET_CTRL_MSG_RECONNECT,
                            &sSysNetCfg,
                            sizeof(sSysNetCfg))) != SYS_NET_SUCCESS)
            {
                SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "SYS_NET_CtrlMsg() Failed (%d)\r\n", rc);
                return SYS_MQTT_FAILURE;
            }
            SYS_MQTT_StartTimer(hdl, SYS_MQTT_TIMEOUT_CONST);
            hdl->eStatus = SYS_MQTT_STATUS_SOCK_CLIENT_CONNECTING;
        }
        break;
        
        case SYS_MQTT_CTRL_MSG_TYPE_MQTT_DISCONNECT:
        {            
            int rc = FAILURE;
            SYS_CONSOLE_PRINT("In SYS_MQTT_CTRL_MSG_TYPE_MQTT_DISCONNECT case Status %d\r\n", hdl->eStatus);
            if((hdl->eStatus == SYS_MQTT_STATUS_MQTT_CONNECTED) ||
                    (hdl->eStatus == SYS_MQTT_STATUS_WAIT_FOR_MQTT_CONACK) ||
                    (hdl->eStatus == SYS_MQTT_STATUS_WAIT_FOR_MQTT_SUBACK) ||
                    (hdl->eStatus == SYS_MQTT_STATUS_WAIT_FOR_MQTT_PUBACK))
            {
                if ((rc = MQTTDisconnect(&(hdl->uVendorInfo.sPahoInfo.sPahoClient))) != 0)
                {
                    SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "MQTTDisconnect() Failed (%d)\r\n", rc);
                    return SYS_MQTT_FAILURE;
                }
                hdl->eStatus = SYS_MQTT_STATUS_MQTT_DISCONNECTING;
            }
            if((hdl->eStatus != SYS_MQTT_STATUS_IDLE) &&
                    (hdl->eStatus != SYS_MQTT_STATUS_LOWER_LAYER_DOWN) &&
                    (hdl->eStatus != SYS_MQTT_STATUS_SOCK_OPEN_FAILED))
            {
                if ((rc = SYS_NET_CtrlMsg(hdl->netSrvcHdl,
                                SYS_NET_CTRL_MSG_DISCONNECT,
                                NULL,0)) != SYS_NET_SUCCESS)
                {
                    SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "SYS_NET_CtrlMsg() Failed (%d)\r\n", rc);
                    return SYS_MQTT_FAILURE;
                }
                hdl->eStatus = SYS_MQTT_STATUS_MQTT_DISCONNECTING;
            }
        }
        break;        
    }
    SYS_APPDEBUG_FN_EXIT_PRINT(g_AppDebugHdl, MQTT_DATA);
    return SYS_MQTT_SUCCESS;
}

int32_t	SYS_MQTT_Paho_SendMsg(SYS_MODULE_OBJ obj, SYS_MQTT_PublishTopicCfg  *psTopicCfg, char *message, uint16_t message_len)
{
	SYS_MQTT_Handle	*hdl = (SYS_MQTT_Handle *)obj;    
    int rc=0;

    SYS_APPDEBUG_FN_ENTER_PRINT(g_AppDebugHdl, MQTT_DATA);

    if(hdl->eStatus != SYS_MQTT_STATUS_MQTT_CONNECTED)
    {
        SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "Instance not in CONNECTED State (%d)\r\n", hdl->eStatus);
        return SYS_MQTT_FAILURE;
    }
    
    memset(&g_sMqttMsg, 0, sizeof(g_sMqttMsg));
    g_sMqttMsg.dup = 0;
    g_sMqttMsg.id = 1;
    g_sMqttMsg.payload = message;
    g_sMqttMsg.payloadlen = message_len;
    g_sMqttMsg.qos = psTopicCfg->qos;
    g_sMqttMsg.retained = psTopicCfg->retain;
    rc = MQTTPublish(&(hdl->uVendorInfo.sPahoInfo.sPahoClient), 
        psTopicCfg->topicName, 
        &g_sMqttMsg);
    if (rc != 0)
    {
        SYS_APPDEBUG_ERR_PRINT(g_AppDebugHdl, MQTT_DATA, "MQTTPublish() Failed (%d)\r\n", rc);
        return SYS_MQTT_FAILURE;
    }
    SYS_MQTT_StartTimer(hdl, SYS_MQTT_TIMEOUT_CONST);
    if(psTopicCfg->qos != 0)
    {
        hdl->eStatus = SYS_MQTT_STATUS_WAIT_FOR_MQTT_PUBACK;
    }
    SYS_APPDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_DATA, "Publish to Topic (%s)\r\n", psTopicCfg->topicName);
    return SYS_MQTT_SUCCESS;
}

SYS_MODULE_OBJ SYS_MQTT_Paho_GetNetHdlFromNw(Network* n)
{
    int32_t i = 0;
    SYS_APPDEBUG_FN_ENTER_PRINT(g_AppDebugHdl, MQTT_DATA);

    for(i = 0; i < SYS_MQTT_MAX_NUM_OF_INSTANCES; i++)
    {
        if(n == &g_asSysMqttHandle[i].uVendorInfo.sPahoInfo.sPahoNetwork)
        {
            SYS_APPDEBUG_FN_EXIT_PRINT(g_AppDebugHdl, MQTT_CFG);
            return g_asSysMqttHandle[i].netSrvcHdl;
        }
    }
    SYS_APPDEBUG_FN_EXIT_PRINT(g_AppDebugHdl, MQTT_CFG);
    return SYS_MODULE_OBJ_INVALID;
}

void SYS_MQTT_Paho_Close(SYS_MODULE_OBJ obj)
{
    SYS_MQTT_Handle	*hdl = (SYS_MQTT_Handle *)obj;

    SYS_APPDEBUG_FN_ENTER_PRINT(g_AppDebugHdl, MQTT_CFG);
    SYS_MQTT_Paho_CtrlMsg(obj, SYS_MQTT_CTRL_MSG_TYPE_MQTT_DISCONNECT, NULL, 0);
    
    SYS_APPDEBUG_Close(g_AppDebugHdl);
    
    SYS_NET_Close(hdl->netSrvcHdl);
    
    /* Delete Semaphore */
    OSAL_SEM_Delete(&hdl->InstSemaphore);
    
	/* Free the handle */
    SYS_MQTT_FreeHandle(hdl);
}

#endif //SYS_MQTT_PAHO
