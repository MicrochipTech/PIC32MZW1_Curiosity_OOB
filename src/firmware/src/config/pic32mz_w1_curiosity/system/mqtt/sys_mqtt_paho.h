#ifndef SYS_MQTT_PAHO_H    // Guards against multiple inclusion
#define SYS_MQTT_PAHO_H
#include "configuration.h"
#ifdef SYS_MQTT_PAHO
#include <stdlib.h>
#include "definitions.h"
#include "sys_mqtt.h"

SYS_MODULE_OBJ SYS_MQTT_PAHO_Open(SYS_MQTT_Config *cfg, 
													SYS_MQTT_CALLBACK fn, 
													void *cookie);
void SYS_MQTT_Paho_Task(SYS_MODULE_OBJ obj);
int32_t	SYS_MQTT_Paho_CtrlMsg(SYS_MODULE_OBJ obj, SYS_MQTT_CtrlMsgType eCtrlMsgType, void *data, uint16_t len);
int32_t	SYS_MQTT_Paho_SendMsg(SYS_MODULE_OBJ obj, SYS_MQTT_PublishTopicCfg  *psTopicCfg, char *message, uint16_t message_len);
SYS_MODULE_OBJ SYS_MQTT_Paho_GetNetHdlFromNw(Network* n);
void SYS_MQTT_Paho_Close(SYS_MODULE_OBJ obj);
#endif
#endif //SYS_MQTT_PAHO_H
