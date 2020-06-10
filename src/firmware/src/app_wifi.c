/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_wifi.c

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
#include "app_wifi.h"
#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_sta.h"
#include "wdrv_pic32mzw_authctx.h"
#include "wdrv_pic32mzw_bssctx.h"
#include "wdrv_pic32mzw_bssfind.h"
#include "wdrv_pic32mzw_common.h"
#include "tcpip/tcpip_manager.h"
#include "cryptoauthlib.h"
#include "app_control.h"

#define DEVICE_CHANNEL WDRV_PIC32MZW_CID_ANY

extern uint8_t g_macaddress[6];

typedef struct wifiConfiguration {
    WDRV_PIC32MZW_AUTH_CONTEXT authCtx;
    WDRV_PIC32MZW_BSS_CONTEXT bssCtx;
} wifiConfig;

typedef struct {
    APP_WIFI_STATES state;
    DRV_HANDLE wdrvHandle;
    volatile bool isRegDomainSet;
    volatile bool isConnected;
} APP_WIFI_DATA;

char *WIFI_AUTH_STRING[] = {
    "OPEN", /*0*/
    "WPA2",
    "WPAWPA2MIXED",
    "WEP",
    "NONE"
};

APP_WIFI_DATA app_wifiData;
static wifiConfig g_wifiConfig;
static WDRV_PIC32MZW_ASSOC_HANDLE drvAssocHandle;
static TCPIP_EVENT_HANDLE TCPIP_event_handle;

static void PIC32WIFI_ConnectCallback(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE assocHandle, WDRV_PIC32MZW_CONN_STATE currentState) {
    switch (currentState) {
        case WDRV_PIC32MZW_CONN_STATE_DISCONNECTED:
            SYS_CONSOLE_PRINT("APP_WIFI: " TERM_YELLOW "WiFi Reconnecting\r\n" TERM_RESET);
            app_wifiData.isConnected = false;
            app_controlData.wifiCtrl.wifiConnected = false;
            app_wifiData.state = APP_WIFI_RECONNECT;
            app_controlData.rssiData.assocHandle=NULL;
            break;
        case WDRV_PIC32MZW_CONN_STATE_CONNECTED:
            app_wifiData.isConnected = true;
            app_controlData.wifiCtrl.wifiConnected = true;
            app_controlData.rssiData.assocHandle=assocHandle;
            SYS_CONSOLE_PRINT("APP_WIFI: " TERM_GREEN "WiFi Connected\r\n" TERM_RESET);
            break;
        case WDRV_PIC32MZW_CONN_STATE_FAILED:
            app_wifiData.isConnected = false;
            app_controlData.wifiCtrl.wifiConnected = false;
            app_controlData.rssiData.assocHandle=NULL;
            SYS_CONSOLE_PRINT("APP_WIFI: " TERM_RED "WiFi connection Failed\r\n" TERM_RESET);
            app_wifiData.state = APP_WIFI_RECONNECT;
            break;
        case WDRV_PIC32MZW_CONN_STATE_CONNECTING:
            SYS_CONSOLE_PRINT("APP_WIFI: " TERM_YELLOW "WiFi Connecting\r\n" TERM_RESET);
            app_wifiData.isConnected = false;
            app_controlData.wifiCtrl.wifiConnected = false;
            app_controlData.rssiData.assocHandle=NULL;
            break;
    }
    drvAssocHandle = assocHandle;
}

static void APP_TcpipStack_EventHandler(TCPIP_NET_HANDLE hNet, TCPIP_EVENT event, const void *fParam) {
    const char *netName = TCPIP_STACK_NetNameGet(hNet);
    if (event & TCPIP_EV_CONN_ESTABLISHED) {
        //SYS_CONSOLE_PRINT("APP_WIFI: %s - TCPIP stack connection established\r\n", netName);
        if (TCPIP_DHCP_IsEnabled(hNet) == false) {
            //TCPIP_STACK_AddressServiceSelect((TCPIP_NET_IF*)&hNet, TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON);
            //SYS_CONSOLE_PRINT("APP_WIFI: Starting DHCP Client for %s\r\n", netName);
            TCPIP_DHCP_Enable(hNet);
        } else {
            //SYS_CONSOLE_PRINT("APP_WIFI: DHCP Client Already running...!\r\n");
        }
    } else if (event & TCPIP_EV_CONN_LOST) {
        //SYS_CONSOLE_PRINT("APP_WIFI: %s - TCPIP stack connection lost\r\n", netName);
        //SYS_CONSOLE_PRINT("APP_WIFI: Stop DHCP Client for %s\r\n", netName);
        TCPIP_DHCP_Disable(hNet);
    } else {
        SYS_CONSOLE_PRINT("APP_WIFI: %s Unknown event event = %d\r\n", netName, event);
    }
}

static bool WIFI_Config(char *ssid, char *pass, WIFI_AUTH auth) {
    bool ret = true;
    uint8_t ssidLength = strlen((const char *) ssid);
    uint8_t pskLength = strlen(pass);

    WDRV_PIC32MZW_BSSCtxSetChannel(&g_wifiConfig.bssCtx, DEVICE_CHANNEL);
    //WDRV_PIC32MZW_BSSFindFirst(app_wifiData.wdrvHandle, DEVICE_CHANNEL, true, APP_WiFi_Scan_Handler);

    SYS_CONSOLE_PRINT("APP_WIFI: SSID is %s \r\n", ssid);

    if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSCtxSetSSID(&g_wifiConfig.bssCtx, (uint8_t * const) ssid, ssidLength)) {
        switch (auth) {
            case OPEN:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetOpen(&g_wifiConfig.authCtx)) {
                    SYS_CONSOLE_MESSAGE("APP_WFI: " TERM_RED "ERROR" TERM_RESET "- Unable to set Authentication\r\n");
                    ret = false;
                }
                break;
            }
            case WPA2:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&g_wifiConfig.authCtx, (uint8_t *) pass, pskLength, WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL)) {
                    SYS_CONSOLE_MESSAGE("APP_WIFI: " TERM_RED "ERROR" TERM_RESET " -Unable to set authentication to WPA2\r\n");
                    ret = false;
                }
                break;
            }
            case WPAWPA2MIXED:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&g_wifiConfig.authCtx, (uint8_t *) pass, pskLength, WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL)) {
                    SYS_CONSOLE_MESSAGE("WPP_WIFI: " TERM_RED "ERROR" TERM_RESET " - Unable to set authentication to WAPWPA2 MIXED\r\n");
                    ret = false;
                }
                break;
            }
            case WEP:
            {
                //TODO
                break;
            }
            default:
            {
                SYS_CONSOLE_PRINT("APP_WIFI: " TERM_RED "ERROR" TERM_RESET ": Set Authentication type");
                ret = false;
            }
        }
    }
    return ret;
}

void APP_WIFI_Initialize(void) {

    /* Place the App state machine in its initial state. */
    app_wifiData.state = APP_WIFI_STATE_INIT;
    app_wifiData.isConnected = false;
    app_controlData.wifiCtrl.wifiConnected = false;
    if (!WDRV_PIC32MZW_BSSCtxIsValid(&g_wifiConfig.bssCtx, true)) {
        //if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetDefaults(&g_wifiConfig.bssCtx))
        //SYS_CONSOLE_MESSAGE("APP_WIFI: BSS Set Default\n");
        WDRV_PIC32MZW_BSSCtxSetDefaults(&g_wifiConfig.bssCtx);
    }
}

void APP_WIFI_Tasks(void) {
    SYS_STATUS tcpipStat;
    bool status;
    //    TCPIP_MAC_ADDR macAddr;
    TCPIP_NET_HANDLE netH;
    int i, nNets;

    switch (app_wifiData.state) {
        case APP_WIFI_STATE_INIT:
            /*This is used to disable WiFi with a switch press. */

            if (SYS_STATUS_READY == WDRV_PIC32MZW_Status(sysObj.drvWifiPIC32MZW1)) {
                app_wifiData.state = APP_WIFI_STATE_WDRV_INIT_READY;
            }
#if 0            
            extern ATCAIfaceCfg atecc608a_0_init_data;
            uint8_t sernum[9];
            char displayStr[ATCA_SERIAL_NUM_SIZE * 3];
            size_t displen = sizeof (displayStr);
            ATCA_STATUS atcaStat;
            atcaStat = ATCA_TX_FAIL;
            atcaStat = atcab_init(&atecc608a_0_init_data);
            if (ATCA_SUCCESS == atcaStat) {
                atcaStat = atcab_read_serial_number(sernum);
                atcab_bin2hex(sernum, 9, displayStr, &displen);
                SYS_CONSOLE_PRINT("Serial Number of the Device: %s\r\n\n", displayStr);
            }
            atcab_release();
#endif
            break;

        case APP_WIFI_STATE_WDRV_INIT_READY:
        {
            app_wifiData.wdrvHandle = WDRV_PIC32MZW_Open(0, 0);
            if (DRV_HANDLE_INVALID != app_wifiData.wdrvHandle) {
                app_wifiData.state = APP_WIFI_TCPIP_WAIT_FOR_TCPIP_INIT;
            } else {
            }
            break;
        }
        case APP_WIFI_TCPIP_WAIT_FOR_TCPIP_INIT:
        {
            tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);

            if (tcpipStat < 0) { // some error occurred
                SYS_CONSOLE_MESSAGE("APP_WIFI: " TERM_RED "TCP/IP stack initialization failed!\r\n" TERM_RESET);
                app_wifiData.state = APP_WIFI_TCPIP_ERROR;
            } else if (SYS_STATUS_READY == tcpipStat) {
                nNets = TCPIP_STACK_NumberOfNetworksGet();
                for (i = 0; i < nNets; i++) {
                    netH = TCPIP_STACK_IndexToNet(i);
                    //if (strcmp(TCPIP_STACK_IF_NAME_PIC32MZW1, TCPIP_STACK_NetNameGet(netH))) {
                    TCPIP_event_handle = TCPIP_STACK_HandlerRegister(netH, TCPIP_EV_CONN_ALL, APP_TcpipStack_EventHandler, NULL);
                    //TCPIP_DHCP_HandlerRegister(netH, APP_TcpipDhcp_EventHandler, NULL);
                    //TCPIP_DHCP_HandlerDeRegister();
                    //}
                }
#if 0
                /*Temporary workaround for stack MAC address issue. */
                netH = TCPIP_STACK_NetHandleGet(TCPIP_STACK_IF_NAME_PIC32MZW1);
                if (netH == 0) {
                    SYS_CONSOLE_MESSAGE("APP_WIFI: " TERM_RED "SetMAC- Unknown interface specified \r\n" TERM_RESET);
                } else {
                    memcpy(macAddr.v, g_macaddress, 6);
                    if (!TCPIP_STACK_NetAddressMacSet(netH, &macAddr)) {
                        SYS_CONSOLE_MESSAGE("APP_WIFI: " TERM_RED "Set MAC address failed. Will use default\r\n" TERM_RESET);
                    } else {
                        SYS_CONSOLE_PRINT("APP_WIFI ("TERM_RED"WORKAROUND"TERM_RESET"): MAC address set to " TERM_YELLOW "%02X:%02X:%02X:%02X:%02X:%02X\r\n" TERM_RESET,
                                g_macaddress[0], g_macaddress[1], g_macaddress[2], g_macaddress[3],
                                g_macaddress[4], g_macaddress[5], g_macaddress[6]);
                    }
                }
#endif
                app_wifiData.state = APP_WIFI_CONFIG;
            }
            break;
        }
        case APP_WIFI_CONFIG:
        {
            if (true == app_controlData.wifiCtrl.wifiCtrlValid) {
                status = WIFI_Config(app_controlData.wifiCtrl.SSID, app_controlData.wifiCtrl.pass, app_controlData.wifiCtrl.authmode);
            } else {
                break;
            }

            if (status) {
                SYS_CONSOLE_PRINT("APP_WIFI: " TERM_YELLOW "Connecting to Wi-Fi\r\n" TERM_RESET);
                app_wifiData.state = APP_WIFI_CONNECT;
            } else {
                SYS_CONSOLE_MESSAGE("APP_WIFI: " TERM_RED "Failed connecting to Wi-Fi\r\n" TERM_RESET);
                app_wifiData.state = APP_WIFI_TCPIP_ERROR;
            }
            break;
        }
        case APP_WIFI_CONNECT:
        {
            if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSConnect(app_wifiData.wdrvHandle, &g_wifiConfig.bssCtx, &g_wifiConfig.authCtx, PIC32WIFI_ConnectCallback)) {
                //SYS_CONSOLE_PRINT("APP_WIFI: " TERM_YELLOW "Waiting for WiFi Connect Callback\r\n" TERM_RESET);
                app_wifiData.state = APP_WIFI_IDLE;
            }
            break;
        }
        case APP_WIFI_IDLE:
            if (true == app_controlData.wifiCtrl.wifiCtrlChanged) {
                app_wifiData.state = APP_WIFI_RECONNECT;
                app_controlData.wifiCtrl.wifiCtrlChanged = false;
                /*TODO: Workaround until de-init re-inint is functional*/
                softResetDevice();
            }
            break;
        case APP_WIFI_RECONNECT:
            TCPIP_STACK_HandlerDeregister(TCPIP_event_handle);
            WDRV_PIC32MZW_Close(app_wifiData.wdrvHandle);
            app_wifiData.state = APP_WIFI_STATE_INIT;
            break;
        case APP_WIFI_TCPIP_ERROR:
            break;
        default:
        {
            break;
        }
    }
}
/*******************************************************************************
 End of File
 */
