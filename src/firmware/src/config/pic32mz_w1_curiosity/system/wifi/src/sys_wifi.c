/*******************************************************************************
  Wi-Fi System Service Implementation

  File Name:
    sys_wifi.c

  Summary:
    Source code for the Wi-Fi system service implementation.

  Description:
    This file contains the source code for the Wi-Fi system service
    implementation.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020-2021 released Microchip Technology Inc.  All rights reserved.

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
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdlib.h>
#include "definitions.h"
#include "wdrv_pic32mzw_client_api.h"
#include "tcpip_manager_control.h"
#include "system/wifi/sys_wifi.h"
#include "configuration.h"


// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

typedef struct 
{
    /* The WiFi service current status */
    SYS_WIFI_STATUS wifiSrvcStatus;
    
    /* Wi-Fi Service handle */
    DRV_HANDLE wifiSrvcDrvHdl;
    
    /* Wi-Fi Service Auth Context */
    WDRV_PIC32MZW_AUTH_CONTEXT wifiSrvcAuthCtx;
    
    /* Wi-Fi Service BSS Context */
    WDRV_PIC32MZW_BSS_CONTEXT wifiSrvcBssCtx;

} SYS_WIFI_OBJ; /* Wi-Fi System Service Object */

/* Wi-Fi STA Mode, maximum auto connect retry */
#define MAX_AUTO_CONNECT_RETRY                5
// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
/* Storing Wi-Fi Service Callbacks */
static    SYS_WIFI_CALLBACK     g_wifiSrvcCallBack[SYS_WIFI_MAX_CBS];

/* Wi-Fi Service Object */
static    SYS_WIFI_OBJ          g_wifiSrvcObj = {SYS_WIFI_STATUS_NONE,0};

/* Wi-Fi Driver ASSOC Handle */
static WDRV_PIC32MZW_ASSOC_HANDLE g_wifiSrvcDrvAssocHdl = WDRV_PIC32MZW_ASSOC_HANDLE_INVALID;

/* Wi-Fi DHCP handler */
static    TCPIP_DHCP_HANDLE     g_wifiSrvcDhcpHdl = NULL;

/* Wi-Fi STA Mode, Auto connect retry count */
static    uint32_t              g_wifiSrvcAutoConnectRetry = 0;


/* Wi-Fi  Service Configuration Structure */
static    SYS_WIFI_CONFIG       g_wifiSrvcConfig;

/* Wi-Fi Service cookie */
static    void *                g_wifiSrvcCookie ;

/* Wi-Fi Service init enable wait status */
static    bool                  g_wifiSrvcInit = false;


/* RegDomain set status */
bool      g_isRegDomainSetReq = false;

/* Semaphore for Critical Section */
static    OSAL_SEM_HANDLE_TYPE  g_wifiSrvcSemaphore;
// *****************************************************************************

// *****************************************************************************

static uint8_t SYS_WIFI_DisConnect(void);
static SYS_WIFI_RESULT SYS_WIFI_ConnectReq(void);




// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************


static inline void SYS_WIFI_CallBackFun
(
    uint32_t event, 
    void * data, void *cookie
) 
{
    uint8_t idx;
    
    for (idx = 0; idx < SYS_WIFI_MAX_CBS; idx++) 
    {
        if (g_wifiSrvcCallBack[idx]) 
        {  
            /* Call client register functions */
            (g_wifiSrvcCallBack[idx])(event, data, cookie);
        }
    }
}

static inline SYS_WIFI_RESULT SYS_WIFI_REGCB
(
    SYS_WIFI_CALLBACK callback
) 
{
    SYS_WIFI_RESULT ret = SYS_WIFI_FAILURE;
    uint8_t idx;
    
    for (idx = 0; idx < SYS_WIFI_MAX_CBS; idx++) 
    {
        if (!g_wifiSrvcCallBack[idx]) 
        {
            /* Copy the client function pointer */
            g_wifiSrvcCallBack[idx] = callback;
            ret = SYS_WIFI_SUCCESS;
            break;
        }
    }
    return ret;
}

static inline void SYS_WIFI_InitConfig
(
    SYS_WIFI_CONFIG *config
) 
{   
    
    if (!config) 
    {   
        /* User has not set configuration during Wi-Fi Service initialization,
           Copy the MHC configuration into Wi-Fi Service structure */
        g_wifiSrvcConfig.mode = SYS_WIFI_DEVMODE;
        g_wifiSrvcConfig.saveConfig = 0;
        memcpy(g_wifiSrvcConfig.countryCode, SYS_WIFI_COUNTRYCODE, strlen(SYS_WIFI_COUNTRYCODE));

        /* STA Mode Configuration */
        g_wifiSrvcConfig.staConfig.channel = 0;        
        g_wifiSrvcConfig.staConfig.autoConnect = SYS_WIFI_STA_AUTOCONNECT;
        g_wifiSrvcConfig.staConfig.authType = SYS_WIFI_STA_AUTHTYPE;
        memcpy(g_wifiSrvcConfig.staConfig.ssid, SYS_WIFI_STA_SSID, sizeof(SYS_WIFI_STA_SSID));        
        memcpy(g_wifiSrvcConfig.staConfig.psk, SYS_WIFI_STA_PWD, sizeof(SYS_WIFI_STA_PWD));
    } 
    else 
    {
        /* User has set configuration during Wi-Fi Service initialization,
           Copy the user configuration into Wi-Fi Service structure  */
        memcpy(&g_wifiSrvcConfig, config, sizeof(SYS_WIFI_CONFIG));
    }
}
 
static inline void SYS_WIFI_SetCookie
(
    void *cookie
) 
{
    g_wifiSrvcCookie = cookie;
}

static inline SYS_WIFI_MODE SYS_WIFI_GetMode(void)
{
    return g_wifiSrvcConfig.mode;
}

static inline bool SYS_WIFI_GetSaveConfig(void)
{
    return g_wifiSrvcConfig.saveConfig;
}

static inline uint8_t * SYS_WIFI_GetSSID(void)  
{
    return g_wifiSrvcConfig.staConfig.ssid;
}

static inline uint8_t SYS_WIFI_GetSSIDLen(void)  
{
    return strlen((const char *)g_wifiSrvcConfig.staConfig.ssid);
}

static inline uint8_t SYS_WIFI_GetChannel(void)
{
    return (g_wifiSrvcConfig.staConfig.channel);
}

static inline uint8_t SYS_WIFI_GetAuthType(void)
{
    return g_wifiSrvcConfig.staConfig.authType;
}

static inline uint8_t *SYS_WIFI_GetPsk(void)
{
    return g_wifiSrvcConfig.staConfig.psk;
}

static inline uint8_t SYS_WIFI_GetPskLen(void)
{
    return strlen((const char *)g_wifiSrvcConfig.staConfig.psk);
}

static inline bool SYS_WIFI_GetSSIDVisibility(void)
{
    return 0;
}

static inline const char *SYS_WIFI_GetCountryCode(void)
{
    return (const char *) g_wifiSrvcConfig.countryCode;
}

static inline bool SYS_WIFI_GetAutoConnect(void)
{
    return g_wifiSrvcConfig.staConfig.autoConnect;        
}

static inline void SYS_WIFI_SetTaskstatus
(
    SYS_WIFI_STATUS status
)
{
    g_wifiSrvcObj.wifiSrvcStatus = status;
}

static inline SYS_WIFI_STATUS SYS_WIFI_GetTaskstatus(void)
{
    return g_wifiSrvcObj.wifiSrvcStatus;
}

static inline void SYS_WIFI_PrintWifiConfig(void)
{
    SYS_CONSOLE_MESSAGE("Wi-Fi Configuration:\r\n");
    SYS_CONSOLE_PRINT("\r\n mode=%d (0-STA,1-AP) saveConfig=%d \r\n ", g_wifiSrvcConfig.mode, g_wifiSrvcConfig.saveConfig);
    if (g_wifiSrvcConfig.mode == SYS_WIFI_STA) 
    {
        SYS_CONSOLE_PRINT("\r\n STA Configuration :\r\n channel=%d \r\n autoConnect=%d \r\n ssid=%s \r\n passphase=%s \r\n authentication type=%d (1-Open,2-WEP,3-Mixed mode(WPA/WPA2),4-WPA2,5-Mixed mode(WPA2/WPA3),6-WPA3)\r\n", g_wifiSrvcConfig.staConfig.channel, g_wifiSrvcConfig.staConfig.autoConnect, g_wifiSrvcConfig.staConfig.ssid, g_wifiSrvcConfig.staConfig.psk, g_wifiSrvcConfig.staConfig.authType);
    }

}
static void SYS_WIFI_STAConnCallBack
(
    DRV_HANDLE handle, 
    WDRV_PIC32MZW_ASSOC_HANDLE assocHandle,
    WDRV_PIC32MZW_CONN_STATE currentState
) 
{
    switch (currentState) 
    {
        case WDRV_PIC32MZW_CONN_STATE_CONNECTED:
        {
            /* When HOMEAP connect to PIC32MZW1 STA, Wi-Fi driver updated connected event */
            /* Updating Wi-Fi service associate handle on receiving driver 
               connection event */
            g_wifiSrvcDrvAssocHdl = assocHandle;
            g_wifiSrvcAutoConnectRetry = 0;
            break;
        }

        case WDRV_PIC32MZW_CONN_STATE_FAILED:
        {
            /* When user provided HOMEAP configuration is not matching with near 
               by available HOMEAPs,Wi-Fi driver updated event Fail. */
            SYS_CONSOLE_PRINT(" Trying to connect to SSID : %s \r\n STA Connection failed. \r\n \r\n", SYS_WIFI_GetSSID());
            
            /* check user has enable the Auto connect feature in the STA
               mode,Auto connect Retry count is less then user configured 
               auto connect retry then request Wi-Fi driver for connection request */
            if ((true == SYS_WIFI_GetAutoConnect()) && (g_wifiSrvcAutoConnectRetry < MAX_AUTO_CONNECT_RETRY)) 
            {
                SYS_WIFI_SetTaskstatus(SYS_WIFI_STATUS_CONNECT_REQ);
                g_wifiSrvcAutoConnectRetry++;
            } 
            else if (g_wifiSrvcAutoConnectRetry == MAX_AUTO_CONNECT_RETRY) 
            {
                /* Auto connect Retry count is equal to Maximum connection attempt,
                   then set the Wi-Fi Service status to connection error and wait 
                   for user to re-configuration. */
                SYS_WIFI_SetTaskstatus(SYS_WIFI_STATUS_CONNECT_ERROR);
            }
            g_wifiSrvcDrvAssocHdl = WDRV_PIC32MZW_ASSOC_HANDLE_INVALID;
            break;
        }

        case WDRV_PIC32MZW_CONN_STATE_DISCONNECTED:
        {
            /* when PIC32MZW1 STA disconnected from connected HOMEAP,Wi-Fi driver 
               updated event disconnected. */
            SYS_CONSOLE_PRINT("STA DisConnected\r\n");
            if (true == SYS_WIFI_GetAutoConnect()) 
            {
                SYS_WIFI_SetTaskstatus(SYS_WIFI_STATUS_CONNECT_REQ);
            }

            /* Updating Wi-Fi service associate handle on receiving disconnection
               event */
            g_wifiSrvcDrvAssocHdl = WDRV_PIC32MZW_ASSOC_HANDLE_INVALID;

            /* Reset the Auto Connection retry to zero on receiving disconnect 
               event */
            g_wifiSrvcAutoConnectRetry = 0;
            break;
        }
        default:
        {
            break;
        }
    }       
}

/* Wi-Fi driver callback received on setting Regulatory domain */
static void SYS_WIFI_RegDomainCallback
(
    DRV_HANDLE handle, 
    uint8_t index, 
    uint8_t ofTotal, 
    bool isCurrent, 
    const WDRV_PIC32MZW_REGDOMAIN_INFO * const pRegDomInfo
) 
{
    if ((1 != index) || (1 != ofTotal) || (false == isCurrent) || (NULL == pRegDomInfo)) 
    {
        SYS_CONSOLE_MESSAGE("Regulatory domain set unsuccessful\r\n");
    }  
    if(!memcmp(pRegDomInfo,SYS_WIFI_GetCountryCode(),strlen((const char *)pRegDomInfo)))
    {
        g_isRegDomainSetReq = true;
    }

}


static void SYS_WIFI_TCPIP_DHCP_EventHandler
(
    TCPIP_NET_HANDLE hNet, 
    TCPIP_DHCP_EVENT_TYPE evType, 
    const void* param
) 
{
    IPV4_ADDR ipAddr;
    IPV4_ADDR gateWayAddr;
    switch (evType) 
    {
        case DHCP_EVENT_BOUND:
        {

            /* TCP/IP Stack BOUND event indicates
               PIC32MZW1 has received the IP address from connected HOMEAP */
            ipAddr.Val = TCPIP_STACK_NetAddress(hNet);
            if (ipAddr.Val) 
            {
                gateWayAddr.Val = TCPIP_STACK_NetAddressGateway(hNet);
                SYS_CONSOLE_PRINT("IP address obtained = %d.%d.%d.%d \r\n",
                        ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                SYS_CONSOLE_PRINT("Gateway IP address = %d.%d.%d.%d \r\n",
                        gateWayAddr.v[0], gateWayAddr.v[1], gateWayAddr.v[2], gateWayAddr.v[3]);

                g_wifiSrvcConfig.staConfig.ipAddr.Val=ipAddr.Val;
                SYS_WIFI_SetTaskstatus(SYS_WIFI_STATUS_STA_IP_RECIEVED);
            }
            break;
        }

        case DHCP_EVENT_CONN_ESTABLISHED:
        {
            break;
        }

        case DHCP_EVENT_CONN_LOST:
        {

            /* Received the TCP/IP Stack Connection Lost event,
               PIC32MZW1 has lost IP address from connected HOMEAP */

            /* Update the application(client) on lost IP address */
            SYS_WIFI_CallBackFun(SYS_WIFI_DISCONNECT,NULL,g_wifiSrvcCookie);
            break;
        }

        default:
        {
            break;
        }
    }
}



static SYS_WIFI_RESULT SYS_WIFI_SetChannel(void)
{
    uint8_t ret = SYS_WIFI_FAILURE;
    uint8_t channel = SYS_WIFI_GetChannel();

    if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSCtxSetChannel(&g_wifiSrvcObj.wifiSrvcBssCtx, channel)) 
    {
        ret = SYS_WIFI_SUCCESS;
    }
    return ret;
}

static uint8_t SYS_WIFI_DisConnect(void)
{
    uint8_t ret = SYS_WIFI_FAILURE;

    if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSDisconnect(g_wifiSrvcObj.wifiSrvcDrvHdl)) 
    {
        ret = SYS_WIFI_SUCCESS;
    }
    return ret;
}

static SYS_WIFI_RESULT SYS_WIFI_ConnectReq(void)
{
    SYS_WIFI_RESULT ret = SYS_WIFI_CONNECT_FAILURE;
    
    if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSConnect(g_wifiSrvcObj.wifiSrvcDrvHdl, &g_wifiSrvcObj.wifiSrvcBssCtx, &g_wifiSrvcObj.wifiSrvcAuthCtx, SYS_WIFI_STAConnCallBack)) 
    {
        ret = SYS_WIFI_SUCCESS ;
    }
    
    if (SYS_WIFI_CONNECT_FAILURE == ret) 
    {
       SYS_WIFI_SetTaskstatus(SYS_WIFI_STATUS_CONNECT_ERROR);	
    }
    return ret;
}

static SYS_WIFI_RESULT SYS_WIFI_SetSSID(void)
{
    SYS_WIFI_RESULT ret = SYS_WIFI_CONFIG_FAILURE;
    uint8_t * ssid = SYS_WIFI_GetSSID();
    uint8_t ssidLen = SYS_WIFI_GetSSIDLen();

    if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSCtxSetSSID(&g_wifiSrvcObj.wifiSrvcBssCtx, ssid, ssidLen)) 
    {
        ret = SYS_WIFI_SUCCESS;
    }
    return ret;
}

static SYS_WIFI_RESULT SYS_WIFI_ConfigReq(void)
{
    SYS_WIFI_RESULT ret = SYS_WIFI_SUCCESS;
    uint8_t authType = SYS_WIFI_GetAuthType();
    uint8_t * const pwd = SYS_WIFI_GetPsk();
    uint8_t pwdLen = SYS_WIFI_GetPskLen();

    if (SYS_WIFI_SUCCESS == SYS_WIFI_SetSSID()) 
    {
        switch (authType) 
        {
            case SYS_WIFI_OPEN:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetOpen(&g_wifiSrvcObj.wifiSrvcAuthCtx)) 
                {
                    ret = SYS_WIFI_CONFIG_FAILURE;
                }
                break;
            }

            case SYS_WIFI_WPA2:
            {                
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&g_wifiSrvcObj.wifiSrvcAuthCtx, pwd, pwdLen, WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL)) 
                {
                    ret = SYS_WIFI_CONFIG_FAILURE;
                }
                break;
            }

            case SYS_WIFI_WPAWPA2MIXED:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&g_wifiSrvcObj.wifiSrvcAuthCtx, pwd, pwdLen, WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL)) 
                {
                    ret = SYS_WIFI_CONFIG_FAILURE;
                }
                break;
            }

            case SYS_WIFI_WPA2WPA3MIXED:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&g_wifiSrvcObj.wifiSrvcAuthCtx, pwd, pwdLen, WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL)) 
                {
                    ret = SYS_WIFI_CONFIG_FAILURE;
                }
                break;
            }

            case SYS_WIFI_WPA3:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&g_wifiSrvcObj.wifiSrvcAuthCtx, pwd, pwdLen, WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL)) 
                {
                    ret = SYS_WIFI_CONFIG_FAILURE;
                }
                break;
            }
            case SYS_WIFI_WEP:
            {
                ret = SYS_WIFI_CONFIG_FAILURE; 
                /* Wi-Fi service doesn't support WEP */
                break;
            }

            default:
            {
                ret = SYS_WIFI_CONFIG_FAILURE;
            }
        }
    }

    if (SYS_WIFI_CONFIG_FAILURE == ret) 
    {
        SYS_CONSOLE_MESSAGE("Error:Enter valid Wi-Fi configuration\r\n");
        SYS_WIFI_SetTaskstatus(SYS_WIFI_STATUS_CONFIG_ERROR);
    }

    return ret;
}

static SYS_WIFI_RESULT SYS_WIFI_SetConfig
(
    SYS_WIFI_CONFIG *wifi_config, 
    SYS_WIFI_STATUS status
) 
{

    /* When Wi-Fi provisioning service is not enable by user from MHC configuration */
    SYS_WIFI_RESULT ret = SYS_WIFI_SUCCESS;

    /* Copy the user Wi-Fi configuration and make connection request */
    memcpy(&g_wifiSrvcConfig,wifi_config,sizeof(SYS_WIFI_CONFIG));
    SYS_WIFI_SetTaskstatus(status);
    return ret;	
}

static uint32_t SYS_WIFI_ExecuteBlock
(
    SYS_MODULE_OBJ object
) 
{
    SYS_STATUS                   tcpIpStat;
    static TCPIP_NET_HANDLE      netHdl;
    SYS_WIFI_OBJ *               wifiSrvcObj = (SYS_WIFI_OBJ *) object;
    uint8_t                      ret =  SYS_WIFI_OBJ_INVALID;

 
    if (&g_wifiSrvcObj == (SYS_WIFI_OBJ*) wifiSrvcObj)
    {    
        switch (wifiSrvcObj->wifiSrvcStatus) 
        {
            case SYS_WIFI_STATUS_INIT:
            { 
                if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&g_wifiSrvcSemaphore, OSAL_WAIT_FOREVER)) 
                {
                    /* When g_wifiSrvcInit set to true :
                        Case 1: When Wi-Fi provisioning service is enabled by user in the MHC, 
                               and when Wi-Fi Configuration read from NVM 
                               using Wi-Fi provisioning is valid  
                    
                        Case 2: When Wi-Fi provisioning service is disabled by user in the MHC, 
                               g_wifiSrvcInit set true by Wi-Fi service initialization */

                    if (true == g_wifiSrvcInit) 
                    {
                        if (SYS_STATUS_READY == WDRV_PIC32MZW_Status(sysObj.drvWifiPIC32MZW1)) 
                        {
                            wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_WDRV_OPEN_REQ;                
                        }
                    }
                    OSAL_SEM_Post(&g_wifiSrvcSemaphore);
                }
                break;
            }

            case SYS_WIFI_STATUS_WDRV_OPEN_REQ:
            {
                if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&g_wifiSrvcSemaphore, OSAL_WAIT_FOREVER)) 
                {
                    wifiSrvcObj->wifiSrvcDrvHdl = WDRV_PIC32MZW_Open(0, 0);
                    if (DRV_HANDLE_INVALID != wifiSrvcObj->wifiSrvcDrvHdl) 
                    {
                        if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_RegDomainGet(wifiSrvcObj->wifiSrvcDrvHdl,WDRV_PIC32MZW_REGDOMAIN_SELECT_CURRENT,SYS_WIFI_RegDomainCallback))
                        {
                            SYS_WIFI_PrintWifiConfig();
                            wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_AUTOCONNECT_WAIT;
                        }
                    }
                    OSAL_SEM_Post(&g_wifiSrvcSemaphore);
                }
                break;
            }

            case SYS_WIFI_STATUS_AUTOCONNECT_WAIT:
            {
                if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&g_wifiSrvcSemaphore, OSAL_WAIT_FOREVER)) 
                {
                    /* When user has enabled the auto connect feature of the Wi-Fi service in MHC(STA mode).
                       or in AP mode, below condition will be always true */

                    if (true == SYS_WIFI_GetAutoConnect()) 
                    {
                        if(g_isRegDomainSetReq == true)
                        {

                            if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_RegDomainSet(wifiSrvcObj->wifiSrvcDrvHdl, SYS_WIFI_GetCountryCode(), SYS_WIFI_RegDomainCallback)) 
                            {
                                
                                g_isRegDomainSetReq = false;
                            }
                        }
                        wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_TCPIP_WAIT_FOR_TCPIP_INIT;
                    }
                    OSAL_SEM_Post(&g_wifiSrvcSemaphore);
                }
                break;
            }

            case SYS_WIFI_STATUS_TCPIP_WAIT_FOR_TCPIP_INIT:
            {
                if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&g_wifiSrvcSemaphore, OSAL_WAIT_FOREVER)) 
                {
                    tcpIpStat = TCPIP_STACK_Status(sysObj.tcpip);
                    if (tcpIpStat < 0) 
                    {
                        SYS_CONSOLE_MESSAGE("  TCP/IP stack initialization failed!\r\n");
                        wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_TCPIP_ERROR;
                    } 
                    else if (tcpIpStat == SYS_STATUS_READY) 
                    {
                        /* PIC32MZW1 network handle*/
                        netHdl = TCPIP_STACK_NetHandleGet("PIC32MZW1");
                        if ((true == TCPIP_DHCP_Enable(netHdl)) && 
                            (TCPIP_STACK_ADDRESS_SERVICE_DHCPC == TCPIP_STACK_AddressServiceSelect(_TCPIPStackHandleToNet(netHdl), TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON))) 
                        {
                            g_wifiSrvcDhcpHdl= TCPIP_DHCP_HandlerRegister (netHdl, SYS_WIFI_TCPIP_DHCP_EventHandler, NULL);
                        }
                    }                
                    wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_CONNECT_REQ;
                    OSAL_SEM_Post(&g_wifiSrvcSemaphore);
                }
                break;
            }

            case SYS_WIFI_STATUS_CONNECT_REQ:
            {
                if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&g_wifiSrvcSemaphore, OSAL_WAIT_FOREVER)) 
                {
                    if (SYS_WIFI_SUCCESS == SYS_WIFI_SetChannel()) 
                    {
                        if (SYS_WIFI_SUCCESS == SYS_WIFI_ConfigReq()) 
                        {
						

                            if (SYS_WIFI_SUCCESS == SYS_WIFI_ConnectReq()) 
                            {
                                wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_TCPIP_READY;
                            }
                        }
                    }
                    OSAL_SEM_Post(&g_wifiSrvcSemaphore);
                }
                break;
            }
            case SYS_WIFI_STATUS_STA_IP_RECIEVED:
            {
                WDRV_PIC32MZW_CHANNEL_ID channel = 0;
                 
                /* Update the application(client) on receiving IP address */
                SYS_WIFI_CallBackFun(SYS_WIFI_CONNECT, &g_wifiSrvcConfig.staConfig.ipAddr, g_wifiSrvcCookie);
                wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_TCPIP_READY;
                break;
            }
			

			
            case SYS_WIFI_STATUS_CONNECT_ERROR:
            {
                if (g_wifiSrvcAutoConnectRetry == MAX_AUTO_CONNECT_RETRY)
                {
                    SYS_WIFI_CallBackFun(SYS_WIFI_AUTO_CONNECT_FAIL, NULL, g_wifiSrvcCookie); 
                    wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_CONFIG_ERROR;
                }
                break;
            }



            case SYS_WIFI_STATUS_TCPIP_READY:
            {
                break;
            }

            case SYS_WIFI_STATUS_TCPIP_ERROR:
            {
                SYS_STATUS tcpIpStat;
                tcpIpStat = TCPIP_STACK_Status(sysObj.tcpip);
            
                if (tcpIpStat < 2) 
                {
                    wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_TCPIP_ERROR;
                } 
                else 
                {
                    wifiSrvcObj->wifiSrvcStatus = SYS_WIFI_STATUS_TCPIP_WAIT_FOR_TCPIP_INIT;
                }
                break;
            }

            default:
            {
                break;
            }
        }
        ret = wifiSrvcObj->wifiSrvcStatus;
    }
    return ret;
}


// *****************************************************************************
// *****************************************************************************
// Section:  SYS WIFI Initialize Interface 
// *****************************************************************************
// *****************************************************************************
SYS_MODULE_OBJ SYS_WIFI_Initialize
(
    SYS_WIFI_CONFIG *config, 
    SYS_WIFI_CALLBACK callback, 
    void *cookie
) 
{

    if (SYS_WIFI_STATUS_NONE == SYS_WIFI_GetTaskstatus()) 
    {
        if (OSAL_SEM_Create(&g_wifiSrvcSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1) != OSAL_RESULT_TRUE) 
        {
            SYS_CONSOLE_MESSAGE("Failed to Initialize Wi-Fi Service as Semaphore NOT created\r\n");
            return SYS_MODULE_OBJ_INVALID;
        }
        memset(g_wifiSrvcCallBack,0,sizeof(g_wifiSrvcCallBack));
        if (callback != NULL) 
        {
            SYS_WIFI_REGCB(callback);
        }
        WDRV_PIC32MZW_BSSCtxSetDefaults(&g_wifiSrvcObj.wifiSrvcBssCtx);
        WDRV_PIC32MZW_AuthCtxSetDefaults(&g_wifiSrvcObj.wifiSrvcAuthCtx);

        /* Set Wi-Fi service state to init */
        SYS_WIFI_SetTaskstatus(SYS_WIFI_STATUS_INIT);
        SYS_WIFI_SetCookie(cookie);

        /* Wi-Fi provisioning service is disabled */
        SYS_WIFI_InitConfig(config);
        g_wifiSrvcInit = true;
        return (SYS_MODULE_OBJ) &g_wifiSrvcObj;
    }
    return SYS_MODULE_OBJ_INVALID;
}

// *****************************************************************************
// *****************************************************************************
// Section:  SYS WIFI Deinitialize Interface 
// *****************************************************************************
// *****************************************************************************
SYS_WIFI_RESULT SYS_WIFI_Deinitialize
(
    SYS_MODULE_OBJ object
) 
{
    uint32_t ret = SYS_WIFI_OBJ_INVALID;
    
    if (&g_wifiSrvcObj == (SYS_WIFI_OBJ *) object) 
    {
        SYS_WIFI_DisConnect();
        TCPIP_DHCP_HandlerDeRegister(g_wifiSrvcDhcpHdl);
        WDRV_PIC32MZW_Close(g_wifiSrvcObj.wifiSrvcDrvHdl);
        g_wifiSrvcInit = false;
        memset(&g_wifiSrvcObj,0,sizeof(SYS_WIFI_OBJ));
        memset(g_wifiSrvcCallBack,0,sizeof(g_wifiSrvcCallBack));
        SYS_WIFI_SetTaskstatus(SYS_WIFI_STATUS_NONE);
        if (OSAL_SEM_Delete(&g_wifiSrvcSemaphore) != OSAL_RESULT_TRUE) 
        {
            SYS_CONSOLE_MESSAGE("Failed to Delete Wi-Fi Service Semaphore \r\n");
        }
        ret = SYS_WIFI_SUCCESS;
    }
    return ret;
}

// *****************************************************************************
// *****************************************************************************
// Section:  SYS WiFi Get Status Interface
// *****************************************************************************
// *****************************************************************************
uint8_t SYS_WIFI_GetStatus
(
    SYS_MODULE_OBJ object
) 
{
    uint8_t ret = SYS_WIFI_OBJ_INVALID;
    
    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&g_wifiSrvcSemaphore, OSAL_WAIT_FOREVER)) 
    {
        if (&g_wifiSrvcObj == (SYS_WIFI_OBJ *) object) 
        {
            /* Provide current status of Wi-Fi service to client */
            ret = ((SYS_WIFI_OBJ *) object)->wifiSrvcStatus;
        }
        OSAL_SEM_Post(&g_wifiSrvcSemaphore);
    }
    return ret;
}

// *****************************************************************************
// *****************************************************************************
// Section:  SYS WiFi Tasks Interface
// *****************************************************************************
// *****************************************************************************
uint8_t SYS_WIFI_Tasks
(
    SYS_MODULE_OBJ object
) 
{
    uint8_t ret = SYS_WIFI_OBJ_INVALID;

    if (&g_wifiSrvcObj == (SYS_WIFI_OBJ *) object) 
    {
        ret = SYS_WIFI_ExecuteBlock(object);
    }
    return ret;
}

// *****************************************************************************
// *****************************************************************************
// Section:  SYS WiFi Control Message Interface
// *****************************************************************************
// *****************************************************************************
SYS_WIFI_RESULT SYS_WIFI_CtrlMsg
(
    SYS_MODULE_OBJ object, 
    uint32_t event, 
    void *buffer, 
    uint32_t length
) 
{
    uint8_t ret = SYS_WIFI_OBJ_INVALID;

    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&g_wifiSrvcSemaphore, OSAL_WAIT_FOREVER)) 
    {
        if (&g_wifiSrvcObj == (SYS_WIFI_OBJ *)object) 
        {
            switch (event) 
            {

                case SYS_WIFI_CONNECT:
                {
                    /* if service is already processing pending request from 
                       client then ignore new request */
                    if (SYS_WIFI_STATUS_CONNECT_REQ != g_wifiSrvcObj.wifiSrvcStatus) 
                    {
                        if ((buffer) && (length == sizeof (SYS_WIFI_CONFIG)))
                        {
                            SYS_WIFI_STATUS wifiStatus = ((SYS_WIFI_OBJ *)object)->wifiSrvcStatus;
                            if(wifiStatus >= SYS_WIFI_STATUS_CONNECT_REQ)
                            { 
                                wifiStatus = SYS_WIFI_STATUS_CONNECT_REQ; 
                            }
                            ret = SYS_WIFI_SetConfig((SYS_WIFI_CONFIG *) buffer, wifiStatus);
                        }
                    } 
                    else
                    {
                        /* Client is currently processing pending connection request */
                        ret = SYS_WIFI_CONNECT_FAILURE;
                    }
                    break;
                }

                case SYS_WIFI_REGCALLBACK:
                {
                    SYS_WIFI_CALLBACK g_wifiSrvcFunPtr = buffer;
                    if ((g_wifiSrvcFunPtr) && (length == sizeof(g_wifiSrvcFunPtr))) 
                    {
                        /* Register the client callback function */
                        ret = SYS_WIFI_REGCB(g_wifiSrvcFunPtr);
                    }
                    break;
                }

                case SYS_WIFI_DISCONNECT:
                {
                    /* Client has made disconnect request */
                    g_wifiSrvcConfig.staConfig.autoConnect = 0;
                    ret = SYS_WIFI_DisConnect();
                    break;
                }

                case SYS_WIFI_GETWIFICONFIG:
                {
                    if (true == g_wifiSrvcInit) 
                    {
                        if ((buffer) && (length == sizeof (SYS_WIFI_CONFIG))) 
                        {

                            /* Client has request Wi-Fi service configuration,
                               Copy Wi-Fi configuration into client structure */
                            memcpy(buffer, &g_wifiSrvcConfig, sizeof (g_wifiSrvcConfig));
                            ret = SYS_WIFI_SUCCESS;
                        }
                    } 
                    else
                    {

                        /* Wi-Fi service is not started */
                        ret = SYS_WIFI_SERVICE_UNINITIALIZE;
                    }
                    break;
                }


                case SYS_WIFI_GETDRVHANDLE:
                {
                    if ((buffer) && (length == sizeof (DRV_HANDLE))) 
                    {
                        /* Client has requested Wi-Fi driver handle,
                        Copy driver handle into client structure */
                        *(DRV_HANDLE *)buffer = g_wifiSrvcObj.wifiSrvcDrvHdl;
                        ret = SYS_WIFI_SUCCESS;
                    }
                    else
                    {
                        ret = SYS_WIFI_FAILURE;
                    }
                    break;
                }
                case SYS_WIFI_GETDRVASSOCHANDLE:
                {
                    if ((buffer) && (length == sizeof (WDRV_PIC32MZW_ASSOC_HANDLE))) 
                    {
                        /* Client has requested Assoc handle,
                        Copy Assoc handle into client structure */
                        *(WDRV_PIC32MZW_ASSOC_HANDLE *)buffer = g_wifiSrvcDrvAssocHdl;
                        ret = SYS_WIFI_SUCCESS;
                    }
                    else
                    {
                        ret = SYS_WIFI_FAILURE;
                    }
                    break;
                }
            }
        }
        OSAL_SEM_Post(&g_wifiSrvcSemaphore);
    }
    return ret;
}
/* *****************************************************************************
 End of File
 */
