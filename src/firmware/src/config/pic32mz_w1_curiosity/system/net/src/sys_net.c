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

#include "system/net/sys_net.h"
#include "tcpip/sntp.h"

typedef union
{
    UDP_SOCKET_INFO sUdpInfo; /* UDP Socket Info Maintained by the TCP Stack */
    TCP_SOCKET_INFO sTcpInfo; /* TCP Socket Info Maintained by the TCP Stack */
} SYS_NET_SOCKET_INFO;

typedef struct
{
    uint32_t startTime;
    uint32_t timeOut;
} SYS_NET_TimerInfo;

/* 
 ** The below structure is returned as a handle by the init call 
 */
typedef struct
{
    SYS_NET_Config cfg_info; /* config info received at the time of init */
    SYS_NET_SOCKET_INFO sNetInfo; /* Socket Info Maintained by the TCP Stack */
    SYS_NET_CALLBACK callback_fn; /* callback registered with the service */
    void * cookie; /* Cookie to Identify the Instance by the User */
    OSAL_SEM_HANDLE_TYPE InstSemaphore; /* Semaphore for Critical Section */
    int16_t socket; /* socket id received after opening the socket */
    uint8_t status; /* Current state of the service */
    IP_MULTI_ADDRESS server_ip; /* Server IP received after Resolving DNS */
    NET_PRES_SKT_T sock_type;
    SYS_NET_TimerInfo timerInfo;
} SYS_NET_Handle;

static SYS_NET_Handle g_asSysNetHandle[SYS_NET_MAX_NUM_OF_SOCKETS];
static OSAL_SEM_HANDLE_TYPE g_SysNetSemaphore; /* Semaphore for Critical Section */
uint32_t g_u32SysNetInitDone = 0;

#ifdef SYS_NET_ENABLE_DEBUG_PRINT
SYS_APPDEBUG_CONFIG g_sNetAppDbgCfg;
#endif
SYS_MODULE_OBJ g_NetAppDbgHdl;

#ifdef SYS_NET_TLS_ENABLED
#define SYS_NET_GET_STATUS_STR(status)  \
    (status == SYS_NET_STATUS_IDLE)?"IDLE" : \
    (status == SYS_NET_STATUS_RESOLVING_DNS)?"RESOLVING_DNS" : \
    (status == SYS_NET_STATUS_SERVER_AWAITING_CONNECTION)?"SERVER_AWAITING_CONNECTION" : \
    (status == SYS_NET_STATUS_CLIENT_CONNECTING)?"CLIENT_CONNECTING" : \
    (status == SYS_NET_STATUS_CONNECTED)?"CONNECTED" : \
    (status == SYS_NET_STATUS_LOWER_LAYER_DOWN)?"LOWER_LAYER_DOWN" : \
    (status == SYS_NET_STATUS_DNS_RESOLVED)?"DNS_RESOLVED" : \
    (status == SYS_NET_STATUS_WAIT_FOR_SNTP)?"WAIT_FOR_SNTP" : \
    (status == SYS_NET_STATUS_TLS_NEGOTIATING)?"SSL_NEGOTIATING" : \
    (status == SYS_NET_STATUS_SOCK_OPEN_FAILED)?"SOCK_OPEN_FAILED" : \
    (status == SYS_NET_STATUS_DNS_RESOLVE_FAILED)?"DNS_RESOLVE_FAILED" : \
    (status == SYS_NET_STATUS_TLS_NEGOTIATION_FAILED)?"TLS_NEGOTIATION_FAILED" : \
    (status == SYS_NET_STATUS_PEER_SENT_FIN)?"PEER_SENT_FIN" : \
    (status == SYS_NET_STATUS_DISCONNECTED)?"DISCONNECTED" : "Invalid Status"
#else
#define SYS_NET_GET_STATUS_STR(status)  \
    (status == SYS_NET_STATUS_IDLE)?"IDLE" : \
    (status == SYS_NET_STATUS_RESOLVING_DNS)?"RESOLVING_DNS" : \
    (status == SYS_NET_STATUS_SERVER_AWAITING_CONNECTION)?"SERVER_AWAITING_CONNECTION" : \
    (status == SYS_NET_STATUS_CLIENT_CONNECTING)?"CLIENT_CONNECTING" : \
    (status == SYS_NET_STATUS_CONNECTED)?"CONNECTED" : \
    (status == SYS_NET_STATUS_LOWER_LAYER_DOWN)?"LOWER_LAYER_DOWN" : \
    (status == SYS_NET_STATUS_DNS_RESOLVED)?"DNS_RESOLVED" : \
    (status == SYS_NET_STATUS_SOCK_OPEN_FAILED)?"SOCK_OPEN_FAILED" : \
    (status == SYS_NET_STATUS_DNS_RESOLVE_FAILED)?"DNS_RESOLVE_FAILED" : \
    (status == SYS_NET_STATUS_PEER_SENT_FIN)?"PEER_SENT_FIN" : \
    (status == SYS_NET_STATUS_DISCONNECTED)?"DISCONNECTED" : "Invalid Status"
#endif

#define SYS_NET_GET_MODE_STR(mode)  (mode == SYS_NET_MODE_CLIENT)?"CLIENT" : "SERVER"

#define SYS_NET_PERIOIDC_TIMEOUT   30 //Sec
#define SYS_NET_TIMEOUT_CONST (SYS_NET_PERIOIDC_TIMEOUT * SYS_TMR_TickCounterFrequencyGet())

static inline void SYS_NET_SetInstStatus(SYS_NET_Handle *hdl, SYS_NET_STATUS status)
{
    hdl->status = status;
    SYS_NETDEBUG_INFO_PRINT(g_NetAppDbgHdl, NET_CFG, "Handle (0x%p) State (%s)\r\n", hdl, SYS_NET_GET_STATUS_STR(status));
}

void SYS_NET_StartTimer(SYS_NET_Handle *hdl, uint32_t timerInfo)
{
    /* Start the Timer */
    hdl->timerInfo.startTime = SYS_TMR_TickCountGet();

    hdl->timerInfo.timeOut = timerInfo;
}

bool SYS_NET_TimerExpired(SYS_NET_Handle *hdl)
{
    /* Check if Timer is started */
    if (hdl->timerInfo.startTime == 0)
    {
        return false;
    }

    /* Return the true if timer expired */
    return (SYS_TMR_TickCountGet() - hdl->timerInfo.startTime > hdl->timerInfo.timeOut);
}

void SYS_NET_ResetTimer(SYS_NET_Handle *hdl)
{
    /* Reset the Timer */
    hdl->timerInfo.startTime = 0;
}

static void SysNet_Command_Process(int argc, char *argv[])
{
    /* Check if Service is initialized  */
    if (g_u32SysNetInitDone == 0)
    {
        SYS_CONSOLE_PRINT("\n\n\rNet Service Not Initialized");
    }

    if ((argc >= 2) && (!strcmp((char*) argv[1], "open")))
    {
        if (((argv[2] == NULL)) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysnet open <mode> <ip_prot> <host_name> <port> <auto_reconnect> <tls_enable>");
            SYS_CONSOLE_PRINT("\n\r\t* mode\t\t\t- 0 (client)/ 1(server)");
            SYS_CONSOLE_PRINT("\n\r\t* ip_prot\t\t- 0 (udp)/ 1(tcp)");
            SYS_CONSOLE_PRINT("\n\r\t* host_name\t\t- Host Name/ IP Address");
            SYS_CONSOLE_PRINT("\n\r\t* port\t\t\t- Server Port - 1:65535");
            SYS_CONSOLE_PRINT("\n\r\t* auto_reconnect\t\t- 0/1 optional: Default 1");
            SYS_CONSOLE_PRINT("\n\r\t* tls_enable\t\t- 0/1 optional: Default 0");
            SYS_CONSOLE_PRINT("\n\r\t* Example: sysnet 0 1 google.com 443 tls_enable 1 auto_reconnect 1");
            return;
        }
        else
        {
            SYS_NET_Config sCfg;
            SYS_NET_RESULT ret_val = SYS_NET_FAILURE;
            int i = 0;

            memset(&sCfg, 0, sizeof (sCfg));

            /* Initializing the Default values for Optional Parameters */
            sCfg.enable_tls = SYS_NET_DEFAULT_TLS_ENABLE;

            sCfg.enable_reconnect = SYS_NET_DEFAULT_AUTO_RECONNECT;

            sCfg.mode = strtoul(argv[2], 0, 10);
            if (sCfg.mode > SYS_NET_MODE_SERVER)
            {
                SYS_CONSOLE_PRINT("\n\rInvalid <mode>");
                return;
            }

            sCfg.ip_prot = strtoul(argv[3], 0, 10);
            if (sCfg.ip_prot > SYS_NET_IP_PROT_TCP)
            {
                SYS_CONSOLE_PRINT("\n\rInvalid <ip_prot>");
                return;
            }

            strcpy(sCfg.host_name, argv[4]);

            sCfg.port = strtoul(argv[5], 0, 10);

            for (i = 6; i < argc; i = i + 2)
            {
                if (argv[i] != NULL)
                {
                    if (!strcmp((char*) argv[i], "auto_reconnect"))
                    {
                        sCfg.enable_reconnect = strtoul(argv[i + 1], 0, 10);
                        if (sCfg.enable_reconnect > 1)
                        {
                            SYS_CONSOLE_PRINT("\n\rInvalid <auto_reconnect>");
                            return;
                        }
                    }
                    else if (!strcmp((char*) argv[i], "tls_enable"))
                    {
                        sCfg.enable_tls = strtoul(argv[i + 1], 0, 10);
                        if (sCfg.enable_tls > 1)
                        {
                            SYS_CONSOLE_PRINT("\n\rInvalid <enable_tls>");
                            return;
                        }
                    }

                }
            }

            ret_val = SYS_NET_CtrlMsg((SYS_MODULE_OBJ) (&g_asSysNetHandle[0]),
                                      SYS_NET_CTRL_MSG_RECONNECT,
                                      &sCfg,
                                      sizeof (sCfg));
            if (ret_val == SYS_NET_FAILURE)
            {
                SYS_CONSOLE_PRINT("\n\rFailed to Open");
            }

            SYS_CONSOLE_MESSAGE("\n\rDone");
            return;
        }
    }
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "send")))
    {
        /*Set the required config to ConfigData
         * NOTE: The content set here will be save on "save" command to NVM
         * Applied to MAC on "apply" command*/
        if (((argv[2] == NULL)) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysnet send <net_service_instance> <message>");
            return;
        }
        else if (((argv[3] == NULL)) || (!strcmp("?", argv[3])))
        {
            SYS_CONSOLE_PRINT("\n\r\t* sysnet send <net_service_instance> <message>");
            SYS_CONSOLE_PRINT("\n\r<message>: is the message to be sent to the peer");
            return;
        }
        else
        {
            int temp = strtoul(argv[2], 0, 10);
            if (temp >= SYS_NET_MAX_NUM_OF_SOCKETS)
            {
                SYS_CONSOLE_PRINT("\n\rInvalid <net_service_instance>");
                return;
            }

            SYS_NET_SendMsg((SYS_MODULE_OBJ) (&g_asSysNetHandle[temp]),
                            (uint8_t *) argv[3],
                            strlen(argv[3]));
            SYS_CONSOLE_MESSAGE("\n\rDone");
            return;
        }
    }
    if ((argc >= 2) && (!strcmp((char*) argv[1], "close")))
    {
        if (((argv[2] == NULL)) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysnet close <net_service_instance>");
            return;
        }
        else
        {
            int temp = strtoul(argv[2], 0, 10);
            if (temp >= SYS_NET_MAX_NUM_OF_SOCKETS)
            {
                SYS_CONSOLE_PRINT("\n\rInvalid <net_service_instance>");
                return;
            }

            SYS_NET_CtrlMsg((SYS_MODULE_OBJ) (&g_asSysNetHandle[temp]),
                            SYS_NET_CTRL_MSG_DISCONNECT,
                            NULL, 0);
            SYS_CONSOLE_MESSAGE("\n\rDone");
            return;
        }
    }
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "get")))
    {
        if ((argv[2] == NULL) || (!strcmp("?", argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysnet get info");
            return;
        }
        else if (!strcmp((char*) argv[2], "info"))
        {
            uint8_t i = 0;
            uint8_t k = 0;
            int nNets = 0;
            TCPIP_NET_HANDLE netH;

            for (i = 0; i < SYS_NET_MAX_NUM_OF_SOCKETS; i++)
            {
                SYS_CONSOLE_PRINT("\n\n\r*****************************************");
                SYS_CONSOLE_PRINT("\n\rNET Service Instance: %d", i);
                SYS_CONSOLE_PRINT("\n\rStatus: %s", SYS_NET_GET_STATUS_STR(g_asSysNetHandle[i].status));
                SYS_CONSOLE_PRINT("\n\rMode: %s", SYS_NET_GET_MODE_STR(g_asSysNetHandle[i].cfg_info.mode));
                SYS_CONSOLE_PRINT("\n\rSocket ID: %d", g_asSysNetHandle[i].socket);
                SYS_CONSOLE_PRINT("\n\rHost: %s", g_asSysNetHandle[i].cfg_info.host_name);

                if (g_asSysNetHandle[i].cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
                {
                    SYS_CONSOLE_PRINT("\n\rRemote IP: %d.%d.%d.%d",
                                      g_asSysNetHandle[i].sNetInfo.sUdpInfo.remoteIPaddress.v4Add.v[0],
                                      g_asSysNetHandle[i].sNetInfo.sUdpInfo.remoteIPaddress.v4Add.v[1],
                                      g_asSysNetHandle[i].sNetInfo.sUdpInfo.remoteIPaddress.v4Add.v[2],
                                      g_asSysNetHandle[i].sNetInfo.sUdpInfo.remoteIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("\n\rRemote Port: %d", g_asSysNetHandle[i].sNetInfo.sUdpInfo.remotePort);
                    SYS_CONSOLE_PRINT("\n\rLocal Port: %d", g_asSysNetHandle[i].sNetInfo.sUdpInfo.localPort);
                    SYS_CONSOLE_PRINT("\n\rhNet: 0x%x", (uint32_t) (g_asSysNetHandle[i].sNetInfo.sUdpInfo.hNet));

                    /* Find the network interface index from the network handle */
                    nNets = TCPIP_STACK_NumberOfNetworksGet();

                    for (k = 0; k < nNets; k++)
                    {
                        netH = TCPIP_STACK_IndexToNet(k);
                        if (netH == g_asSysNetHandle[i].sNetInfo.sUdpInfo.hNet)
                        {
                            SYS_CONSOLE_PRINT("\n\rNet Intf Index: %x", k);
                        }
                    }
                }
                else if (g_asSysNetHandle[i].cfg_info.ip_prot == SYS_NET_IP_PROT_TCP)
                {
                    SYS_CONSOLE_PRINT("\n\rRemote IP: %d.%d.%d.%d",
                                      g_asSysNetHandle[i].sNetInfo.sTcpInfo.remoteIPaddress.v4Add.v[0],
                                      g_asSysNetHandle[i].sNetInfo.sTcpInfo.remoteIPaddress.v4Add.v[1],
                                      g_asSysNetHandle[i].sNetInfo.sTcpInfo.remoteIPaddress.v4Add.v[2],
                                      g_asSysNetHandle[i].sNetInfo.sTcpInfo.remoteIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("\n\rRemote Port: %d", g_asSysNetHandle[i].sNetInfo.sTcpInfo.remotePort);
                    SYS_CONSOLE_PRINT("\n\rLocal Port: %d", g_asSysNetHandle[i].sNetInfo.sTcpInfo.localPort);
                    SYS_CONSOLE_PRINT("\n\rhNet: 0x%x", (uint32_t) (g_asSysNetHandle[i].sNetInfo.sTcpInfo.hNet));

                    /* Find the network interface index from the network handle */
                    nNets = TCPIP_STACK_NumberOfNetworksGet();

                    for (k = 0; k < nNets; k++)
                    {
                        netH = TCPIP_STACK_IndexToNet(k);
                        if (netH == g_asSysNetHandle[i].sNetInfo.sTcpInfo.hNet)
                        {
                            SYS_CONSOLE_PRINT("\n\rNet Intf Index: %x", k);
                        }
                    }
                }
            }
            return;
        }
    }
#ifdef SYS_NET_ENABLE_DEBUG_PRINT
    else if ((argc >= 2) && (!strcmp((char*) argv[1], "debug")))
    {
        if ((argv[2] == NULL) || (!strcmp((char*) argv[2], "?")))
        {
            SYS_CONSOLE_MESSAGE("\n\rFollowing debug Command:");
            SYS_CONSOLE_MESSAGE("\n\r\t* sysnet debug level");
            SYS_CONSOLE_MESSAGE("\n\r\t* sysnet debug flow");
            return;
        }
        else if ((!strcmp((char*) argv[2], "level")))
        {
            int32_t appDebugRet = SYS_APPDEBUG_FAILURE;

            g_sNetAppDbgCfg.logLevel = strtoul(argv[3], 0, 16);

            appDebugRet = SYS_APPDEBUG_CtrlMsg(g_NetAppDbgHdl,
                                               SYS_APPDEBUG_CTRL_MSG_TYPE_SET_LEVEL,
                                               &g_sNetAppDbgCfg.logLevel, 4);
            if (appDebugRet != SYS_APPDEBUG_SUCCESS)
            {
                SYS_CONSOLE_PRINT("Failed to set the Level as 0x%x\r\n", g_sNetAppDbgCfg.logLevel);
            }

            SYS_CONSOLE_MESSAGE("\r\n");

            return;
        }
        else if ((!strcmp((char*) argv[2], "flow")))
        {
            int32_t appDebugRet = SYS_APPDEBUG_FAILURE;

            g_sNetAppDbgCfg.logFlow = strtoul(argv[3], 0, 16);

            appDebugRet = SYS_APPDEBUG_CtrlMsg(g_NetAppDbgHdl,
                                               SYS_APPDEBUG_CTRL_MSG_TYPE_SET_FLOW,
                                               &g_sNetAppDbgCfg.logFlow, 4);
            if (appDebugRet != SYS_APPDEBUG_SUCCESS)
            {
                SYS_CONSOLE_PRINT("Failed to set the Flow as 0x%x\r\n", g_sNetAppDbgCfg.logFlow);
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

static int SysNetCMDProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) \

{

    SysNet_Command_Process(argc, argv);

    return 0;
}

static int SysNetCMDHelp(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    SYS_CONSOLE_PRINT("SysNet commands:\r\n");
    SYS_CONSOLE_PRINT("\n1) sysnet open ?\t\t-- To open the sysnet service\t\r\n");
    SYS_CONSOLE_PRINT("\n2) sysnet close ?\t\t-- To close the sysnet service\t\r\n");
    SYS_CONSOLE_PRINT("\n3) sysnet send ?\t\t-- To send message via the sysnet service\t\r\n");
    SYS_CONSOLE_PRINT("\n4) sysnet get info ?\t\t-- To get list of sysnet service instances\t\r\n");
    return 0;
}

static const SYS_CMD_DESCRIPTOR g_SysNetCmdTbl[] = {
    {"sysnet", (SYS_CMD_FNC) SysNetCMDProcessing, ": SysNet commands processing"},
    {"sysnethelp", (SYS_CMD_FNC) SysNetCMDHelp, ": SysNet commands help "},
};

int32_t SYS_NET_Initialize()
{
    memset(g_asSysNetHandle, 0, sizeof (g_asSysNetHandle));

    /* Create Semaphore for ensuring the SYS NET APIs are re-entrant */
    if (OSAL_SEM_Create(&g_SysNetSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1) != OSAL_RESULT_TRUE)
    {
        SYS_CONSOLE_MESSAGE("NET_SRVC: Failed to Initialize Service as Semaphore NOT created\r\n");
        return SYS_NET_FAILURE;
    }

    /* Add Sys NET Commands to System Command service */
    if (!SYS_CMD_ADDGRP(g_SysNetCmdTbl, sizeof (g_SysNetCmdTbl) / sizeof (*g_SysNetCmdTbl), "sysnet", ": Sys NET commands"))
    {
        SYS_CONSOLE_MESSAGE("NET_SRVC: Failed to Initialize Service as SysNet Commands NOT created\r\n");
        return SYS_NET_FAILURE;
    }

    /* Set the initialization flag to 1  */
    g_u32SysNetInitDone = 1;

    return SYS_NET_SUCCESS;
}

void SYS_NET_Deinitialize()
{
    /* Delete Semaphore */
    OSAL_SEM_Delete(&g_SysNetSemaphore);
}

static void* SYS_NET_AllocHandle()
{
    uint8_t i = 0;

    /* Take Semaphore */
    OSAL_SEM_Pend(&g_SysNetSemaphore, OSAL_WAIT_FOREVER);

    for (i = 0; i < SYS_NET_MAX_NUM_OF_SOCKETS; i++)
    {
        /* Search for a free handle */
        if (g_asSysNetHandle[i].status == SYS_NET_STATUS_IDLE)
        {
            /* Give Semaphore */
            OSAL_SEM_Post(&g_SysNetSemaphore);

            SYS_NETDEBUG_INFO_PRINT(g_NetAppDbgHdl, NET_CFG, "Assigned g_asSysNetHandle[%d] (%p)\r\n", i, &g_asSysNetHandle[i]);

            return &g_asSysNetHandle[i];
        }
    }

    /* Give Semaphore */
    OSAL_SEM_Post(&g_SysNetSemaphore);

    /* No Free Handle found */
    return NULL;
}

static void SYS_NET_FreeHandle(void *handle)
{
    SYS_NET_Handle *hdl = (SYS_NET_Handle *) handle;

    /* Take Semaphore */
    OSAL_SEM_Pend(&g_SysNetSemaphore, OSAL_WAIT_FOREVER);

    /* Free the Handle */
    hdl->status = SYS_NET_STATUS_IDLE;

    /* Give Semaphore */
    OSAL_SEM_Post(&g_SysNetSemaphore);
}

/*
 ** Take a Semaphore before entering Critical Section
 */
static int32_t SYS_NET_TakeSemaphore(SYS_NET_Handle *hdl)
{
    return OSAL_SEM_Pend(&hdl->InstSemaphore, OSAL_WAIT_FOREVER);
}

/*
 ** Give the Semaphore while leaving Critical Section
 */
static void SYS_NET_GiveSemaphore(SYS_NET_Handle *hdl)
{
    OSAL_SEM_Post(&hdl->InstSemaphore);
}

static void SYS_NET_SetSockType(SYS_NET_Handle *hdl)
{
    hdl->sock_type = 0;

    hdl->sock_type |= NET_PRES_SKT_UNENCRYPTED;

    /* Check if the Socket is in Client or Server Mode */
    if (hdl->cfg_info.mode == SYS_NET_MODE_CLIENT)
    {
        hdl->sock_type |= NET_PRES_SKT_CLIENT;
    }
    else
    {
        hdl->sock_type |= NET_PRES_SKT_SERVER;
    }

    /* Check if the Socket is for TCP or UDP Protocol */
    if (hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_TCP)
    {
        hdl->sock_type |= NET_PRES_SKT_STREAM;
    }
    else
    {
        hdl->sock_type |= NET_PRES_SKT_DATAGRAM;
    }
}

static bool SYS_NET_Ll_Status(SYS_NET_Handle *hdl)
{
    TCPIP_NET_HANDLE hNet = TCPIP_STACK_IndexToNet(SYS_NET_DEFAULT_NET_INTF);

    /* Check if the Lower Interface is up or not */
    if (TCPIP_STACK_NetIsLinked(hNet) == false)
    {
        return false;
    }
	
    /* Check if the IP Layer is Ready */
    if (TCPIP_STACK_NetIsReady(hNet) == false)
    {
        return false;
    }

    return true;
}

static bool SYS_NET_Ll_Link_Status(SYS_NET_Handle *hdl)
{
    TCPIP_NET_HANDLE hNet = TCPIP_STACK_IndexToNet(SYS_NET_DEFAULT_NET_INTF);

    /* We Do not Check Lower Layer Status in case of TCP 
            since KeepAlive takes care of it */
    if (hdl->cfg_info.ip_prot != SYS_NET_IP_PROT_UDP)
    {
        return true;
    }

    /* Check if the Lower Interface is up or not */
    if (TCPIP_STACK_NetIsLinked(hNet) == false)
    {
        SYS_NETDEBUG_INFO_PRINT(g_NetAppDbgHdl, NET_CFG, "Lower Interface Not UP");

        return false;
    }

    return true;
}

TCPIP_DNS_RESULT SYS_NET_DNS_Resolve(SYS_NET_Handle *hdl)
{
    TCPIP_DNS_RESULT result = TCPIP_DNS_Resolve(hdl->cfg_info.host_name, TCPIP_DNS_TYPE_A);
    if (result == TCPIP_DNS_RES_NAME_IS_IPADDRESS)
    {
        /* If Host Name is IP Address itself */

        if ((TCPIP_Helper_StringToIPAddress(hdl->cfg_info.host_name, &hdl->server_ip.v4Add)) ||
                (TCPIP_Helper_StringToIPv6Address(hdl->cfg_info.host_name, &hdl->server_ip.v6Add)))
        {
            /* In case the Host Name is the IP Address itself */
            SYS_NETDEBUG_INFO_PRINT(g_NetAppDbgHdl, NET_CFG, "DNS Resolved; IP = %s", hdl->cfg_info.host_name);

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DNS_RESOLVED);

            return (SYS_MODULE_OBJ) hdl;
        }

        /* In case the Host Name is the IP Address itself; This should never come */
        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DNS_RESOLVE_FAILED);

        return result;
    }

    /* If Host name could not be resolved */
    if (result < 0)
    {
        if (hdl->cfg_info.enable_reconnect == 0)
        {
            /* DNS cannot be resolved */
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Could Not Resolve DNS = %d (TCPIP_DNS_RESULT)\r\n", result);

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DNS_RESOLVE_FAILED);
        }

        return result;
    }

    /* Wait for the DNS Client to resolve the Host Name */
    SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_RESOLVING_DNS);

    return result;
}

void SYS_NET_NetPres_Signal(NET_PRES_SKT_HANDLE_T handle, NET_PRES_SIGNAL_HANDLE hNet,
                            uint16_t sigType, const void* param)
{
    /* Peer sent a FIN to close the connection */
    if (sigType & TCPIP_TCP_SIGNAL_RX_FIN)
    {
        SYS_NET_Handle *hdl = (SYS_NET_Handle *) param;

        SYS_NETDEBUG_DBG_PRINT(g_NetAppDbgHdl, NET_CFG, "Received FIN from Peer\r\n");

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_PEER_SENT_FIN);
    }
}

bool SYS_NET_Set_Sock_Option(SYS_NET_Handle *hdl)
{
    bool ret;
    TCP_OPTION_KEEP_ALIVE_DATA sKeepAliveData;

    /* KeepAlive Valid only TCP Connection */
    if (hdl->cfg_info.ip_prot != SYS_NET_IP_PROT_TCP)
    {
        return true;
    }

    memset(&sKeepAliveData, 0, sizeof (sKeepAliveData));

    /* Enable KeepAlive Timer for TCP Connection */
    sKeepAliveData.keepAliveEnable = true;

    ret = NET_PRES_SocketOptionsSet(hdl->socket, TCP_OPTION_KEEP_ALIVE, &sKeepAliveData);

    return ret;
}

SYS_MODULE_OBJ SYS_NET_Open(SYS_NET_Config *cfg, SYS_NET_CALLBACK net_cb, void *cookie)
{
    SYS_NET_Handle *hdl = NULL;

    /* Check if Service is initialized  */
    if (g_u32SysNetInitDone == 0)
    {
        SYS_CONSOLE_PRINT("\n\n\rMqtt Service Not Initialized");

        return SYS_MODULE_OBJ_INVALID;
    }

    /* Check if the Debug Logs need to be enabled */
    if (g_u32SysNetInitDone == 1)
    {
#ifdef SYS_NET_ENABLE_DEBUG_PRINT
        g_sNetAppDbgCfg.logLevel = 0;
#ifdef SYS_NET_APPDEBUG_ERR_LEVEL_ENABLE        
        g_sNetAppDbgCfg.logLevel |= APP_LOG_ERROR_LVL;
#endif        
#ifdef SYS_NET_APPDEBUG_DBG_LEVEL_ENABLE        
        g_sNetAppDbgCfg.logLevel |= APP_LOG_DBG_LVL;
#endif        
#ifdef SYS_NET_APPDEBUG_INFO_LEVEL_ENABLE        
        g_sNetAppDbgCfg.logLevel |= APP_LOG_INFO_LVL;
#endif        
#ifdef SYS_NET_APPDEBUG_FUNC_LEVEL_ENABLE        
        g_sNetAppDbgCfg.logLevel |= APP_LOG_FN_EE_LVL;
#endif        
        g_sNetAppDbgCfg.logFlow = 0;
#ifdef SYS_NET_APPDEBUG_CFG_FLOW_ENABLE
        g_sNetAppDbgCfg.logFlow |= NET_CFG;
#endif
#ifdef SYS_NET_APPDEBUG_DATA_FLOW_ENABLE
        g_sNetAppDbgCfg.logFlow |= NET_DATA;
#endif

        g_sNetAppDbgCfg.prefixString = SYS_NET_DEBUG_PRESTR;
        g_NetAppDbgHdl = SYS_APPDEBUG_Open(&g_sNetAppDbgCfg);
#else
        g_NetAppDbgHdl = SYS_MODULE_OBJ_INVALID;
#endif  //#ifdef SYS_NET_ENABLE_DEBUG_PRINT 
        g_u32SysNetInitDone = 2;
    }

    /* Allocate Handle - hdl */
    hdl = SYS_NET_AllocHandle();
    if (hdl == NULL)
    {
        SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Failed to allocate Handle\r\n");

        return SYS_MODULE_OBJ_INVALID;
    }

    /* Create Semaphore for ensuring the SYS NET APIs are re-entrant */
    if (OSAL_SEM_Create(&hdl->InstSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1) != OSAL_RESULT_TRUE)
    {
        /* Free Handle */
        SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Failed to create Semaphore\r\n");

        SYS_NET_FreeHandle(hdl);

        return SYS_MODULE_OBJ_INVALID;
    }

    /* Copy the config info and the fn ptr into the Handle */
    if (cfg == NULL)
    {
        memcpy(&hdl->cfg_info, &g_sSysNetConfig, sizeof (SYS_NET_Config));
    }
    else
    {
        memcpy(&hdl->cfg_info, cfg, sizeof (SYS_NET_Config));
    }

    hdl->cookie = cookie;

    hdl->callback_fn = net_cb;

    /* Set Sock type based on Mode, IP Protocol, and TLS enabled or not */
    SYS_NET_SetSockType(hdl);

    /* Check if the NET IP Stack is UP */
    if (SYS_NET_Ll_Status(hdl) == false)
    {
        SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "TCPIP Stack Not UP\r\n");

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_LOWER_LAYER_DOWN);

        return (SYS_MODULE_OBJ) hdl;
    }

    /* Open the Socket based on the Mode */
    if (hdl->cfg_info.mode == SYS_NET_MODE_CLIENT)
    {
        SYS_NET_DNS_Resolve(hdl);

        return (SYS_MODULE_OBJ) hdl;
    }

    /* In case the Mode is NET Server, Open Socket and Wait for Connection */
    hdl->socket = NET_PRES_SocketOpen(0,
                                      hdl->sock_type,
                                      TCPIP_DNS_TYPE_A,
                                      hdl->cfg_info.port,
                                      0,
                                      NULL);
    if (hdl->socket == INVALID_SOCKET)
    {
        SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "ServerOpen failed!\r\n");

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_SOCK_OPEN_FAILED);

        return (SYS_MODULE_OBJ) hdl;
    }

    if (SYS_NET_Set_Sock_Option(hdl) == false)
    {
        SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Set Sock Option Failed\r\n");
    }

    /* Register the CB with NetPres */
    if (NET_PRES_SocketSignalHandlerRegister(hdl->socket, 0xffff, SYS_NET_NetPres_Signal, hdl) == NULL)
    {
        SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Handler Registration failed!\r\n");
    }

    /* Wait for Connection */
    SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_SERVER_AWAITING_CONNECTION);

    return (SYS_MODULE_OBJ) hdl;
}

static void SYS_NET_Client_Task(SYS_NET_Handle *hdl)
{
    if (SYS_NET_TakeSemaphore(hdl) == 0)
    {
        return;
    }

    switch (hdl->status)
    {
        /* Lower Layer is Down */
    case SYS_NET_STATUS_LOWER_LAYER_DOWN:
    {
        if (SYS_NET_Ll_Status(hdl) == false)
        {
            SYS_NET_GiveSemaphore(hdl);

            return;
        }
        SYS_NET_DNS_Resolve(hdl);
    }
        break;

        /* Waiting for DNS Client to resolve the Host Name */
    case SYS_NET_STATUS_RESOLVING_DNS:
    {
        TCPIP_DNS_RESULT result =
                TCPIP_DNS_IsResolved(hdl->cfg_info.host_name,
                                     &hdl->server_ip,
                                     TCPIP_DNS_TYPE_A);
        switch (result)
        {
            /* DNS Resolved */
        case TCPIP_DNS_RES_OK:
        {
            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DNS_RESOLVED);
        }
            break;

            /* DNS Resolution Pending */
        case TCPIP_DNS_RES_PENDING:
            break;

            /* DNS Timed out */
        case TCPIP_DNS_RES_SERVER_TMO:
        {
            if (hdl->cfg_info.enable_reconnect == 0)
            {
                SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Could Not Resolve DNS\r\n");

                SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DNS_RESOLVE_FAILED);

                break;
            }
            result = SYS_NET_DNS_Resolve(hdl);
        }
            break;

        default:
        {
            SYS_NETDEBUG_DBG_PRINT(g_NetAppDbgHdl, NET_CFG, "Could Not Resolve DNS = %d (TCPIP_DNS_RESULT)\r\n", result);

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DNS_RESOLVE_FAILED);
        }
        }
    }
        break;

        /* DNS Resolved; Open Socket */
    case SYS_NET_STATUS_DNS_RESOLVED:
    {
        /* We now have an IPv4 Address; Open a socket */
        hdl->socket = NET_PRES_SocketOpen(0,
                                          hdl->sock_type,
                                          TCPIP_DNS_TYPE_A,
                                          hdl->cfg_info.port,
                                          (NET_PRES_ADDRESS *) & hdl->server_ip,
                                          NULL);
        if (hdl->socket == INVALID_SOCKET)
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "ClientOpen failed\r\n");

            /* Check if AutoReconnect is enabled and we need 
                    to retry Opeing the socket */
            if (hdl->cfg_info.enable_reconnect == 0)
            {
                SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Could Not Open Socket\r\n");

                SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_SOCK_OPEN_FAILED);
            }

            SYS_NET_GiveSemaphore(hdl);

            return;
        }

        NET_PRES_SocketWasReset(hdl->socket);
        if (hdl->socket == INVALID_SOCKET)
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Could not create socket - aborting\r\n");

            return;
        }

        /* Set Socket option for KeepAlive Timer */
        if (SYS_NET_Set_Sock_Option(hdl) == false)
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Set Sock Option Failed\r\n");
        }

        /* Register the CB with NetPres */
        if (NET_PRES_SocketSignalHandlerRegister(hdl->socket, 0xffff, SYS_NET_NetPres_Signal, hdl) == NULL)
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Handler Registration failed!\r\n");
        }

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_CLIENT_CONNECTING);
    }
        break;

        /* Client Connecting to Server */
    case SYS_NET_STATUS_CLIENT_CONNECTING:
    {
        if ((!NET_PRES_SocketIsConnected(hdl->socket)) || (!SYS_NET_Ll_Link_Status(hdl)))
        {
            break;
        }

#ifdef SYS_NET_TLS_ENABLED
        /* Check if it is a secured connection */
        if (hdl->cfg_info.enable_tls)
        {
            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_WAIT_FOR_SNTP);
            break;
        }
#endif            
        /* Get Info related to the Server */
        if (hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
        {
            NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sUdpInfo);

            memcpy(&hdl->server_ip, &hdl->sNetInfo.sUdpInfo.remoteIPaddress, sizeof (hdl->server_ip));
        }
        else
        {
            NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sTcpInfo);

            memcpy(&hdl->server_ip, &hdl->sNetInfo.sTcpInfo.remoteIPaddress, sizeof (hdl->server_ip));
        }

        /* Set the state to Connected */
        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_CONNECTED);

        SYS_NET_GiveSemaphore(hdl);

        /* Call the Application CB to give 'Connected' event */
        if (hdl->callback_fn)
        {
            hdl->callback_fn(SYS_NET_EVNT_CONNECTED, NULL, hdl->cookie);
        }

        return;
    }
        break;

#ifdef SYS_NET_TLS_ENABLED
        /* Waiting for SNTP updates */
    case SYS_NET_STATUS_WAIT_FOR_SNTP:
    {
        uint32_t time = TCPIP_SNTP_UTCSecondsGet();
        if (time == 0)
        {
            /* SNTP Time Stamp NOT Available */
            break;
        }

        if (NET_PRES_SocketEncryptSocket(hdl->socket) != true)
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "SSL Connection Negotiation Failed; Aborting\r\n");

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_TLS_NEGOTIATION_FAILED);

            break;
        }

        SYS_NET_StartTimer(hdl, SYS_NET_TIMEOUT_CONST);

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_TLS_NEGOTIATING);
    }
        break;

        /* Waiting for TLS Negotiations to finish */
    case SYS_NET_STATUS_TLS_NEGOTIATING:
    {
        if (SYS_NET_TimerExpired(hdl) == true)
        {
            SYS_NET_ResetTimer(hdl);

            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "SSL Connection Negotiation Failed - Timer Expired\r\n");

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_TLS_NEGOTIATION_FAILED);

            break;
        }

        if (NET_PRES_SocketIsNegotiatingEncryption(hdl->socket))
        {
            break;
        }

        if (!NET_PRES_SocketIsSecure(hdl->socket))
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "SSL Connection Negotiation Failed - Aborting\r\n");

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_TLS_NEGOTIATION_FAILED);

            break;
        }

        if (hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
        {
            NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sUdpInfo);
        }
        else
        {
            NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sTcpInfo);
        }

        SYS_NET_ResetTimer(hdl);

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_CONNECTED);

        SYS_NET_GiveSemaphore(hdl);

        /* Call the Application CB to give 'Connected' event */
        if (hdl->callback_fn)
        {
            hdl->callback_fn(SYS_NET_EVNT_CONNECTED, NULL, hdl->cookie);
        }

        return;
    }
        break;
#endif

        /* Connected State */
    case SYS_NET_STATUS_CONNECTED:
    {
        if ((!NET_PRES_SocketIsConnected(hdl->socket)) || (!SYS_NET_Ll_Link_Status(hdl)))
        {
            /* Close socket */
            NET_PRES_SocketClose(hdl->socket);

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DISCONNECTED);

            SYS_NET_GiveSemaphore(hdl);

            /* Call the Application CB to give 'Disconnected' event */
            if (hdl->callback_fn)
            {
                hdl->callback_fn(SYS_NET_EVNT_DISCONNECTED, NULL, hdl->cookie);
            }

            SYS_NET_TakeSemaphore(hdl);

            if (hdl->cfg_info.enable_reconnect)
            {               
                SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_LOWER_LAYER_DOWN);
            }

            break;
        }

        if (NET_PRES_SocketReadIsReady(hdl->socket))
        {
            SYS_NET_GiveSemaphore(hdl);

            /* Call the Application CB to give 'Received Data' event */
            if (hdl->callback_fn)
            {
                hdl->callback_fn(SYS_NET_EVNT_RCVD_DATA, NULL, hdl->cookie);
            }
            return;
        }
    }
        break;

#ifdef SYS_NET_TLS_ENABLED		
        /* TLS Negotiations failed */
    case SYS_NET_STATUS_TLS_NEGOTIATION_FAILED:
    {
        /* Close socket */
        NET_PRES_SocketClose(hdl->socket);

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DISCONNECTED);

        /* Call the Application CB to give 'SSL Negotiation Failed' event */
        if (hdl->callback_fn)
        {
            hdl->callback_fn(SYS_NET_EVNT_SSL_FAILED, NULL, hdl->cookie);
        }

        if (hdl->cfg_info.enable_reconnect)
        {
            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_LOWER_LAYER_DOWN);
        }
    }
        break;
#endif

        /* DNS Could not be resolved */
    case SYS_NET_STATUS_DNS_RESOLVE_FAILED:
    {
        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DISCONNECTED);

        /* Call the Application CB to give 'DNS Resolve Failed' event */
        if (hdl->callback_fn)
        {
            hdl->callback_fn(SYS_NET_EVNT_DNS_RESOLVE_FAILED, NULL, hdl->cookie);
        }

        if (hdl->cfg_info.enable_reconnect)
        {
            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_LOWER_LAYER_DOWN);
        }
    }
        break;

        /* Socket Open failed */
    case SYS_NET_STATUS_SOCK_OPEN_FAILED:
    {
        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DISCONNECTED);

        /* Call the Application CB to give 'Socket Open Failed' event */
        if (hdl->callback_fn)
        {
            hdl->callback_fn(SYS_NET_EVNT_SOCK_OPEN_FAILED, NULL, hdl->cookie);
        }
    }
        break;

        /* Peer sent a FIN */
    case SYS_NET_STATUS_PEER_SENT_FIN:
    {
        if (NET_PRES_SocketReadIsReady(hdl->socket))
        {
            SYS_NET_GiveSemaphore(hdl);

            /* Call the Application CB to give 'Received Data' event */
            if (hdl->callback_fn)
            {
                hdl->callback_fn(SYS_NET_EVNT_RCVD_DATA, NULL, hdl->cookie);
            }
            return;
        }

        /* Close socket */
        NET_PRES_SocketClose(hdl->socket);

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DISCONNECTED);

        SYS_NET_GiveSemaphore(hdl);

        /* Call the Application CB to give 'Disconnected' event */
        if (hdl->callback_fn)
        {
            hdl->callback_fn(SYS_NET_EVNT_DISCONNECTED, NULL, hdl->cookie);
        }

        SYS_NET_TakeSemaphore(hdl);

        if (hdl->cfg_info.enable_reconnect)
        {
            hdl->socket = NET_PRES_SocketOpen(0,
                                              hdl->sock_type,
                                              TCPIP_DNS_TYPE_A,
                                              hdl->cfg_info.port,
                                              (NET_PRES_ADDRESS*) & hdl->server_ip,
                                              NULL);
            if (hdl->socket == INVALID_SOCKET)
            {
                SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "ClientOpen failed!\r\n");
            }
            else
                SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_CLIENT_CONNECTING);

            if (SYS_NET_Set_Sock_Option(hdl) == false)
            {
                SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Set Sock Option Failed\r\n");
            }

            /* Register the CB with NetPres */
            if (NET_PRES_SocketSignalHandlerRegister(hdl->socket, 0xffff, SYS_NET_NetPres_Signal, hdl) == NULL)
            {
                SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Handler Registration failed!\r\n");
            }
        }
    }
        break;

    default:
        break;

    }

    SYS_NET_GiveSemaphore(hdl);
    return;
}

static void SYS_NET_Server_Task(SYS_NET_Handle *hdl)
{
    if (SYS_NET_TakeSemaphore(hdl) == 0)
    {
        return;
    }

    switch (hdl->status)
    {
        /* Lower Layer is Down */
    case SYS_NET_STATUS_LOWER_LAYER_DOWN:
    {
        if (SYS_NET_Ll_Status(hdl) == false)
        {
            SYS_NET_GiveSemaphore(hdl);

            return;
        }

        /* In case the Mode is NET Server, Open Socket and Wait for Connection */
        hdl->socket = NET_PRES_SocketOpen(0,
                                          hdl->sock_type,
                                          TCPIP_DNS_TYPE_A,
                                          hdl->cfg_info.port,
                                          0,
                                          NULL);
        if (hdl->socket == INVALID_SOCKET)
        {
            /* Failed to open a server socket */
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "ServerOpen failed!\r\n");

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_SOCK_OPEN_FAILED);

            SYS_NET_GiveSemaphore(hdl);

            return;
        }

        if (SYS_NET_Set_Sock_Option(hdl) == false)
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Set Sock Option Failed\r\n");
        }

        /* Register the CB with NetPres */
        if (NET_PRES_SocketSignalHandlerRegister(hdl->socket, 0xffff, SYS_NET_NetPres_Signal, hdl) == NULL)
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Handler Registration failed!\r\n");
        }

        /* Wait for Connection */
        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_SERVER_AWAITING_CONNECTION);

        SYS_NET_GiveSemaphore(hdl);

        return;
    }
        break;

        /* Server is Awaiting Connection from a client */
    case SYS_NET_STATUS_SERVER_AWAITING_CONNECTION:
    {
        if ((!NET_PRES_SocketIsConnected(hdl->socket)) || (!SYS_NET_Ll_Link_Status(hdl)))
        {
            SYS_NET_GiveSemaphore(hdl);

            return;
        }
        else
        {
#ifdef SYS_NET_TLS_ENABLED				
            if (hdl->cfg_info.enable_tls)
            {
                SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_WAIT_FOR_SNTP);

                break;
            }
#endif
            if (hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
            {
                NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sUdpInfo);
            }
            else
            {
                NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sTcpInfo);
            }

            /* We got a connection */
            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_CONNECTED);

            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Received a connection\r\n");

            SYS_NET_GiveSemaphore(hdl);

            /* Call the Application CB to give 'Connected' event */
            if (hdl->callback_fn)
            {
                hdl->callback_fn(SYS_NET_EVNT_CONNECTED, hdl, hdl->cookie);
            }

            return;
        }
    }
        break;

#ifdef SYS_NET_TLS_ENABLED		
        /* Waiting for SNTP updates */
    case SYS_NET_STATUS_WAIT_FOR_SNTP:
    {
        uint32_t time = TCPIP_SNTP_UTCSecondsGet();
        if (time == 0)
        {
            /* SNTP Time Stamp NOT Available */
            break;
        }

        if (NET_PRES_SocketEncryptSocket(hdl->socket) != true)
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "SSL Connection Negotiation Failed; Aborting\r\n");

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_TLS_NEGOTIATION_FAILED);

            break;
        }

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_TLS_NEGOTIATING);
    }
        break;

        /* Waiting for TLS Negotiations to finish */
    case SYS_NET_STATUS_TLS_NEGOTIATING:
    {
        if (NET_PRES_SocketIsNegotiatingEncryption(hdl->socket))
        {
            break;
        }

        if (!NET_PRES_SocketIsSecure(hdl->socket))
        {
            SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "SSL Connection Negotiation Failed - Aborting\r\n");

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_TLS_NEGOTIATION_FAILED);

            break;
        }

        if (hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
        {
            NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sUdpInfo);
        }
        else
        {
            NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sTcpInfo);
        }

        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_CONNECTED);

        SYS_NET_GiveSemaphore(hdl);

        /* Call the Application CB to give 'Connected' event */
        if (hdl->callback_fn)
        {
            hdl->callback_fn(SYS_NET_EVNT_CONNECTED, hdl, hdl->cookie);
        }

        return;
    }
        break;
#endif				

        /* Server is connected to a client */
    case SYS_NET_STATUS_CONNECTED:
    {
        if ((!NET_PRES_SocketIsConnected(hdl->socket)) || (!SYS_NET_Ll_Link_Status(hdl)))
        {
            SYS_NETDEBUG_DBG_PRINT(g_NetAppDbgHdl, NET_CFG, "Socket got Disconnected\r\n");

            /* Close socket */
            NET_PRES_SocketClose(hdl->socket);

            /* Change the State */
            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DISCONNECTED);

            SYS_NET_GiveSemaphore(hdl);

            /* Call the Application CB to give 'Disconnected' event */
            if (hdl->callback_fn)
            {
                hdl->callback_fn(SYS_NET_EVNT_DISCONNECTED, hdl, hdl->cookie);
            }

            SYS_NET_TakeSemaphore(hdl);

            if (hdl->cfg_info.enable_reconnect)
            {
                /* In case the Mode is NET Server, Open Socket and Wait for Connection */
                hdl->socket = NET_PRES_SocketOpen(0,
                                                  hdl->sock_type,
                                                  TCPIP_DNS_TYPE_A,
                                                  hdl->cfg_info.port,
                                                  0,
                                                  NULL);
                if (hdl->socket == INVALID_SOCKET)
                {
                    SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "SYS_NET_TCPIP_ServerOpen failed!\r\n");
                }
                else
                    SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_SERVER_AWAITING_CONNECTION);

                if (SYS_NET_Set_Sock_Option(hdl) == false)
                {
                    SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Set Sock Option Failed\r\n");
                }

                /* Register the CB with NetPres */
                if (NET_PRES_SocketSignalHandlerRegister(hdl->socket, 0xffff, SYS_NET_NetPres_Signal, hdl) == NULL)
                {
                    SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Handler Registration failed!\r\n");
                }
            }
            SYS_NET_GiveSemaphore(hdl);
            return;
        }

        if (NET_PRES_SocketReadIsReady(hdl->socket))
        {
            SYS_NET_GiveSemaphore(hdl);

            /* Call the Application CB to give 'Received Data' event */
            if (hdl->callback_fn)
            {
                hdl->callback_fn(SYS_NET_EVNT_RCVD_DATA, hdl, hdl->cookie);
            }

            return;
        }
    }
        break;

        /* Received a FIN from the peer */
    case SYS_NET_STATUS_PEER_SENT_FIN:
    {
        if (NET_PRES_SocketReadIsReady(hdl->socket))
        {
            SYS_NET_GiveSemaphore(hdl);

            /* Call the Application CB to give 'Received Data' event */
            if (hdl->callback_fn)
            {
                hdl->callback_fn(SYS_NET_EVNT_RCVD_DATA, NULL, hdl->cookie);
            }

            return;
        }

        /* Close socket */
        NET_PRES_SocketClose(hdl->socket);

        /* Change the State */
        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DISCONNECTED);

        SYS_NET_GiveSemaphore(hdl);

        /* Call the Application CB to give 'Disconnected' event */
        if (hdl->callback_fn)
        {
            hdl->callback_fn(SYS_NET_EVNT_DISCONNECTED, hdl, hdl->cookie);
        }

        SYS_NET_TakeSemaphore(hdl);

        if (hdl->cfg_info.enable_reconnect)
        {
            /* In case the Mode is NET Server, Open Socket and Wait for Connection */
            hdl->socket = NET_PRES_SocketOpen(0,
                                              hdl->sock_type,
                                              TCPIP_DNS_TYPE_A,
                                              hdl->cfg_info.port,
                                              0,
                                              NULL);
            if (hdl->socket == INVALID_SOCKET)
            {
                SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "SYS_NET_TCPIP_ServerOpen failed!\r\n");
            }
            else
            {
                SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_SERVER_AWAITING_CONNECTION);
            }

            if (SYS_NET_Set_Sock_Option(hdl) == false)
            {
                SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Set Sock Option Failed\r\n");
            }

            /* Register the CB with NetPres */
            if (NET_PRES_SocketSignalHandlerRegister(hdl->socket, 0xffff, SYS_NET_NetPres_Signal, hdl) == NULL)
            {
                SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "Handler Registration failed!\r\n");
            }
        }

        SYS_NET_GiveSemaphore(hdl);

        return;
    }
        break;

#ifdef SYS_NET_TLS_ENABLED		
    case SYS_NET_STATUS_TLS_NEGOTIATION_FAILED:
        break;
#endif		
    }

    SYS_NET_GiveSemaphore(hdl);

    return;
}

void SYS_NET_Task(SYS_MODULE_OBJ obj)
{
    SYS_NET_Handle *hdl = (SYS_NET_Handle*) obj;

    if (obj != SYS_MODULE_OBJ_INVALID)
    {
        if (hdl->cfg_info.mode == SYS_NET_MODE_CLIENT)
        {
            /* Client Mode */
            SYS_NET_Client_Task(hdl);
        }
        else
        {
            /* Server Mode */
            SYS_NET_Server_Task(hdl);
        }
    }
}

int32_t SYS_NET_SendMsg(SYS_MODULE_OBJ obj, uint8_t *data, uint16_t len)
{
    SYS_NET_Handle *hdl = (SYS_NET_Handle*) obj;
    int16_t wMaxPut = 0;
    int16_t sentBytes = 0;

    if (obj == SYS_MODULE_OBJ_INVALID)
    {
        SYS_NETDEBUG_DBG_PRINT(g_NetAppDbgHdl, NET_DATA, "Invalid Handle\r\n");
        return SYS_NET_INVALID_HANDLE;
    }

    if (SYS_NET_TakeSemaphore(hdl) == 0)
    {
        return SYS_NET_SEM_OPERATION_FAILURE;
    }

    /* check if the service is UP  */
    if ((!NET_PRES_SocketIsConnected(hdl->socket)) || (!SYS_NET_Ll_Link_Status(hdl)))
    {
        SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_CFG, "NET Intf Not Connected\r\n");
        SYS_NET_GiveSemaphore(hdl);
        return SYS_NET_SERVICE_DOWN;
    }

    /* check if the socket is free */
    wMaxPut = NET_PRES_SocketWriteIsReady(hdl->socket, len, len);
    if (wMaxPut == 0)
    {
        SYS_NET_GiveSemaphore(hdl);
        SYS_NETDEBUG_ERR_PRINT(g_NetAppDbgHdl, NET_DATA, "TCP/IP Write NOT ready\r\n");
        return SYS_NET_PUT_NOT_READY;
    }

    /* send data */
    sentBytes = NET_PRES_SocketWrite(hdl->socket, data, len);

    NET_PRES_SocketFlush(hdl->socket);

    SYS_NET_GiveSemaphore(hdl);

    return sentBytes;
}

int32_t SYS_NET_CtrlMsg(SYS_MODULE_OBJ obj,
                        SYS_NET_CTRL_MSG msg_type,
                        void *buffer,
                        uint32_t length)
{
    SYS_NET_Handle *hdl = (SYS_NET_Handle*) obj;
    SYS_NET_RESULT ret_val = SYS_NET_FAILURE;

    if (obj == SYS_MODULE_OBJ_INVALID)
    {
        SYS_NETDEBUG_DBG_PRINT(g_NetAppDbgHdl, NET_CFG, "Invalid Handle\r\n");
        return SYS_NET_INVALID_HANDLE;
    }

    if (SYS_NET_TakeSemaphore(hdl) == 0)
    {
        return SYS_NET_SEM_OPERATION_FAILURE;
    }

    switch (msg_type)
    {
    case SYS_NET_CTRL_MSG_RECONNECT:
    {
        if (hdl->status != SYS_NET_STATUS_DISCONNECTED)
        {
            /* Close socket */
            NET_PRES_SocketClose(hdl->socket);

            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DISCONNECTED);
        }

        if (buffer != NULL)
        {
            SYS_NET_Config *cfg = (SYS_NET_Config *) buffer;

            memcpy(&hdl->cfg_info, cfg, sizeof (SYS_NET_Config));
        }

        /* Changing the status to lower layer down as 
         * DNS needs to be resolved */
        SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_LOWER_LAYER_DOWN);

        SYS_NET_GiveSemaphore(hdl);

        return SYS_NET_SUCCESS;
    }
        break;

    case SYS_NET_CTRL_MSG_DISCONNECT:
    {
        if (hdl->status != SYS_NET_STATUS_IDLE)
        {
            /* Close socket */
            NET_PRES_SocketClose(hdl->socket);

            /* Change the State */
            SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_DISCONNECTED);

            SYS_NET_GiveSemaphore(hdl);

            /* Call the Application CB to give 'Disconnected' event */
            if (hdl->callback_fn)
            {
                hdl->callback_fn(SYS_NET_EVNT_DISCONNECTED, NULL, hdl->cookie);
            }

            SYS_NET_TakeSemaphore(hdl);

            if (hdl->cfg_info.enable_reconnect)
            {
                /* Changing the status to lower layer down as 
                 * DNS needs to be resolved */
                SYS_NET_SetInstStatus(hdl, SYS_NET_STATUS_LOWER_LAYER_DOWN);
            }

            ret_val = SYS_NET_SUCCESS;
        }
    }
        break;
    }

    SYS_NET_GiveSemaphore(hdl);
    return ret_val;
}

void SYS_NET_Close(SYS_MODULE_OBJ obj)
{
    SYS_NET_Handle *hdl = (SYS_NET_Handle*) obj;

    if (obj == SYS_MODULE_OBJ_INVALID)
    {
        return;
    }

    /* Close socket */
    NET_PRES_SocketClose(hdl->socket);

    /* Delete Semaphore */
    OSAL_SEM_Delete(&hdl->InstSemaphore);

    /* Free the handle */
    SYS_NET_FreeHandle(hdl);
}

SYS_NET_STATUS SYS_NET_GetStatus(SYS_MODULE_OBJ obj)
{
    SYS_NET_Handle *hdl = (SYS_NET_Handle*) obj;

    if (obj == SYS_MODULE_OBJ_INVALID)
    {
        return SYS_NET_INVALID_HANDLE;
    }

    return hdl->status;
}

int32_t SYS_NET_RecvMsg(SYS_MODULE_OBJ obj, void *buffer, uint16_t length)
{
    SYS_NET_Handle *hdl = (SYS_NET_Handle*) obj;
    uint32_t len = 0;

    if (obj == SYS_MODULE_OBJ_INVALID)
    {
        return SYS_NET_INVALID_HANDLE;
    }

    if (SYS_NET_TakeSemaphore(hdl) == 0)
    {
        return SYS_NET_SEM_OPERATION_FAILURE;
    }

    /* check if the service is UP  */
    if ((!NET_PRES_SocketIsConnected(hdl->socket)) || (!SYS_NET_Ll_Link_Status(hdl)))
    {
        SYS_NET_GiveSemaphore(hdl);

        return SYS_NET_SERVICE_DOWN;
    }

    /* check if there is Data to be received  */
    len = NET_PRES_SocketReadIsReady(hdl->socket);
    if (len == 0)
    {
        SYS_NET_GiveSemaphore(hdl);

        return SYS_NET_GET_NOT_READY;
    }

    if (len > length)
    {
        len = length;
    }

    /* Get Data from the NET Stack  */
    len = NET_PRES_SocketRead(hdl->socket, buffer, len);

    SYS_NET_GiveSemaphore(hdl);

    return len;
}

int32_t SYS_NET_SetConfigParam(SYS_MODULE_OBJ obj,
                               uint32_t paramType,
                               void *data)
{
    SYS_NET_Handle *hdl = (SYS_NET_Handle*) obj;

    hdl->cfg_info.enable_reconnect = *((bool *) data);

    return 0;
}
