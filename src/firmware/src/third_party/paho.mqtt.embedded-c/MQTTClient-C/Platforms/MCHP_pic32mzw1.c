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

 #include "MCHP_pic32mzw1.h"
#include "string.h"
#include "tcpip/tcpip.h"
#include <stdlib.h>
#include "definitions.h"
#include "system/mqtt/sys_mqtt_paho.h"


char TimerIsExpired(Timer* timer) {
    long left = 0;
    if (timer)
    {
        left = timer->end_time - SYS_TMR_TickCountGet();
        return (left < 0);
    }
    else 
        return 0;
}

void TimerCountdownMS(Timer* timer, unsigned int timeout) {
	timer->end_time = SYS_TMR_TickCountGet() + timeout;
}

void TimerCountdown(Timer* timer, unsigned int timeout) {
	timer->end_time = SYS_TMR_TickCountGet() + (timeout * 1000);
}

int TimerLeftMS(Timer* timer) {
    if (timer == NULL)
        return 0;
	long left = timer->end_time - SYS_TMR_TickCountGet();
	return (left < 0) ? 0 : left;
}

void TimerInit(Timer* timer) {
	timer->end_time = 0;
}

int pic32mzw1_read(Network* n, unsigned char* buffer, int len, int timeout_ms) 
{ 
    int recv_len = 0;
    SYS_MODULE_OBJ obj = SYS_MQTT_Paho_GetNetHdlFromNw(n);

    if(obj  == SYS_MODULE_OBJ_INVALID)
    {
        SYS_CONSOLE_PRINT("pic32mzw1_write():Invalid Nw Handle\r\n");
		return -1;
    }
    recv_len = SYS_NET_RecvMsg(obj, buffer, len);
    return recv_len;
}

int pic32mzw1_write(Network* n, unsigned char* buffer, int len, int timeout_ms) 
{
    int bytes_sent = 0;
    SYS_MODULE_OBJ obj = SYS_MQTT_Paho_GetNetHdlFromNw(n);

    if(obj  == SYS_MODULE_OBJ_INVALID)
    {
        SYS_CONSOLE_PRINT("pic32mzw1_write():Invalid Nw Handle\r\n");
		return -1;
    }

    bytes_sent = SYS_NET_SendMsg(obj, (uint8_t*)buffer, len); 
    if( bytes_sent <= 0)
    {
        SYS_CONSOLE_PRINT("SYS_NET_SendMsg():Failed\r\n");
		return -1;
    }

  //TODO: figure out how to get actual send length from callback
  //this length will be updated in the callback
  //return gu32MQTTBrokerSendLen;
  return bytes_sent;
}

void pic32mzw1_disconnect(Network* n) 
{
    SYS_MODULE_OBJ obj = SYS_MQTT_Paho_GetNetHdlFromNw(n);

    if(obj  == SYS_MODULE_OBJ_INVALID)
    {
        SYS_CONSOLE_PRINT("pic32mzw1_write():Invalid Nw Handle\r\n");
		return;
    }

    SYS_NET_CtrlMsg(SYS_MQTT_Paho_GetNetHdlFromNw(n), 
                        SYS_NET_CTRL_MSG_DISCONNECT, 
                        NULL, 
                        0);
}

void NetworkInit(Network* n) 
{
	n->mqttread = pic32mzw1_read;
	n->mqttwrite = pic32mzw1_write;
	n->disconnect = pic32mzw1_disconnect;
}
