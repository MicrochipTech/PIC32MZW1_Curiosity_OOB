/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    mqtt_app.c

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

#include "mqtt_app.h"
#include "system/command/sys_command.h"
#include "app_control.h"
#include "cJSON.h"
#include "system/mqtt/sys_mqtt.h"
#include "bsp/bsp.h"

MQTT_APP_DATA mqtt_appData;

int32_t MqttCallback(SYS_MQTT_EVENT_TYPE eEventType, void *data, uint16_t len, void* cookie) {
    static int errorCount = 0;
    switch (eEventType) {
        case SYS_MQTT_EVENT_MSG_RCVD:
        {
            SYS_MQTT_PublishConfig *psMsg = (SYS_MQTT_PublishConfig *) data;
            psMsg->message[psMsg->messageLength] = 0;
            psMsg->topicName[psMsg->topicLength] = 0;
            //SYS_CONSOLE_PRINT("\nMqttCallback(): Msg received on Topic: %s ; Msg: %s\r\n",psMsg->topicName, psMsg->message);

            if (NULL != strstr((char*) psMsg->topicName, "/shadow/update/delta")) {
                cJSON *messageJson = cJSON_Parse((char*) psMsg->message);
                if (messageJson == NULL) {
                    const char *error_ptr = cJSON_GetErrorPtr();
                    if (error_ptr != NULL) {
                        SYS_CONSOLE_PRINT(TERM_RED"Message JSON parse Error. Error before: %s\n"TERM_RESET, error_ptr);
                    }
                    cJSON_Delete(messageJson);
                    break;
                }

                //Get the desired state
                cJSON *state = cJSON_GetObjectItem(messageJson, "state");
                if (!state) {
                    cJSON_Delete(messageJson);
                    break;
                }

                //Get the toggle state
                cJSON *toggle = cJSON_GetObjectItem(state, "toggle");
                if (!toggle) {
                    cJSON_Delete(messageJson);
                    break;
                }

                bool desiredState = (bool) toggle->valueint;
                if (desiredState) {
                    LED_GREEN_On();
                    SYS_CONSOLE_PRINT(TERM_GREEN"LED ON\r\n"TERM_RESET);
                } else {
                    LED_GREEN_Off();
                    SYS_CONSOLE_PRINT(TERM_YELLOW"LED OFF\r\n"TERM_RESET);
                }
                cJSON_Delete(messageJson);
#if 0
                if (NULL != strstr((char*) psMsg->message, "\"state\":{\"toggle\":1}")) {
                    LED_GREEN_On();
                    SYS_CONSOLE_PRINT(TERM_GREEN"LED ON"TERM_RESET);
                } else if (NULL != strstr((char*) psMsg->message, "\"state\":{\"toggle\":0}")) {
                    LED_GREEN_Off();
                    SYS_CONSOLE_PRINT(TERM_GREEN"LED OFF"TERM_RESET);
                }
#endif
                mqtt_appData.shadowUpdate = true;
                mqtt_appData.pubFlag = true;
            }
        }
            break;

        case SYS_MQTT_EVENT_MSG_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): MQTT Disconnected\r\n");
            mqtt_appData.MQTTConnected = false;
            app_controlData.mqttCtrl.conStat = false;
            mqtt_appData.MQTTPubQueued = false;
        }
            break;

        case SYS_MQTT_EVENT_MSG_CONNECTED:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): MQTT Connected\r\n");
            mqtt_appData.MQTTConnected = true;
            app_controlData.mqttCtrl.conStat = true;
        }
            break;

        case SYS_MQTT_EVENT_MSG_SUBSCRIBED:
        {
            SYS_MQTT_SubscribeConfig *psMqttSubCfg = (SYS_MQTT_SubscribeConfig *) data;
            SYS_CONSOLE_PRINT("\nMqttCallback(): Subscribed to Topic '%s'\r\n", psMqttSubCfg->topicName);
        }
            break;

        case SYS_MQTT_EVENT_MSG_UNSUBSCRIBED:
        {
            SYS_MQTT_SubscribeConfig *psMqttSubCfg = (SYS_MQTT_SubscribeConfig *) data;
            SYS_CONSOLE_PRINT("\nMqttCallback(): UnSubscribed to Topic '%s'\r\n", psMqttSubCfg->topicName);
        }
            break;

        case SYS_MQTT_EVENT_MSG_PUBLISHED:
        {
            //SYS_CONSOLE_PRINT("\nMqttCallback(): Published Sensor Data\r\n");
            mqtt_appData.MQTTPubQueued = false;
            errorCount = 0;
        }
            break;
        case SYS_MQTT_EVENT_MSG_CONNACK_TO:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): CONNACK Timed out\r\n");

        }
            break;
        case SYS_MQTT_EVENT_MSG_SUBACK_TO:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): SUBACK Timed out\r\n");

        }
            break;
        case SYS_MQTT_EVENT_MSG_PUBACK_TO:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): PUBACK Timed out. non-Fatal error.\r\n");
            mqtt_appData.MQTTPubQueued = false;
            errorCount++;
            if (errorCount > 5) {
                SYS_CONSOLE_PRINT(TERM_RED"\nMqttCallback(): Too many failed events. Forcing a reset\r\n"TERM_RESET);
                while (1); //force a WDT reset
            }            
        }
            break;
        case SYS_MQTT_EVENT_MSG_UNSUBACK_TO:
        {
            SYS_CONSOLE_PRINT("\nMqttCallback(): UNSUBACK Timed out\r\n");

        }
            break;
    }
    return SYS_MQTT_SUCCESS;
}

static void timerCallback(uintptr_t context) {
    //SYS_CONSOLE_PRINT("Timer : Publishing data\r\n");
    mqtt_appData.pubFlag = true;
}

static void publishMessage() {
    /*MQTT service does not queue messages*/
    
    if (mqtt_appData.MQTTConnected && !mqtt_appData.MQTTPubQueued) {
        SYS_MQTT_PublishTopicCfg sMqttTopicCfg;
        int32_t retVal = SYS_MQTT_FAILURE;
        /* All Params other than the message are initialized by the config provided in MHC*/
        char pubTopic[MQTT_APP_TOPIC_NAME_MAX_LEN] = {'\0'};
        char message[MQTT_APP_MAX_MSG_LLENGTH] = {'\0'};

        if (!mqtt_appData.shadowUpdate) { /*if a shadow update is requested, do it in this round*/
            snprintf(pubTopic, MQTT_APP_TOPIC_NAME_MAX_LEN, "%s/sensors", app_controlData.mqttCtrl.clientId);
            sprintf(message, MQTT_APP_TELEMETRY_MSG_TEMPLATE, (int) app_controlData.adcData.temp);
            /*Graduation step to include an additional sensor data. Comment out the above line and uncomment the one below.*/
            //sprintf(message, MQTT_APP_TELEMETRY_MSG_GRAD_TEMPLATE, (int) app_controlData.adcData.temp,app_controlData.switchData.switchStatus);
        } else {
            snprintf(pubTopic, MQTT_APP_TOPIC_NAME_MAX_LEN, MQTT_APP_SHADOW_UPDATE_TOPIC_TEMPLATE, app_controlData.mqttCtrl.clientId);
            sprintf(message, MQTT_APP_SHADOW_MSG_TEMPLATE, LED_GREEN_Get());
            mqtt_appData.shadowUpdate = false; /*TODO: Parse puback topic and make this false in the CB*/
        }
        strcpy(sMqttTopicCfg.topicName, pubTopic);
        sMqttTopicCfg.topicLength = strlen(pubTopic);
        sMqttTopicCfg.retain = 0;
        sMqttTopicCfg.qos = 1;

        //SYS_CONSOLE_PRINT("Publishing:\r\n    Topic: %s\r\n    Message: %s\r\n",pubTopic,message);

        mqtt_appData.MQTTPubQueued = true;
        retVal = SYS_MQTT_Publish(mqtt_appData.SysMqttHandle,
                &sMqttTopicCfg,
                message,
                strlen(message) + 1);
        if (retVal != SYS_MQTT_SUCCESS) {
            SYS_CONSOLE_PRINT("\nMQTT_APP: publishMessage() Failed (%d)\r\n", retVal);
        }
    } else {
        return;
    }
}

static void MQTT_APP_SysMQTT_init() {
    SYS_MQTT_Config cloudConfig;
    cloudConfig = g_sSysMqttConfig; /*take a copy of the global config and modify just what is required*/
    strncpy(cloudConfig.sBrokerConfig.brokerName, app_controlData.mqttCtrl.mqttBroker, APP_CTRL_MAX_BROKER_NAME_LEN);
    strncpy(cloudConfig.sBrokerConfig.clientId, app_controlData.mqttCtrl.clientId, APP_CTRL_MAX_CLIENT_ID_LEN);

    /*subscribe to shadow delta topic*/
    char subTopic[MQTT_APP_TOPIC_NAME_MAX_LEN];
    snprintf(subTopic, MQTT_APP_TOPIC_NAME_MAX_LEN, MQTT_APP_SHADOW_DELTA_TOPIC_TEMPLATE, app_controlData.mqttCtrl.clientId);

    cloudConfig.subscribeCount = 1;
    memcpy(cloudConfig.sSubscribeConfig[0].topicName, subTopic, strlen(subTopic)+1);
    cloudConfig.sSubscribeConfig[0].qos = 1;
    mqtt_appData.SysMqttHandle = SYS_MQTT_Connect(&cloudConfig, MqttCallback, NULL);
}

void MQTT_APP_Initialize(void) {
    mqtt_appData.state = MQTT_APP_STATE_INIT;
    mqtt_appData.SysMqttHandle = SYS_MODULE_OBJ_INVALID;
    mqtt_appData.pubFlag = true;
    mqtt_appData.MQTTPubQueued = false;
    mqtt_appData.MQTTConnected = false;
    mqtt_appData.shadowUpdate = true; /*so that we send the boot status update*/
    mqtt_appData.state = MQTT_APP_STATE_INIT;
}

void MQTT_APP_Tasks(void) {
    switch (mqtt_appData.state) {
        case MQTT_APP_STATE_INIT:
        {
            if (app_controlData.serialNumValid && app_controlData.mqttCtrl.mqttConfigValid) {
                SYS_CONSOLE_PRINT("Found valid MQTT config\r\n");
                SYS_CONSOLE_PRINT("Device SerialNumber is : "TERM_GREEN"%s\r\n"TERM_RESET, app_controlData.devSerialStr);
                MQTT_APP_SysMQTT_init();
                SYS_TIME_HANDLE handle = SYS_TIME_CallbackRegisterMS(timerCallback, (uintptr_t) 0, 1000, SYS_TIME_PERIODIC);
                if (handle == SYS_TIME_HANDLE_INVALID) {
                    SYS_CONSOLE_PRINT(TERM_RED"MQTT_APP: Failed creating a timer for publish \r\n"TERM_RESET);
                }
                mqtt_appData.state = MQTT_APP_STATE_SERVICE_TASKS;
            }
            break;
        }
        case MQTT_APP_STATE_SERVICE_TASKS:
        {
            //APP_MQTT_Task();
            if (mqtt_appData.pubFlag) {/*This flag will be set in timerCallback()*/
                mqtt_appData.pubFlag = false;
                publishMessage();
            }
            SYS_MQTT_Task(mqtt_appData.SysMqttHandle);
            break;
        }
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
