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

#include "configuration.h"
#include "system/mqtt/sys_mqtt.h"
#include "system/mqtt/sys_mqtt_paho.h"

#define SYS_MQTT_MAX_NUM_OF_INSTANCES  1
SYS_MQTT_Handle g_asSysMqttHandle[SYS_MQTT_MAX_NUM_OF_INSTANCES];
uint32_t g_u32SysMqttInitDone = 0;
SYS_MODULE_OBJ g_AppDebugHdl = SYS_MODULE_OBJ_INVALID;
static OSAL_SEM_HANDLE_TYPE g_SysMqttSemaphore; /* Semaphore for Critical Section */

uint8_t g_OmitPacketType = 0;

#ifdef SYS_MQTT_ENABLE_DEBUG_PRINT
SYS_APPDEBUG_CONFIG g_sMqttAppDbgCfg;
#endif    

#define SYS_MQTT_GET_STATUS_STR(status)  \
    (status == SYS_MQTT_STATUS_IDLE)?"IDLE" : \
    (status == SYS_MQTT_STATUS_LOWER_LAYER_DOWN)?"LOWER_LAYER_DOWN" : \
    (status == SYS_MQTT_STATUS_SOCK_CLIENT_CONNECTING)?"SOCK_CLIENT_CONNECTING" : \
    (status == SYS_MQTT_STATUS_SOCK_CONNECTED)?"SOCK_CONNECTED" : \
    (status == SYS_MQTT_STATUS_SOCK_OPEN_FAILED)?"SOCK_OPEN_FAILED" : \
    (status == SYS_MQTT_STATUS_MQTT_CONNECTED)?"MQTT_CONNECTED" : \
    (status == SYS_MQTT_STATUS_MQTT_DISCONNECTING)?"MQTT_DISCONNECTING" : \
    (status == SYS_MQTT_STATUS_MQTT_DISCONNECTED)?"MQTT_DISCONNECTED" : \
    (status == SYS_MQTT_STATUS_WAIT_FOR_MQTT_CONACK)?"WAIT_FOR_MQTT_CONACK" : \
    (status == SYS_MQTT_STATUS_SEND_MQTT_CONN)?"SEND_MQTT_CONN" : \
    (status == SYS_MQTT_STATUS_WAIT_FOR_MQTT_SUBACK)?"WAIT_FOR_MQTT_SUBACK" : \
    (status == SYS_MQTT_STATUS_WAIT_FOR_MQTT_PUBACK)?"WAIT_FOR_MQTT_PUBACK" : \
    (status == SYS_MQTT_STATUS_MQTT_CONN_FAILED)?"MQTT_CONN_FAILED" : \
    (status == SYS_MQTT_STATUS_WAIT_FOR_MQTT_UNSUBACK)?"WAIT_FOR_MQTT_UNSUBACK" : "Invalid Status"

static void SysMqtt_Command_Process(int argc, char *argv[])
{
    if (g_u32SysMqttInitDone == 0)
    {
        SYS_CONSOLE_PRINT("\n\n\rMqtt Service Not Initialized");
        return;
    }

    if ((argc >= 2) && (!strcmp((char*) argv[1], "open")))
    {
        if (((argv[2] == NULL)) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> mqtt_broker <broker_name>' Set the MQTT Broker Name");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> mqtt_port <port_number>' Set the MQTT Broker Port");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> tls_enabled <0/1>' 1 if TLS is Enabled, else 0");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> auto_connect <0/1>' 1 if Auto Connect is Enabled, else 0");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> client_id <Client_Id>' Set the Client ID for the MQTT Session");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> username <Username>' Set the Username for the MQTT Session");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> password <Password>' Set the Password for the MQTT Session");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> sub_topic <topic_name>' Set the Subscription Topic Name");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> sub_qos <topic_qos>' Set the Subscription Topic Name");
            SYS_CONSOLE_PRINT("\n\r\t* 'sysmqtt open <instance> apply' Apply the Configuration");
        }
        else
        {
            SYS_MQTT_RESULT ret_val = SYS_NET_FAILURE;
            int inst = strtoul(argv[2], 0, 10);

            if (inst >= SYS_MQTT_MAX_NUM_OF_INSTANCES)
            {
                SYS_CONSOLE_PRINT("\n\rInvalid <instance>");

                return;
            }

            if (!strcmp((char*) argv[3], "auto_connect"))
            {
                g_asSysMqttHandle[inst].sCfgInfo.sBrokerConfig.autoConnect = strtoul(argv[4], 0, 10);

                SYS_NET_SetConfigParam(g_asSysMqttHandle[inst].netSrvcHdl, 0, &g_asSysMqttHandle[inst].sCfgInfo.sBrokerConfig.autoConnect);

                SYS_CONSOLE_MESSAGE("\n\rDone");

                return;
            }

            if (g_asSysMqttHandle[inst].eStatus != SYS_MQTT_STATUS_MQTT_DISCONNECTED)
            {
                SYS_CONSOLE_PRINT("\n\rInstance %d not in DISCONNECTED state", inst);

                return;
            }

            if (!strcmp((char*) argv[3], "mqtt_broker"))
            {
                strcpy((char *) &g_asSysMqttHandle[inst].sCfgInfo.sBrokerConfig.brokerName, (char*) argv[4]);

                SYS_CONSOLE_MESSAGE("\n\rDone");

                return;
            }

            if (!strcmp((char*) argv[3], "mqtt_port"))
            {
                g_asSysMqttHandle[inst].sCfgInfo.sBrokerConfig.serverPort = strtoul(argv[4], 0, 10);

                SYS_CONSOLE_MESSAGE("\n\rDone");

                return;
            }

            if (!strcmp((char*) argv[3], "tls_enabled"))
            {
                g_asSysMqttHandle[inst].sCfgInfo.sBrokerConfig.tlsEnabled = strtoul(argv[4], 0, 10);

                SYS_CONSOLE_MESSAGE("\n\rDone");

                return;
            }

            if (!strcmp((char*) argv[3], "client_id"))
            {
                strcpy((char *) &g_asSysMqttHandle[inst].sCfgInfo.sBrokerConfig.clientId, (char*) argv[4]);

                SYS_CONSOLE_MESSAGE("\n\rDone");

                return;
            }

            if (!strcmp((char*) argv[3], "username"))
            {
                strcpy((char *) &g_asSysMqttHandle[inst].sCfgInfo.sBrokerConfig.username, (char*) argv[4]);

                SYS_CONSOLE_MESSAGE("\n\rDone");

                return;
            }

            if (!strcmp((char*) argv[3], "password"))
            {
                strcpy((char *) &g_asSysMqttHandle[inst].sCfgInfo.sBrokerConfig.password, (char*) argv[4]);

                SYS_CONSOLE_MESSAGE("\n\rDone");

                return;
            }

            if (!strcmp((char*) argv[3], "sub_topic"))
            {
                strcpy((char *) &g_asSysMqttHandle[inst].sCfgInfo.sSubscribeConfig[0].topicName, (char*) argv[4]);

                SYS_CONSOLE_MESSAGE("\n\rDone");

                return;
            }

            if (!strcmp((char*) argv[3], "sub_qos"))
            {
                g_asSysMqttHandle[inst].sCfgInfo.sSubscribeConfig[0].qos = strtoul(argv[2], 0, 10);

                SYS_CONSOLE_MESSAGE("\n\rDone");

                return;
            }

            if (!strcmp((char*) argv[3], "apply"))
            {
                ret_val = SYS_MQTT_CtrlMsg((SYS_MODULE_OBJ) (&g_asSysMqttHandle[inst]),
                                           SYS_MQTT_CTRL_MSG_TYPE_MQTT_CONNECT,
                                           NULL,
                                           0);
                if (ret_val == SYS_MQTT_FAILURE)
                {
                    SYS_CONSOLE_PRINT("\n\rFailed to Open (%d)", ret_val);
                }
            }

            SYS_CONSOLE_MESSAGE("\n\rDone");
        }
    }
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "send")))
    {
        /*Set the required config to ConfigData
         * NOTE: The content set here will be save on "save" command to NVM
         * Applied to MAC on "apply" command */
        if (((argv[2] == NULL)) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");

            SYS_CONSOLE_PRINT("\n\r\t* sysmqtt send <instance> <topic_name> <topic_qos> <retain_enabled> <message>");
        }
        else
        {
            SYS_MQTT_RESULT ret_val = SYS_MQTT_FAILURE;
            SYS_MQTT_PublishTopicCfg sPubTopic;

            int inst = strtoul(argv[2], 0, 10);
            if (inst > SYS_MQTT_MAX_NUM_OF_INSTANCES)
            {
                SYS_CONSOLE_PRINT("\n\rInvalid <instance>");

                return;
            }

            memset(&sPubTopic, 0, sizeof (sPubTopic));

            strcpy((char *) &sPubTopic.topicName, (char*) argv[3]);

            sPubTopic.topicLength = strlen(sPubTopic.topicName);

            sPubTopic.qos = strtoul(argv[4], 0, 10);

            sPubTopic.retain = strtoul(argv[5], 0, 10);

            ret_val = SYS_MQTT_Publish((SYS_MODULE_OBJ) (&g_asSysMqttHandle[inst]),
                                       &sPubTopic,
                                       (char*) argv[6],
                                       strlen(argv[6]));
            if (ret_val < 0)
            {
                SYS_CONSOLE_PRINT("\n\rSend Failed (%d)", ret_val);
            }
        }
    }
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "get")))
    {
        if ((argv[2] == NULL) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");

            SYS_CONSOLE_PRINT("\n\r\t* sysmqtt get info");

            return;
        }
        else if (!strcmp((char*) argv[2], "info"))
        {
            int instCnt = 0;

            for (instCnt = 0; instCnt < SYS_MQTT_MAX_NUM_OF_INSTANCES; instCnt++)
            {
                int subCnt = 0;
                SYS_CONSOLE_PRINT("\n\r*****************************************\n");
                SYS_CONSOLE_PRINT("\n\rMQTT Service Instance: %d", instCnt);
                SYS_CONSOLE_PRINT("\n\rStatus: %s", SYS_MQTT_GET_STATUS_STR(g_asSysMqttHandle[instCnt].eStatus));
                SYS_CONSOLE_PRINT("\n\rBrokerName: %s", g_asSysMqttHandle[instCnt].sCfgInfo.sBrokerConfig.brokerName);
                SYS_CONSOLE_PRINT("\n\rBrokerPort: %d", g_asSysMqttHandle[instCnt].sCfgInfo.sBrokerConfig.serverPort);
                SYS_CONSOLE_PRINT("\n\rClientId: %s", g_asSysMqttHandle[instCnt].sCfgInfo.sBrokerConfig.clientId);
                SYS_CONSOLE_PRINT("\n\rTlsEnabled: %d", g_asSysMqttHandle[instCnt].sCfgInfo.sBrokerConfig.tlsEnabled);
                SYS_CONSOLE_PRINT("\n\rAutoReconnect: %d", g_asSysMqttHandle[instCnt].sCfgInfo.sBrokerConfig.autoConnect);
                SYS_CONSOLE_PRINT("\n\rUsername: %s", g_asSysMqttHandle[instCnt].sCfgInfo.sBrokerConfig.username);
                SYS_CONSOLE_PRINT("\n\rPassword: %s", g_asSysMqttHandle[instCnt].sCfgInfo.sBrokerConfig.password);

                for (subCnt = 0; subCnt < SYS_MQTT_SUB_MAX_TOPICS; subCnt++)
                {
                    if (g_asSysMqttHandle[instCnt].sCfgInfo.sSubscribeConfig[subCnt].entryValid)
                    {
                        SYS_CONSOLE_PRINT("\n\rTopic[%d]: %s", subCnt, g_asSysMqttHandle[instCnt].sCfgInfo.sSubscribeConfig[subCnt].topicName);
                        SYS_CONSOLE_PRINT("\n\rQos[%d]: %d", subCnt, g_asSysMqttHandle[instCnt].sCfgInfo.sSubscribeConfig[subCnt].qos);
                    }
                }
            }

            return;
        }
    }
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "close")))
    {
        /*Set the required config to ConfigData
         * NOTE: The content set here will be save on "save" command to NVM
         * Applied to MAC on "apply" command*/
        if (((argv[2] == NULL)) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysmqtt close <instance>");
        }
        else
        {
            SYS_MQTT_RESULT ret_val = SYS_MQTT_FAILURE;

            int inst = strtoul(argv[2], 0, 10);
            if (inst > SYS_MQTT_MAX_NUM_OF_INSTANCES)
            {
                SYS_CONSOLE_PRINT("\n\rInvalid <instance>");

                return;
            }

            ret_val = SYS_MQTT_CtrlMsg((SYS_MODULE_OBJ) (&g_asSysMqttHandle[inst]),
                                       SYS_MQTT_CTRL_MSG_TYPE_MQTT_DISCONNECT,
                                       NULL,
                                       0);
            if (ret_val == SYS_MQTT_FAILURE)
            {
                SYS_CONSOLE_PRINT("\n\rFailed to Close (%d)", ret_val);
            }

            SYS_CONSOLE_MESSAGE("\n\rDone");
        }
    }
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "unsubscribe")))
    {
        /*Set the required config to ConfigData
         * NOTE: The content set here will be save on "save" command to NVM
         * Applied to MAC on "apply" command*/
        if (((argv[2] == NULL)) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysmqtt unsubscribe <instance> <topic_name>");
        }
        else
        {
            SYS_MQTT_RESULT ret_val = SYS_MQTT_FAILURE;

            int inst = strtoul(argv[2], 0, 10);
            if (inst > SYS_MQTT_MAX_NUM_OF_INSTANCES)
            {
                SYS_CONSOLE_PRINT("\n\rInvalid <instance>");

                return;
            }

            ret_val = SYS_MQTT_CtrlMsg((SYS_MODULE_OBJ) (&g_asSysMqttHandle[inst]),
                                       SYS_MQTT_CTRL_MSG_TYPE_MQTT_UNSUBSCRIBE,
                                       (char *) argv[3],
                                       strlen(argv[3]));
            if (ret_val == SYS_MQTT_FAILURE)
            {
                SYS_CONSOLE_PRINT("\n\rFailed to Close (%d)", ret_val);
            }

            SYS_CONSOLE_MESSAGE("\n\rDone");
        }
    }
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "subscribe")))
    {
        /*Set the required config to ConfigData
         * NOTE: The content set here will be save on "save" command to NVM
         * Applied to MAC on "apply" command*/
        if (((argv[2] == NULL)) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysmqtt subscribe <instance> <topic_name> <topic_qos-0/1/2>");
        }
        else
        {
            SYS_MQTT_RESULT ret_val = SYS_MQTT_FAILURE;
            SYS_MQTT_SubscribeConfig sMqttSubCfg;

            int inst = strtoul(argv[2], 0, 10);
            if (inst > SYS_MQTT_MAX_NUM_OF_INSTANCES)
            {
                SYS_CONSOLE_PRINT("\n\rInvalid <instance>");

                return;
            }

            memset(&sMqttSubCfg, 0, sizeof (sMqttSubCfg));

            strcpy((char *) &sMqttSubCfg.topicName, (char *) argv[3]);

            sMqttSubCfg.qos = strtoul(argv[4], 0, 10);
            ;

            ret_val = SYS_MQTT_CtrlMsg((SYS_MODULE_OBJ) (&g_asSysMqttHandle[inst]),
                                       SYS_MQTT_CTRL_MSG_TYPE_MQTT_SUBSCRIBE,
                                       &sMqttSubCfg,
                                       sizeof (sMqttSubCfg));
            if (ret_val == SYS_MQTT_FAILURE)
            {
                SYS_CONSOLE_PRINT("\n\rFailed to Close (%d)", ret_val);
            }
        }
    }
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "omitpacket")))
    {
        g_OmitPacketType = strtoul(argv[2], 0, 10);
    }
#ifdef SYS_MQTT_ENABLE_DEBUG_PRINT
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "debug")))
    {
        if ((argv[2] == NULL) || (!strcmp((char*) argv[2], "?")))
        {
            SYS_CONSOLE_MESSAGE("\n\rFollowing debug Command:");
            SYS_CONSOLE_MESSAGE("\n\r\t* sysmqtt debug level");
            SYS_CONSOLE_MESSAGE("\n\r\t* sysmqtt debug flow");
            return;
        }
        else if ((!strcmp((char*) argv[2], "level")))
        {
            int32_t appDebugRet = SYS_APPDEBUG_FAILURE;

            g_sMqttAppDbgCfg.logLevel = strtoul(argv[3], 0, 16);

            appDebugRet = SYS_APPDEBUG_CtrlMsg(g_AppDebugHdl,
                                               SYS_APPDEBUG_CTRL_MSG_TYPE_SET_LEVEL,
                                               &g_sMqttAppDbgCfg.logLevel, 4);
            if (appDebugRet != SYS_APPDEBUG_SUCCESS)
            {
                SYS_CONSOLE_PRINT("Failed to set the Level as 0x%x\r\n", g_sMqttAppDbgCfg.logLevel);
            }

            SYS_CONSOLE_MESSAGE("\r\n");

            return;
        }
        else if ((!strcmp((char*) argv[2], "flow")))
        {
            int32_t appDebugRet = SYS_APPDEBUG_FAILURE;

            g_sMqttAppDbgCfg.logFlow = strtoul(argv[3], 0, 16);

            appDebugRet = SYS_APPDEBUG_CtrlMsg(g_AppDebugHdl,
                                               SYS_APPDEBUG_CTRL_MSG_TYPE_SET_FLOW,
                                               &g_sMqttAppDbgCfg.logFlow, 4);
            if (appDebugRet != SYS_APPDEBUG_SUCCESS)
            {
                SYS_CONSOLE_PRINT("Failed to set the Flow as 0x%x\r\n", g_sMqttAppDbgCfg.logFlow);
            }

            SYS_CONSOLE_MESSAGE("\r\n");

            return;
        }
    }
#endif    
    else
    {
        SYS_CONSOLE_MESSAGE("*** Command Processor: unknown command. ***");
    }

    return;
}

static int SysMqttCMDProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) \

{

    SysMqtt_Command_Process(argc, argv);

    return 0;
}

static int SysMqttCMDHelp(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    SYS_CONSOLE_PRINT("SysMqtt commands:\r\n");
    SYS_CONSOLE_PRINT("\n1) sysmqtt open ?\t\t-- To open the sysmqtt service instance\t\r\n");
    SYS_CONSOLE_PRINT("\n2) sysmqtt close ?\t\t-- To close the sysmqtt service instance\t\r\n");
    SYS_CONSOLE_PRINT("\n3) sysmqtt send ?\t\t-- To send message on a topic\t\r\n");
    SYS_CONSOLE_PRINT("\n4) sysmqtt subscribe ?\t\t-- To subscribe to a topic\t\r\n");
    SYS_CONSOLE_PRINT("\n5) sysmqtt unsubscribe ?\t-- To unsubscribe from a topic\t\r\n");
    SYS_CONSOLE_PRINT("\n6) sysmqtt get info\t\t-- To get list of sysmqtt service instances\t\r\n");
    return 0;
}

static const SYS_CMD_DESCRIPTOR g_SysMqttCmdTbl[] ={
    {"sysmqtt", (SYS_CMD_FNC) SysMqttCMDProcessing, ": SysMqtt commands processing"},
    {"sysmqtthelp", (SYS_CMD_FNC) SysMqttCMDHelp, ": SysMqtt commands help "},
};

int32_t SYS_MQTT_Initialize()
{
    memset(g_asSysMqttHandle, 0, sizeof (g_asSysMqttHandle));
    /* 
     ** Create Semaphore for ensuring the SYS NET APIs are re-entrant 
     */
    if (OSAL_SEM_Create(&g_SysMqttSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1) != OSAL_RESULT_TRUE)
    {
        SYS_CONSOLE_MESSAGE("NET_SRVC: Failed to Initialize Service as Semaphore NOT created\r\n");

        return SYS_MQTT_FAILURE;
    }

    /*
     ** Add Sys MQTT Commands to System Command service
     */
    if (!SYS_CMD_ADDGRP(g_SysMqttCmdTbl, sizeof (g_SysMqttCmdTbl) / sizeof (*g_SysMqttCmdTbl), "sysmqtt", ": Sys MQTT commands"))
    {
        SYS_CONSOLE_MESSAGE("MQTT_SRVC: Failed to Initialize Service as SysMqtt Commands NOT created\r\n");

        return SYS_MQTT_FAILURE;
    }

    g_u32SysMqttInitDone = 1;

    return SYS_MQTT_SUCCESS;
}

void* SYS_MQTT_AllocHandle()
{
    uint8_t i = 0;

    OSAL_SEM_Pend(&g_SysMqttSemaphore, OSAL_WAIT_FOREVER);

    for (i = 0; i < SYS_MQTT_MAX_NUM_OF_INSTANCES; i++)
    {
        if (g_asSysMqttHandle[i].eStatus == SYS_MQTT_STATUS_IDLE)
        {
            OSAL_SEM_Post(&g_SysMqttSemaphore);

            SYS_MQTTDEBUG_INFO_PRINT(g_AppDebugHdl, MQTT_CFG, "Assigned g_asSysMqttHandle[%d] (%p)\r\n", i, &g_asSysMqttHandle[i]);

            return &g_asSysMqttHandle[i];
        }
    }

    OSAL_SEM_Post(&g_SysMqttSemaphore);

    return NULL;
}

void SYS_MQTT_FreeHandle(void *handle)
{
    SYS_MQTT_Handle *hdl = (SYS_MQTT_Handle *) handle;

    OSAL_SEM_Pend(&g_SysMqttSemaphore, OSAL_WAIT_FOREVER);

    hdl->eStatus = SYS_MQTT_STATUS_IDLE;

    OSAL_SEM_Post(&g_SysMqttSemaphore);
}

SYS_MODULE_OBJ SYS_MQTT_Connect(SYS_MQTT_Config *cfg,
                                SYS_MQTT_CALLBACK fn,
                                void *cookie)
{
    if (g_u32SysMqttInitDone == 0)
    {
        SYS_CONSOLE_PRINT("\n\n\rMqtt Service Not Initialized");

        return SYS_MODULE_OBJ_INVALID;
    }

    if (g_u32SysMqttInitDone == 1)
    {
#ifdef SYS_MQTT_ENABLE_DEBUG_PRINT
        g_sMqttAppDbgCfg.logLevel = 0;
#ifdef SYS_MQTT_APPDEBUG_ERR_LEVEL_ENABLE        
        g_sMqttAppDbgCfg.logLevel |= APP_LOG_ERROR_LVL;
#endif        
#ifdef SYS_MQTT_APPDEBUG_DBG_LEVEL_ENABLE        
        g_sMqttAppDbgCfg.logLevel |= APP_LOG_DBG_LVL;
#endif        
#ifdef SYS_MQTT_APPDEBUG_INFO_LEVEL_ENABLE        
        g_sMqttAppDbgCfg.logLevel |= APP_LOG_INFO_LVL;
#endif        
#ifdef SYS_MQTT_APPDEBUG_FUNC_LEVEL_ENABLE        
        g_sMqttAppDbgCfg.logLevel |= APP_LOG_FN_EE_LVL;
#endif        
        g_sMqttAppDbgCfg.logFlow = 0;
#ifdef SYS_MQTT_APPDEBUG_CFG_FLOW_ENABLE
        g_sMqttAppDbgCfg.logFlow |= MQTT_CFG;
#endif
#ifdef SYS_MQTT_APPDEBUG_DATA_FLOW_ENABLE
        g_sMqttAppDbgCfg.logFlow |= MQTT_DATA;
#endif
#ifdef SYS_MQTT_APPDEBUG_PAHO_FLOW_ENABLE
        g_sMqttAppDbgCfg.logFlow |= MQTT_PAHO;
#endif
        g_sMqttAppDbgCfg.prefixString = SYS_MQTT_DEBUG_PRESTR;
        g_AppDebugHdl = SYS_APPDEBUG_Open(&g_sMqttAppDbgCfg);
#else
        g_AppDebugHdl = SYS_MODULE_OBJ_INVALID;
#endif   //#ifdef SYS_APPDEBUG_ENABLE
        g_u32SysMqttInitDone = 2;
    }

#ifdef SYS_MQTT_PAHO
    return SYS_MQTT_PAHO_Open(cfg, fn, cookie);
#endif            
    return SYS_MODULE_OBJ_INVALID;
}

void SYS_MQTT_Disconnect(SYS_MODULE_OBJ obj)
{
    if (obj == SYS_MODULE_OBJ_INVALID)
    {
        SYS_MQTTDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_CFG, "Handle Invalid\r\n");

        return;
    }
#ifdef SYS_MQTT_PAHO
    /* Close socket */
    SYS_MQTT_Paho_Close(obj);
#endif
}

SYS_MQTT_STATUS SYS_MQTT_GetStatus(SYS_MODULE_OBJ obj)
{
    SYS_MQTT_Handle *hdl = (SYS_MQTT_Handle*) obj;
    if (obj == SYS_MODULE_OBJ_INVALID)
    {
        SYS_MQTTDEBUG_DBG_PRINT(g_AppDebugHdl, MQTT_CFG, "Handle Invalid\r\n");

        return SYS_MQTT_INVALID_HANDLE;
    }

    return hdl->eStatus;
}

void SYS_MQTT_Task(SYS_MODULE_OBJ obj)
{
#ifdef SYS_MQTT_PAHO
    SYS_MQTT_Paho_Task(obj);
#endif    
}

int32_t SYS_MQTT_CtrlMsg(SYS_MODULE_OBJ hdl, SYS_MQTT_CtrlMsgType eCtrlMsgType, void *data, uint16_t len)
{
#ifdef SYS_MQTT_PAHO
    return SYS_MQTT_Paho_CtrlMsg(hdl, eCtrlMsgType, data, len);
#endif    
    return SYS_MQTT_FAILURE;
}

int32_t SYS_MQTT_Subscribe(SYS_MODULE_OBJ hdl, SYS_MQTT_SubscribeConfig *subConfig)
{
#ifdef SYS_MQTT_PAHO
    return SYS_MQTT_Paho_CtrlMsg(hdl, SYS_MQTT_CTRL_MSG_TYPE_MQTT_SUBSCRIBE, subConfig, sizeof (SYS_MQTT_SubscribeConfig));
#endif    
    return SYS_MQTT_FAILURE;

}

int32_t SYS_MQTT_Unsubscribe(SYS_MODULE_OBJ hdl, char *subTopic)
{
#ifdef SYS_MQTT_PAHO
    return SYS_MQTT_Paho_CtrlMsg(hdl, SYS_MQTT_CTRL_MSG_TYPE_MQTT_UNSUBSCRIBE, subTopic, strlen(subTopic));
#endif    
    return SYS_MQTT_FAILURE;

}

int32_t SYS_MQTT_Publish(SYS_MODULE_OBJ obj, SYS_MQTT_PublishTopicCfg *pubConfig, char *message, uint16_t message_len)
{
#ifdef SYS_MQTT_PAHO
    return SYS_MQTT_Paho_SendMsg(obj, pubConfig, message, message_len);
#endif    
    return SYS_MQTT_FAILURE;
}

