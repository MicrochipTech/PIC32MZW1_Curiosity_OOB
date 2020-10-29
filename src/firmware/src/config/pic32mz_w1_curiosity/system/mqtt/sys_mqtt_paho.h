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

#ifndef SYS_MQTT_PAHO_H    // Guards against multiple inclusion
#define SYS_MQTT_PAHO_H
#include "configuration.h"
#ifdef SYS_MQTT_PAHO
#include <stdlib.h>
#include "definitions.h"
#include "sys_mqtt.h"
#include "third_party/paho.mqtt.embedded-c/MQTTClient-C/Platforms/MCHP_pic32mzw1.h" 
#include "third_party/paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.h" 

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

typedef struct {
    uint32_t    startTime;
    uint32_t    timeOut;
}SYS_MQTT_TimerInfo;

typedef union {
    SYS_MQTT_PahoInfo  sPahoInfo;
} SYS_MQTT_VendorInfo;

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
/* System MQTT Control Message Type

  Summary:
    Control Message Type used with SYS_MQTT_CtrlMsg() for performing various 
	MQTT Operations.

  Remarks:
    None.
*/
typedef enum {
	//MQTT - Subscribe to a Topic
	SYS_MQTT_CTRL_MSG_TYPE_MQTT_SUBSCRIBE,          
	
	//MQTT - Unsubscribe from a Topic
	SYS_MQTT_CTRL_MSG_TYPE_MQTT_UNSUBSCRIBE,		
	
	//MQTT - Connect to a Broker
	SYS_MQTT_CTRL_MSG_TYPE_MQTT_CONNECT,			
	
	//MQTT - Disconnect from the Broker
	SYS_MQTT_CTRL_MSG_TYPE_MQTT_DISCONNECT,         
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
void* SYS_MQTT_AllocHandle();
void SYS_MQTT_FreeHandle(void *handle);
SYS_MODULE_OBJ SYS_MQTT_PAHO_Open(SYS_MQTT_Config *cfg, 
													SYS_MQTT_CALLBACK fn, 
													void *cookie);
void SYS_MQTT_Paho_Task(SYS_MODULE_OBJ obj);
int32_t	SYS_MQTT_Paho_CtrlMsg(SYS_MODULE_OBJ obj, SYS_MQTT_CtrlMsgType eCtrlMsgType, void *data, uint16_t len);
int32_t	SYS_MQTT_Paho_SendMsg(SYS_MODULE_OBJ obj, SYS_MQTT_PublishTopicCfg  *psTopicCfg, char *message, uint16_t message_len);
SYS_MODULE_OBJ SYS_MQTT_Paho_GetNetHdlFromNw(Network* n);
void SYS_MQTT_Paho_Close(SYS_MODULE_OBJ obj);
SYS_MODULE_OBJ SYS_MQTT_GetHandleFromPaho(Network* n);

#endif
#endif //SYS_MQTT_PAHO_H
