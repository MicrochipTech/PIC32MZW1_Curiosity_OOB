#include "../sys_net.h"
#include "tcpip/sntp.h"

typedef union {
    UDP_SOCKET_INFO     sUdpInfo;       /* UDP Socket Info Maintained by the TCP Stack */
    TCP_SOCKET_INFO     sTcpInfo;       /* TCP Socket Info Maintained by the TCP Stack */
}SYS_NET_SOCKET_INFO;

typedef struct {
    uint32_t    startTime;
    uint32_t    timeOut;
}SYS_NET_TimerInfo;

/* 
** The below structure is returned as a handle by the init call 
*/
typedef struct {
	SYS_NET_Config      	cfg_info;		/* config info received at the time of init */
    SYS_NET_SOCKET_INFO     sNetInfo;       /* Socket Info Maintained by the TCP Stack */
	SYS_NET_CALLBACK    	callback_fn;	/* callback registered with the service */
	void *					cookie;			/* Cookie to Identify the Instance by the User */
    OSAL_SEM_HANDLE_TYPE    InstSemaphore;	/* Semaphore for Critical Section */
	int16_t 				socket;			/* socket id received after opening the socket */    
	uint8_t					status;			/* Current state of the service */
	IP_MULTI_ADDRESS      	server_ip;		/* Server IP received after Resolving DNS */
    NET_PRES_SKT_T          sock_type;
    SYS_NET_TimerInfo      	timerInfo;
}SYS_NET_Handle;

static SYS_NET_Handle	g_asSysNetHandle[SYS_NET_MAX_NUM_OF_SOCKETS];
static OSAL_SEM_HANDLE_TYPE    g_SysNetSemaphore;	/* Semaphore for Critical Section */
uint32_t g_u32SysNetInitDone = 0;

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
    (status == SYS_NET_STATUS_DISCONNECTED)?"DISCONNECTED" : "Invalid Status"
#endif

#define SYS_NET_GET_MODE_STR(mode)  (mode == SYS_NET_MODE_CLIENT)?"CLIENT" : "SERVER"

#define SYS_NET_PERIOIDC_TIMEOUT   10 //Sec
#define SYS_NET_TIMEOUT_CONST (SYS_NET_PERIOIDC_TIMEOUT * SYS_TMR_TickCounterFrequencyGet())

void SYS_NET_StartTimer(SYS_NET_Handle *hdl, uint32_t timerInfo)
{
    hdl->timerInfo.startTime = SYS_TMR_TickCountGet();
    hdl->timerInfo.timeOut = timerInfo;
}

bool SYS_NET_TimerExpired(SYS_NET_Handle *hdl)
{
    if(hdl->timerInfo.startTime == 0)
        return false;
    
    return (SYS_TMR_TickCountGet() - hdl->timerInfo.startTime > hdl->timerInfo.timeOut);
}

void SYS_NET_ResetTimer(SYS_NET_Handle *hdl)
{
    hdl->timerInfo.startTime = 0;
}

static void softReset(void) {
    bool int_flag = false;
    /*disable interrupts since we are going to do a sysKey unlock*/
    int_flag = (bool) __builtin_disable_interrupts();

    /* unlock system for clock configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    if (int_flag) {
        __builtin_mtc0(12, 0, (__builtin_mfc0(12, 0) | 0x0001)); /* enable interrupts */
    }

     RSWRSTbits.SWRST = 1;
    /*This read is what actually causes the reset*/
	RSWRST=RSWRSTbits.SWRST;

    /*Reference code. We will not hit this due to reset. This is here for reference.*/
    int_flag = (bool) __builtin_disable_interrupts();

    SYSKEY = 0x33333333;

    if (int_flag) /* if interrupts originally were enabled, re-enable them */ {
        __builtin_mtc0(12, 0, (__builtin_mfc0(12, 0) | 0x0001));
    }

}

static void SysNet_Command_Process(int argc,char *argv[])
{
    if(g_u32SysNetInitDone == 0)
        SYS_CONSOLE_PRINT("\n\n\rNet Service Not Initialized");

    if((argc >= 2) && (!strcmp((char*)argv[1],"open")))
    {
        if(((argv[2] == NULL)) || (!strcmp("?",argv[2])))
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
            SYS_NET_Config  sCfg;
            SYS_NET_RESULT  ret_val = SYS_NET_FAILURE;
			int i = 0;

            memset(&sCfg, 0, sizeof(sCfg));
            
            /* Initializing the Default values for Optional Parameters */
            sCfg.enable_tls = SYS_NET_DEFAULT_TLS_ENABLE;
            sCfg.enable_reconnect = SYS_NET_DEFAULT_AUTO_RECONNECT;
                    
			sCfg.mode = strtoul( argv[2], 0, 10);
			if(sCfg.mode >  SYS_NET_MODE_SERVER)
			{
				SYS_CONSOLE_PRINT("\n\rInvalid <mode>");
				return;
			}

			sCfg.ip_prot = strtoul( argv[3], 0, 10);
			if(sCfg.ip_prot >  SYS_NET_IP_PROT_TCP)
			{
				SYS_CONSOLE_PRINT("\n\rInvalid <ip_prot>");
				return;
			}
            
            strcpy(sCfg.host_name, argv[4]);
            
            sCfg.port = strtoul( argv[5], 0, 10);

			for(i = 6; i < argc; i=i+2)
			{
	            if(argv[i] != NULL)
	            {
	                if(!strcmp((char*)argv[i],"auto_reconnect"))
	                {
	                    sCfg.enable_reconnect = strtoul( argv[i+1], 0, 10);
	                    if(sCfg.enable_reconnect >  1)
	                    {
	                        SYS_CONSOLE_PRINT("\n\rInvalid <auto_reconnect>");
	                        return;
	                    }
	                }
	                else if(!strcmp((char*)argv[i],"tls_enable"))
	                {
	                    sCfg.enable_tls = strtoul( argv[i+1], 0, 10);
	                    if(sCfg.enable_tls >  1)
	                    {
	                        SYS_CONSOLE_PRINT("\n\rInvalid <enable_tls>");
	                        return;
	                    }
	                }

	            }
			}

            ret_val = SYS_NET_CtrlMsg((SYS_MODULE_OBJ)(&g_asSysNetHandle[0]), 
                                                SYS_NET_CTRL_MSG_RECONNECT, 
                                                &sCfg, 
                                                sizeof(sCfg));
            if(ret_val == SYS_NET_FAILURE)
                SYS_CONSOLE_PRINT("\n\rFailed to Open");         

            SYS_CONSOLE_MESSAGE("\n\rDone");
			return;
		}
	}
    else if((argc >= 2) && (!strcmp((char*)argv[1],"send")))
    {
        /*Set the required config to ConfigData
          * NOTE: The content set here will be save on "save" command to NVM
          * Applied to MAC on "apply" command*/
        if(((argv[2] == NULL)) || (!strcmp("?",argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysnet send <net_service_instance> <message>");
			return;
        }
        else if (((argv[3] == NULL)) || (!strcmp("?",argv[3])))
		{
			SYS_CONSOLE_PRINT("\n\r\t* sysnet send <net_service_instance> <message>");
			SYS_CONSOLE_PRINT("\n\r<message>: is the message to be sent to the peer");
			return;
		}
        else
		{
			int temp = strtoul( argv[2], 0, 10);
			if(temp >=  SYS_NET_MAX_NUM_OF_SOCKETS)
			{
				SYS_CONSOLE_PRINT("\n\rInvalid <net_service_instance>");
				return;
			}
            
			SYS_NET_SendMsg((SYS_MODULE_OBJ)(&g_asSysNetHandle[temp]), 
                                (uint8_t *)argv[3], 
                                strlen(argv[3]));
            SYS_CONSOLE_MESSAGE("\n\rDone");
            return;
		}
	}
    if((argc >= 2) && (!strcmp((char*)argv[1],"close")))
    {
        if(((argv[2] == NULL)) || (!strcmp("?",argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysnet close <net_service_instance>");
            return;            
        }
        else
		{
			int temp = strtoul( argv[2], 0, 10);
			if(temp >=  SYS_NET_MAX_NUM_OF_SOCKETS)
			{
				SYS_CONSOLE_PRINT("\n\rInvalid <net_service_instance>");
				return;
			}
            
			SYS_NET_CtrlMsg((SYS_MODULE_OBJ)(&g_asSysNetHandle[temp]), 
                                SYS_NET_CTRL_MSG_DISCONNECT, 
                                NULL, 0);
            SYS_CONSOLE_MESSAGE("\n\rDone");
            return;
		}
    }
    else if((argc >= 2) && (!strcmp((char*)argv[1],"get")))
    {
        if((argv[2] == NULL) || (!strcmp("?",argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysnet get info");
            return;
        }        
        else if(!strcmp((char*)argv[2],"info"))
        {
            uint8_t i = 0;
            uint8_t k = 0;
			int nNets = 0;
			TCPIP_NET_HANDLE	netH;
			
			for (i = 0; i < SYS_NET_MAX_NUM_OF_SOCKETS; i++)
			{
				SYS_CONSOLE_PRINT("\n\n\r*****************************************");
				SYS_CONSOLE_PRINT("\n\rNET Service Instance: %d", i);
				SYS_CONSOLE_PRINT("\n\rStatus: %s", SYS_NET_GET_STATUS_STR(g_asSysNetHandle[i].status));
				SYS_CONSOLE_PRINT("\n\rMode: %s", SYS_NET_GET_MODE_STR(g_asSysNetHandle[i].cfg_info.mode));
				SYS_CONSOLE_PRINT("\n\rSocket ID: %d", g_asSysNetHandle[i].socket);
				SYS_CONSOLE_PRINT("\n\rHost: %s", g_asSysNetHandle[i].cfg_info.host_name);
                            
                if(g_asSysNetHandle[i].cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
                {
                    SYS_CONSOLE_PRINT("\n\rRemote IP: %d.%d.%d.%d", 
                            g_asSysNetHandle[i].sNetInfo.sUdpInfo.remoteIPaddress.v4Add.v[0],
                            g_asSysNetHandle[i].sNetInfo.sUdpInfo.remoteIPaddress.v4Add.v[1],
                            g_asSysNetHandle[i].sNetInfo.sUdpInfo.remoteIPaddress.v4Add.v[2],
                            g_asSysNetHandle[i].sNetInfo.sUdpInfo.remoteIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("\n\rRemote Port: %d", g_asSysNetHandle[i].sNetInfo.sUdpInfo.remotePort);
                    SYS_CONSOLE_PRINT("\n\rLocal Port: %d", g_asSysNetHandle[i].sNetInfo.sUdpInfo.localPort);
                    SYS_CONSOLE_PRINT("\n\rhNet: 0x%x", g_asSysNetHandle[i].sNetInfo.sUdpInfo.hNet);

                    /* Find the network interface index from the network handle */
                    nNets = TCPIP_STACK_NumberOfNetworksGet();
                    for(k = 0; k < nNets; k++)
                    {
                        netH = TCPIP_STACK_IndexToNet(k);
                        if(netH == g_asSysNetHandle[i].sNetInfo.sUdpInfo.hNet)
                            SYS_CONSOLE_PRINT("\n\rNet Intf Index: %x", k);
                    }
                }
                else if(g_asSysNetHandle[i].cfg_info.ip_prot == SYS_NET_IP_PROT_TCP)
                {                    
                    SYS_CONSOLE_PRINT("\n\rRemote IP: %d.%d.%d.%d", 
                            g_asSysNetHandle[i].sNetInfo.sTcpInfo.remoteIPaddress.v4Add.v[0],
                            g_asSysNetHandle[i].sNetInfo.sTcpInfo.remoteIPaddress.v4Add.v[1],
                            g_asSysNetHandle[i].sNetInfo.sTcpInfo.remoteIPaddress.v4Add.v[2],
                            g_asSysNetHandle[i].sNetInfo.sTcpInfo.remoteIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("\n\rRemote Port: %d", g_asSysNetHandle[i].sNetInfo.sTcpInfo.remotePort);
                    SYS_CONSOLE_PRINT("\n\rLocal Port: %d", g_asSysNetHandle[i].sNetInfo.sTcpInfo.localPort);
                     SYS_CONSOLE_PRINT("\n\rhNet: 0x%x", g_asSysNetHandle[i].sNetInfo.sTcpInfo.hNet);

                    /* Find the network interface index from the network handle */
                    nNets = TCPIP_STACK_NumberOfNetworksGet();
                    for(k = 0; k < nNets; k++)
                    {
                        netH = TCPIP_STACK_IndexToNet(k);
                        if(netH == g_asSysNetHandle[i].sNetInfo.sTcpInfo.hNet)
                            SYS_CONSOLE_PRINT("\n\rNet Intf Index: %x", k);
                    }
                }
			}
            return;
        }
    }    
    else if((argc >= 2) && (!strcmp((char*)argv[1],"reset")))
    {
        softReset();
    }
    else {
		SYS_CONSOLE_MESSAGE("*** Command Processor: unknown command. ***");
	}

	return;
}

static int SysNetCMDProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) \
{
	
	SysNet_Command_Process(argc,argv);

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

static const SYS_CMD_DESCRIPTOR    g_SysNetCmdTbl[]=
{
    {"sysnet",     (SYS_CMD_FNC)SysNetCMDProcessing,              ": SysNet commands processing"},
    {"sysnethelp",  (SYS_CMD_FNC)SysNetCMDHelp,			  		 ": SysNet commands help "},
};


int32_t SYS_NET_Initialize()
{
    memset(g_asSysNetHandle, 0, sizeof(g_asSysNetHandle));
    
	/* 
	** Create Semaphore for ensuring the SYS NET APIs are re-entrant 
	*/
    if(OSAL_SEM_Create(&g_SysNetSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1) != OSAL_RESULT_TRUE)
    {
        SYS_CONSOLE_MESSAGE("NET_SRVC: Failed to Initialize Service as Semaphore NOT created\r\n");
        return SYS_NET_FAILURE;        
    }
	
	/*
	** Add Sys NET Commands to System Command service
	*/
	if (!SYS_CMD_ADDGRP(g_SysNetCmdTbl, sizeof(g_SysNetCmdTbl)/sizeof(*g_SysNetCmdTbl), "sysnet", ": Sys NET commands"))
	{
        SYS_CONSOLE_MESSAGE("NET_SRVC: Failed to Initialize Service as SysNet Commands NOT created\r\n");
        return SYS_NET_FAILURE;        
	}
    
    g_u32SysNetInitDone = 1;
	return SYS_NET_SUCCESS;        
}

void SYS_NET_Deinitialize()
{
	/* 
	** Delete Semaphore 
	*/
    OSAL_SEM_Delete(&g_SysNetSemaphore);
}

static void* SYS_NET_AllocHandle()
{
	uint8_t i = 0;

	OSAL_SEM_Pend(&g_SysNetSemaphore, OSAL_WAIT_FOREVER);
	
	for(i = 0; i < SYS_NET_MAX_NUM_OF_SOCKETS; i++)
	{
		if(g_asSysNetHandle[i].status == SYS_NET_STATUS_IDLE)
		{
			OSAL_SEM_Post(&g_SysNetSemaphore);
			return &g_asSysNetHandle[i];
		}
	}
    OSAL_SEM_Post(&g_SysNetSemaphore);
    return NULL;
}

static void SYS_NET_FreeHandle(void *handle)
{
	SYS_NET_Handle	*hdl = (SYS_NET_Handle *)handle;

	OSAL_SEM_Pend(&g_SysNetSemaphore, OSAL_WAIT_FOREVER);
    hdl->status = SYS_NET_STATUS_IDLE;
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
    if(hdl->cfg_info.mode == SYS_NET_MODE_CLIENT)
        hdl->sock_type |= NET_PRES_SKT_CLIENT;
    else
        hdl->sock_type |= NET_PRES_SKT_SERVER;

    /* Check if the Socket is in Client or Server Mode */
    if(hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_TCP)
        hdl->sock_type |= NET_PRES_SKT_STREAM;
    else
        hdl->sock_type |= NET_PRES_SKT_DATAGRAM;
}

static bool SYS_NET_Ll_Status(SYS_NET_Handle *hdl)
{
    IPV4_ADDR NetIp;
#if defined(TCPIP_STACK_USE_IPV6)
	IPV6_ADDR_STRUCT currIpv6Add;
#endif
	TCPIP_NET_HANDLE hNet = TCPIP_STACK_IndexToNet(SYS_NET_DEFAULT_NET_INTF);

	if(TCPIP_STACK_NetIsUp(hNet) == false)
		return false;
	
	NetIp.Val = 0;
	NetIp.Val = TCPIP_STACK_NetAddress(hNet);
	if(NetIp.Val == 0)
	{
		
#if defined(TCPIP_STACK_USE_IPV6)
		if (TCPIP_STACK_NetIPv6AddressGet(hNet, IPV6_ADDR_TYPE_UNICAST, &currIpv6Add, NULL))
		{
			return true;
		}
#endif		
		return false;
	}
	return true;
}

TCPIP_DNS_RESULT SYS_NET_DNS_Resolve(SYS_NET_Handle *hdl)
{
	TCPIP_DNS_RESULT result = TCPIP_DNS_Resolve(hdl->cfg_info.host_name, TCPIP_DNS_TYPE_A);
	if (result == TCPIP_DNS_RES_NAME_IS_IPADDRESS)
	{
        if((TCPIP_Helper_StringToIPAddress(hdl->cfg_info.host_name, &hdl->server_ip.v4Add)) ||
			(TCPIP_Helper_StringToIPv6Address(hdl->cfg_info.host_name, &hdl->server_ip.v6Add)))
        {
			/*
			** In case the Host Name is the IP Address itself
			*/
			hdl->status = SYS_NET_STATUS_DNS_RESOLVED;
			return (SYS_MODULE_OBJ)hdl;
        }
            
		/*
		** In case the Host Name is the IP Address itself; This should never come
		*/
		hdl->status = SYS_NET_STATUS_DNS_RESOLVE_FAILED;
		return result;
	}
	if (result < 0)
	{
		
		if(hdl->cfg_info.enable_reconnect == 0)
		{
			/* 
			** Free Handle since the DNS cannot be resolved
			*/		
			SYS_CONSOLE_PRINT("NET_SRVC: Could Not Resolve DNS = %d (TCPIP_DNS_RESULT)\r\n", result);
				
			hdl->status = SYS_NET_STATUS_DNS_RESOLVE_FAILED;			
		}
		return result;
	}

	hdl->status = SYS_NET_STATUS_RESOLVING_DNS;
	return result;	
} 

SYS_MODULE_OBJ SYS_NET_Open(SYS_NET_Config *cfg, SYS_NET_CALLBACK net_cb, void *cookie)
{
    SYS_NET_Handle  	*hdl = NULL;    	
    
	/* 
	** Allocate Handle - hdl 
	*/
	hdl = SYS_NET_AllocHandle();
    if(hdl == NULL)
    {
        SYS_CONSOLE_MESSAGE("NET_SRVC: Failed to allocate Handle\r\n");
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
        SYS_CONSOLE_MESSAGE("NET_SRVC: Failed to create Semaphore\r\n");
        SYS_NET_FreeHandle(hdl);
        return SYS_MODULE_OBJ_INVALID;        
    }
	
	/* 
	** Copy the config info and the fn ptr into the Handle 
	*/
	if(cfg == NULL)
	{
		memcpy(&hdl->cfg_info, &g_sSysNetConfig, sizeof(SYS_NET_Config));
	}
	else
	{
		memcpy(&hdl->cfg_info, cfg, sizeof(SYS_NET_Config));
	}
    hdl->cookie = cookie;
    hdl->callback_fn = net_cb;

    SYS_NET_SetSockType(hdl);

	/* 
	** Check if the NET IP Stack is UP 
	*/
	if(SYS_NET_Ll_Status(hdl) == false)
	{
		SYS_CONSOLE_MESSAGE("NET_SRVC: TCPIP Stack Not UP\r\n");
		hdl->status = SYS_NET_STATUS_LOWER_LAYER_DOWN;
		return (SYS_MODULE_OBJ)hdl; 				
	}

	
    /* 
	** Open the Socket based on the Mode 
	*/
	if (hdl->cfg_info.mode == SYS_NET_MODE_CLIENT)
	{
		SYS_NET_DNS_Resolve(hdl);
		return (SYS_MODULE_OBJ)hdl;
	}
	
	/*
	** In case the Mode is NET Server, Open Socket and Wait for Connection
	*/
    hdl->socket = NET_PRES_SocketOpen(0,
            hdl->sock_type,
            TCPIP_DNS_TYPE_A,
            hdl->cfg_info.port,
            0,
            NULL);
	if (hdl->socket == INVALID_SOCKET)
	{
		/* 
		** Free Handle since failed to open a server socket
		*/
        SYS_CONSOLE_MESSAGE(" NET_SRVC: ServerOpen failed!\r\n");		

		hdl->status = SYS_NET_STATUS_SOCK_OPEN_FAILED;			
		return (SYS_MODULE_OBJ)hdl;
	}

	/*
	** Wait for Connection
	*/
	hdl->status = SYS_NET_STATUS_SERVER_AWAITING_CONNECTION;
	return (SYS_MODULE_OBJ)hdl;	
}

static void SYS_NET_Client_Task(SYS_NET_Handle *hdl)
{
	if (SYS_NET_TakeSemaphore(hdl) == 0)
		return;
    
	switch(hdl->status)
	{
		case SYS_NET_STATUS_LOWER_LAYER_DOWN:
		{			
			if(SYS_NET_Ll_Status(hdl) == false)
			{
				SYS_NET_GiveSemaphore(hdl);
				return;
			}
			SYS_NET_DNS_Resolve(hdl);
		}
		break;
		
		case SYS_NET_STATUS_RESOLVING_DNS:
		{
			TCPIP_DNS_RESULT result = 
					TCPIP_DNS_IsResolved(hdl->cfg_info.host_name, 
									&hdl->server_ip, 
									TCPIP_DNS_TYPE_A);
			switch (result)
			{
				case TCPIP_DNS_RES_OK:
				{					
					hdl->status = SYS_NET_STATUS_DNS_RESOLVED;
				}
				break;
				
				case TCPIP_DNS_RES_PENDING:
					break;
	
				case TCPIP_DNS_RES_SERVER_TMO:
				{					
					if(hdl->cfg_info.enable_reconnect == 0)
					{
                        SYS_CONSOLE_PRINT("\n\rNET_SRVC (%d): Could Not Resolve DNS", __LINE__);
						hdl->status = SYS_NET_STATUS_DNS_RESOLVE_FAILED;
						break;
					}
					result = SYS_NET_DNS_Resolve(hdl);
				}
				break;
				
				default:
				{
                    // SYS_CONSOLE_PRINT("\n\rNET_SRVC (%d): Could Not Resolve DNS = %d (TCPIP_DNS_RESULT) ", __LINE__, result);
					hdl->status = SYS_NET_STATUS_DNS_RESOLVE_FAILED;
					// SYS_CONSOLE_MESSAGE("\n\rNET_SRVC: TCPIP_DNS_IsResolved returned failure");
				}
			}
		}
		break;

		case SYS_NET_STATUS_DNS_RESOLVED:
		{
			// We now have an IPv4 Address
			// Open a socket
            hdl->socket = NET_PRES_SocketOpen(0,
                    hdl->sock_type,
                    TCPIP_DNS_TYPE_A,
                    hdl->cfg_info.port,
                    (NET_PRES_ADDRESS *) &hdl->server_ip,
                    NULL);
			if (hdl->socket == INVALID_SOCKET)
			{
				SYS_CONSOLE_MESSAGE("\n\rNET_SRVC: ClientOpen failed!");

				if(hdl->cfg_info.enable_reconnect == 0)
				{
                    SYS_CONSOLE_PRINT("\n\rNET_SRVC: Could Not Open Socket(%d)", __LINE__);
					hdl->status = SYS_NET_STATUS_SOCK_OPEN_FAILED;					
				}
				SYS_NET_GiveSemaphore(hdl);
				return ;
			}

            NET_PRES_SocketWasReset(hdl->socket);
            if (hdl->socket == INVALID_SOCKET) {
                SYS_CONSOLE_MESSAGE("Could not create socket - aborting\r\n");
                return;
            }
			hdl->status = SYS_NET_STATUS_CLIENT_CONNECTING;
		}
		break;
		
		case SYS_NET_STATUS_CLIENT_CONNECTING:
		{
			if (!NET_PRES_SocketIsConnected(hdl->socket))
			{
				break;
			}
			
#ifdef SYS_NET_TLS_ENABLED
            if(hdl->cfg_info.enable_tls)
            {
                hdl->status = SYS_NET_STATUS_WAIT_FOR_SNTP;
                break;
            }
#endif            
            if(hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
			{
                NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sUdpInfo);
                memcpy(&hdl->server_ip, &hdl->sNetInfo.sUdpInfo.remoteIPaddress, sizeof(hdl->server_ip));
			}
            else
			{
                NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sTcpInfo);
                memcpy(&hdl->server_ip, &hdl->sNetInfo.sTcpInfo.remoteIPaddress, sizeof(hdl->server_ip));
            }

			hdl->status = SYS_NET_STATUS_CONNECTED;			
			SYS_NET_GiveSemaphore(hdl);
			if (hdl->callback_fn)
			{
				hdl->callback_fn(SYS_NET_EVNT_CONNECTED, NULL, hdl->cookie);
			}

			return;
		}
		break;
		
#ifdef SYS_NET_TLS_ENABLED
        case SYS_NET_STATUS_WAIT_FOR_SNTP:
        {
            uint32_t time = TCPIP_SNTP_UTCSecondsGet();
            if(time == 0) //SNTP Time Stamp NOT Available
                break;
            
            if(NET_PRES_SocketEncryptSocket(hdl->socket) != true) {
                SYS_CONSOLE_PRINT("\n\rNET_SRVC (%d): Negotiation Failed; Aborting", __LINE__);		
                hdl->status = SYS_NET_STATUS_TLS_NEGOTIATION_FAILED;
                break;
            }
			SYS_NET_StartTimer(hdl, SYS_NET_TIMEOUT_CONST);
            hdl->status = SYS_NET_STATUS_TLS_NEGOTIATING;            
        }
        break;
        
        case SYS_NET_STATUS_TLS_NEGOTIATING:
        {
			if(SYS_NET_TimerExpired(hdl) == true)
			{
				SYS_NET_ResetTimer(hdl);
				hdl->status = SYS_NET_STATUS_TLS_NEGOTIATION_FAILED;
                break;
			}

			if (NET_PRES_SocketIsNegotiatingEncryption(hdl->socket)) {
                break;
            }
            if (!NET_PRES_SocketIsSecure(hdl->socket)) {
                SYS_CONSOLE_MESSAGE("SSL Connection Negotiation Failed - Aborting\r\n");
				hdl->status = SYS_NET_STATUS_TLS_NEGOTIATION_FAILED;
                break;
            }                            
            
            if(hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
                NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sUdpInfo);
            else
                NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sTcpInfo);

			SYS_NET_ResetTimer(hdl);
			hdl->status = SYS_NET_STATUS_CONNECTED;			
			SYS_NET_GiveSemaphore(hdl);
			if (hdl->callback_fn)
			{
				hdl->callback_fn(SYS_NET_EVNT_CONNECTED, NULL, hdl->cookie);
			}
			return ;
        }
        break;
#endif
        
		case SYS_NET_STATUS_CONNECTED:
		{
			if (!NET_PRES_SocketIsConnected(hdl->socket))
			{
                /* Close socket */
                NET_PRES_SocketClose(hdl->socket);
				hdl->status = SYS_NET_STATUS_DISCONNECTED; 
                SYS_NET_GiveSemaphore(hdl);
                
                if (hdl->callback_fn)
                {
                    hdl->callback_fn(SYS_NET_EVNT_DISCONNECTED, NULL, hdl->cookie);
                }				

                SYS_NET_TakeSemaphore(hdl);

                if(hdl->cfg_info.enable_reconnect)
                {
                    hdl->socket = NET_PRES_SocketOpen(0, 
                                        hdl->sock_type, 
                                        TCPIP_DNS_TYPE_A, 
                                        hdl->cfg_info.port, 
                                        (NET_PRES_ADDRESS*)&hdl->server_ip,
                                        NULL);
                    if (hdl->socket == INVALID_SOCKET)
                    {
                        SYS_CONSOLE_MESSAGE(" NET_SRVC: ClientOpen failed!\r\n");		
                    }
                    else
                        hdl->status = SYS_NET_STATUS_CLIENT_CONNECTING;
                }
				
			break;
			}

        	if (NET_PRES_SocketReadIsReady(hdl->socket))
			{
				SYS_NET_GiveSemaphore(hdl);
				if (hdl->callback_fn)
				{
					hdl->callback_fn(SYS_NET_EVNT_RCVD_DATA, NULL, hdl->cookie);
				}
				return;
			}
		}
		break;

#ifdef SYS_NET_TLS_ENABLED		
		case SYS_NET_STATUS_TLS_NEGOTIATION_FAILED:
        {
                /* Close socket */
            NET_PRES_SocketClose(hdl->socket);
            hdl->status = SYS_NET_STATUS_DISCONNECTED;

			if (hdl->callback_fn)
			{
				hdl->callback_fn(SYS_NET_EVNT_SSL_FAILED, NULL, hdl->cookie);
			}            
            
            if(hdl->cfg_info.enable_reconnect)
                hdl->status = SYS_NET_STATUS_LOWER_LAYER_DOWN;
        }
        break;
#endif
        
		case SYS_NET_STATUS_DNS_RESOLVE_FAILED:
        {
            hdl->status = SYS_NET_STATUS_DISCONNECTED;

			if (hdl->callback_fn)
			{
				hdl->callback_fn(SYS_NET_EVNT_DNS_RESOLVE_FAILED, NULL, hdl->cookie);
			}            

            if(hdl->cfg_info.enable_reconnect)
                hdl->status = SYS_NET_STATUS_LOWER_LAYER_DOWN;
        }
        break;

		case SYS_NET_STATUS_SOCK_OPEN_FAILED:
        {
            hdl->status = SYS_NET_STATUS_DISCONNECTED;
			if (hdl->callback_fn)
			{
				hdl->callback_fn(SYS_NET_EVNT_SOCK_OPEN_FAILED, NULL, hdl->cookie);
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
		return;
    
	switch(hdl->status)
	{
		case SYS_NET_STATUS_LOWER_LAYER_DOWN:
		{			
			if(SYS_NET_Ll_Status(hdl) == false)
			{
				SYS_NET_GiveSemaphore(hdl);
				return;
			}

			/*
			** In case the Mode is NET Server, Open Socket and Wait for Connection
			*/
			hdl->socket = NET_PRES_SocketOpen(0,
					hdl->sock_type,
					TCPIP_DNS_TYPE_A,
					hdl->cfg_info.port,
					0,
					NULL);
			if (hdl->socket == INVALID_SOCKET)
			{
				/* 
				** Free Handle since failed to open a server socket
				*/
			
				SYS_CONSOLE_MESSAGE(" NET_SRVC: ServerOpen failed!\r\n"); 	

				hdl->status = SYS_NET_STATUS_SOCK_OPEN_FAILED;			
				SYS_NET_GiveSemaphore(hdl);
				return;
			}
			
			/*
			** Wait for Connection
			*/
			hdl->status = SYS_NET_STATUS_SERVER_AWAITING_CONNECTION;
            SYS_NET_GiveSemaphore(hdl);
			return;
		}
		break;
		
		case SYS_NET_STATUS_SERVER_AWAITING_CONNECTION:
		{
            if (!NET_PRES_SocketIsConnected(hdl->socket))
            {
				SYS_NET_GiveSemaphore(hdl);
                return;
            }
            else
            {
#ifdef SYS_NET_TLS_ENABLED				
				if(hdl->cfg_info.enable_tls)
				{
                    hdl->status = SYS_NET_STATUS_WAIT_FOR_SNTP;
                    break;
				}
#endif
				if(hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
					NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sUdpInfo);
				else
					NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sTcpInfo);
				
                // We got a connection
                hdl->status = SYS_NET_STATUS_CONNECTED;
                SYS_CONSOLE_MESSAGE("Received a connection\r\n");				
				SYS_NET_GiveSemaphore(hdl);
                if (hdl->callback_fn)
                {
                    hdl->callback_fn(SYS_NET_EVNT_CONNECTED, hdl, hdl->cookie);
                }				
				return;
            }
		}
		break;

#ifdef SYS_NET_TLS_ENABLED		
        case SYS_NET_STATUS_WAIT_FOR_SNTP:
        {
            uint32_t time = TCPIP_SNTP_UTCSecondsGet();
            if(time == 0) //SNTP Time Stamp NOT Available
                break;
            
            if(NET_PRES_SocketEncryptSocket(hdl->socket) != true) {
                SYS_CONSOLE_PRINT("\n\rNET_SRVC (%d): Negotiation Failed; Aborting", __LINE__);		
                hdl->status = SYS_NET_STATUS_TLS_NEGOTIATION_FAILED;
                break;
            }
            hdl->status = SYS_NET_STATUS_TLS_NEGOTIATING;
            break;
        }
        break;
        
		case SYS_NET_STATUS_TLS_NEGOTIATING:
		{
			if (NET_PRES_SocketIsNegotiatingEncryption(hdl->socket)) {
				SYS_CONSOLE_MESSAGE("SSL Connection Negotiation - In-Progress\r\n");
				break;
			}
			if (!NET_PRES_SocketIsSecure(hdl->socket)) {
				SYS_CONSOLE_MESSAGE("SSL Connection Negotiation Failed - Aborting\r\n");
				hdl->status = SYS_NET_STATUS_TLS_NEGOTIATION_FAILED;
				break;
			}							 

			if(hdl->cfg_info.ip_prot == SYS_NET_IP_PROT_UDP)
				NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sUdpInfo);
			else
				NET_PRES_SocketInfoGet(hdl->socket, &hdl->sNetInfo.sTcpInfo);

			hdl->status = SYS_NET_STATUS_CONNECTED; 		
			SYS_NET_GiveSemaphore(hdl);
			if (hdl->callback_fn)
			{
				hdl->callback_fn(SYS_NET_EVNT_CONNECTED, hdl, hdl->cookie);
			}
			return ;
		}
		break;
#endif				
		
		case SYS_NET_STATUS_CONNECTED:
		{
            if (!NET_PRES_SocketIsConnected(hdl->socket))
            {
                /* Close socket */
                NET_PRES_SocketClose(hdl->socket);

				/* Change the State */
                hdl->status = SYS_NET_STATUS_DISCONNECTED;
                
				SYS_NET_GiveSemaphore(hdl);
                
                if (hdl->callback_fn)
                {
                    hdl->callback_fn(SYS_NET_EVNT_DISCONNECTED, hdl, hdl->cookie);
                }				

                SYS_NET_TakeSemaphore(hdl);
                if(hdl->cfg_info.enable_reconnect)
                {
                    /*
                    ** In case the Mode is NET Server, Open Socket and Wait for Connection
                    */
                    hdl->socket = NET_PRES_SocketOpen(0,
                            hdl->sock_type,
                            TCPIP_DNS_TYPE_A,
                            hdl->cfg_info.port,
                            0,
                            NULL);
                    if (hdl->socket == INVALID_SOCKET)
                    {
                        SYS_CONSOLE_MESSAGE(" NET_SRVC: SYS_NET_TCPIP_ServerOpen failed!\r\n");		
                    }
                    else
                        hdl->status = SYS_NET_STATUS_SERVER_AWAITING_CONNECTION;
                }
                SYS_NET_GiveSemaphore(hdl);
                return;
            }
			
        	if (NET_PRES_SocketReadIsReady(hdl->socket))
			{
				SYS_NET_GiveSemaphore(hdl);
				if (hdl->callback_fn)
				{
					hdl->callback_fn(SYS_NET_EVNT_RCVD_DATA, hdl, hdl->cookie);
				}
				return;
			}
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
	SYS_NET_Handle *hdl = (SYS_NET_Handle*)obj;
    
    if(obj != SYS_MODULE_OBJ_INVALID)
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
	SYS_NET_Handle *hdl = (SYS_NET_Handle*)obj;
	int16_t wMaxPut = 0;
    int16_t sentBytes = 0;
    
    if(obj == SYS_MODULE_OBJ_INVALID)
	    return SYS_NET_INVALID_HANDLE;

    if (SYS_NET_TakeSemaphore(hdl) == 0)
		return SYS_NET_SEM_OPERATION_FAILURE;
    
	/* check if the service is UP  */
    if (!NET_PRES_SocketIsConnected(hdl->socket))
    {
        SYS_CONSOLE_PRINT("NET_SRVC: NET Intf Not Connected\r\n");
        SYS_NET_GiveSemaphore(hdl);
        return SYS_NET_SERVICE_DOWN;    
	}
	
	/* check if the socket is free */
    wMaxPut = NET_PRES_SocketWriteIsReady(hdl->socket, len, len);
    if (wMaxPut == 0)
    {
        SYS_NET_GiveSemaphore(hdl);
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
	SYS_NET_Handle *hdl = (SYS_NET_Handle*)obj;
    SYS_NET_RESULT  ret_val = SYS_NET_FAILURE;
    
    if(obj == SYS_MODULE_OBJ_INVALID)
	    return SYS_NET_INVALID_HANDLE;

    if (SYS_NET_TakeSemaphore(hdl) == 0)
		return SYS_NET_SEM_OPERATION_FAILURE;

    switch(msg_type)
    {
        case SYS_NET_CTRL_MSG_RECONNECT:
        {
            if(hdl->status != SYS_NET_STATUS_DISCONNECTED)
            {
                /* Close socket */
                NET_PRES_SocketClose(hdl->socket);
				hdl->status = SYS_NET_STATUS_DISCONNECTED;
            }
            
            if(buffer != NULL)
            {
                SYS_NET_Config  *cfg = (SYS_NET_Config  *)buffer;
                memcpy(&hdl->cfg_info, cfg, sizeof(SYS_NET_Config));
            }
                
            /* Changing the status to lower layer down as 
             * DNS needs to be resolved */
            hdl->status = SYS_NET_STATUS_LOWER_LAYER_DOWN;
            SYS_NET_GiveSemaphore(hdl);
            return SYS_NET_SUCCESS;            
        }
        break;
        
        case SYS_NET_CTRL_MSG_DISCONNECT:
        {
            if(hdl->status != SYS_NET_STATUS_IDLE)
            {
                /* Close socket */
                NET_PRES_SocketClose(hdl->socket);
				/* Change the State */
                hdl->status = SYS_NET_STATUS_DISCONNECTED;
                
				SYS_NET_GiveSemaphore(hdl);
                
                if (hdl->callback_fn)
                {
                    hdl->callback_fn(SYS_NET_EVNT_DISCONNECTED, NULL, hdl->cookie);
                }				

                SYS_NET_TakeSemaphore(hdl);

                if(hdl->cfg_info.enable_reconnect)
                {
					/* Changing the status to lower layer down as 
					 * DNS needs to be resolved */
					hdl->status = SYS_NET_STATUS_LOWER_LAYER_DOWN;
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
	SYS_NET_Handle *hdl = (SYS_NET_Handle*)obj;
	
    if(obj == SYS_MODULE_OBJ_INVALID)
	    return ;
	
    /* Close socket */
    NET_PRES_SocketClose(hdl->socket);

    /* Delete Semaphore */
    OSAL_SEM_Delete(&hdl->InstSemaphore);
    
	/* Free the handle */
    SYS_NET_FreeHandle(hdl);
}

SYS_NET_STATUS SYS_NET_GetStatus(SYS_MODULE_OBJ obj)
{
	SYS_NET_Handle *hdl = (SYS_NET_Handle*)obj;
    if(obj == SYS_MODULE_OBJ_INVALID)
	    return SYS_NET_INVALID_HANDLE;
	
	return hdl->status;
}

int32_t SYS_NET_RecvMsg(SYS_MODULE_OBJ obj, void *buffer, uint16_t length)
{
	SYS_NET_Handle *hdl = (SYS_NET_Handle*)obj;
    uint32_t        len = 0;
    
	if(obj == SYS_MODULE_OBJ_INVALID)
	    return SYS_NET_INVALID_HANDLE;
	
    if (SYS_NET_TakeSemaphore(hdl) == 0)
		return SYS_NET_SEM_OPERATION_FAILURE;
    
	/* check if the service is UP  */
    if (!NET_PRES_SocketIsConnected(hdl->socket))
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

    if(len > length)
    {
        len = length;
    }
    
	/* Get Data from the NET Stack  */
	len = NET_PRES_SocketRead(hdl->socket, buffer, len);
    SYS_NET_GiveSemaphore(hdl);
    return len;
}

int32_t SYS_NET_SetConfigParam(SYS_MODULE_OBJ obj, 
        uint32_t    paramType,
        void *data)
{
	SYS_NET_Handle *hdl = (SYS_NET_Handle*)obj;
    hdl->cfg_info.enable_reconnect = *((bool *)data);
    return 0;
}
