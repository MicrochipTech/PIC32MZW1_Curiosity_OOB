/*******************************************************************************
  Net System Service Interface Header File

  Company
    Microchip Technology Inc.

  File Name
    sys_net.h

  Summary
    Net system service interface.

  Description
    This file defines the interface to the Net system service.  This
    system service provides a simple APIs to enable PIC32MZW1 Networking 
	Functionality.

  Remarks:
    None
*******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

#ifndef SYS_NET_H    // Guards against multiple inclusion
#define SYS_NET_H

#include <stdlib.h>
#include "definitions.h"
#include "tcpip/tcpip.h"
#include "net_pres/pres/net_pres_socketapi.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types and Constants
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* SYS_NET_MODE_CLIENT

  Summary:
    Net Socket Mode - Client

  Remarks:
    None.
*/
#define SYS_NET_MODE_CLIENT     0			// Client Mode Value

// *****************************************************************************
/* SYS_NET_MODE_SERVER

  Summary:
    Net Socket Mode - Server

  Remarks:
    None.
*/
#define SYS_NET_MODE_SERVER     1			// Server Mode Value

// *****************************************************************************
/* SYS_NET_MAX_HOSTNAME_LEN

  Summary:
    Max Host Name Length

  Remarks:
    None.
*/
#define SYS_NET_MAX_HOSTNAME_LEN   	256    	// Max Host Name Length

// *****************************************************************************
/* SYS_NET_IP_PROT_UDP

  Summary:
    Ip Protocol Mode - UDP

  Remarks:
    None.
*/
#define SYS_NET_IP_PROT_UDP     0			// UDP - Ip Protocol Value

// *****************************************************************************
/* SYS_NET_IP_PROT_TCP

  Summary:
    Ip Protocol Mode - TCP

  Remarks:
    None.
*/
#define SYS_NET_IP_PROT_TCP     1			// TCP - Ip Protocol Value


// *****************************************************************************
/* SYS_NET_IP_PROT_TCP

  Summary:
    Number of Instances Supported by the NET System Service

  Remarks:
    None.
*/
#define SYS_NET_MAX_NUM_OF_SOCKETS     2			// umber of Instances Supported by the NET System Service

// *****************************************************************************
/* SYS_NET_DEFAULT_TLS_ENABLE

  Summary:
    Default Values for TLS - False

  Remarks:
    None.
*/
#define SYS_NET_DEFAULT_TLS_ENABLE      0	// TLS Disabled by default

// *****************************************************************************
/* SYS_NET_DEFAULT_AUTO_RECONNECT

  Summary:
    Default Values for Auto Reconnect - True

  Remarks:
    None.
*/
#define SYS_NET_DEFAULT_AUTO_RECONNECT  1	// Auto Reconnect Enabled by default

// *****************************************************************************
/* SYS_NET_DEFAULT_NET_INTF

  Summary:
    Default Values for Interface - 0 (Wifi)

  Remarks:
    None.
*/
#define SYS_NET_DEFAULT_NET_INTF        0	// Interface 0 by default
 
// *****************************************************************************
/* AppDebug Flows

  Summary:
    AppDebug Flow for the Logs - Configuration

  Remarks:
    None.
*/
#define NET_CFG         0x1					// App Debug Print Flows - CFG

// *****************************************************************************
/* AppDebug Flows

  Summary:
    AppDebug Flow for the Logs - Data

  Remarks:
    None.
*/
#define NET_DATA        0x2					// App Debug Print Flows - DATA

// *****************************************************************************
/* System NET Instance Configuration

  Summary:
    Used for passing on the configuration related to the Net Socket that needs 
	to be opened via the Sys Net Service

  Remarks:
    None.
*/
typedef struct {
	// Net Socket Mode to Open - SYS_NET_MODE_CLIENT(0)/  SYS_NET_MODE_SERVER(1)
	uint8_t mode;  
	
	// WiFi or Eth Interface to be used for Opening the socket
	uint8_t	intf; 
	
	// Net Server Port
	uint16_t port; 
	
	// Reconnect in case of disconnection happening - 1(Reconnect Enabled)/ 0(Reconnect Disabled)
	bool enable_reconnect; 
	
	// Net Socket with 1(TLS Enabled)/ 0(TLS Disabled)
    bool enable_tls; 
    
	// Socket IP Protocol - SYS_NET_IP_PROT_UDP(0) or SYS_NET_IP_PROT_TCP(1)
	uint8_t ip_prot; 
	
	// Host Name - could have the server name or IP
	char	host_name[SYS_NET_MAX_HOSTNAME_LEN]; 
} SYS_NET_Config;

extern const SYS_NET_Config g_sSysNetConfig; //Data structure which has the MHC configuration for the NET service. 


// *****************************************************************************
/* System NET Instance Status

  Summary:
    Identifies the current status of the Sys Net Instance.

  Remarks:
    None.
*/
typedef enum {
	// Net Instance is Idle/ Not in Use
    SYS_NET_STATUS_IDLE = 0, 			
	
	// Lower Layer is Down
    SYS_NET_STATUS_LOWER_LAYER_DOWN, 	
	
	// Resolving DNS of NET Server for the Client to connect
    SYS_NET_STATUS_RESOLVING_DNS, 		
	
	// Net Server IP Available for the Client to connect
    SYS_NET_STATUS_DNS_RESOLVED, 		
	
	// Net Server Awaiting Connection
    SYS_NET_STATUS_SERVER_AWAITING_CONNECTION, 
	
	// Net Client connecting to Server
    SYS_NET_STATUS_CLIENT_CONNECTING, 
	
#ifdef SYS_NET_TLS_ENABLED	
	// Net Client Waiting for SNTP Time Stamp
    SYS_NET_STATUS_WAIT_FOR_SNTP, 
    
	// Net Client Starting TLS Negotiations
	SYS_NET_STATUS_TLS_NEGOTIATING, 
	
	// Net Instance TLS Negotiation Failed
	SYS_NET_STATUS_TLS_NEGOTIATION_FAILED, 
#endif	
	// Net Instance connected to the peer
    SYS_NET_STATUS_CONNECTED, 

	// Net Instance Failed to open socket
	SYS_NET_STATUS_SOCK_OPEN_FAILED, 

	// Net Instance Failed to Resolve DNS
	SYS_NET_STATUS_DNS_RESOLVE_FAILED, 

	// Net Instance in disconnected state
    SYS_NET_STATUS_DISCONNECTED, 

	// Net Instance received FIN from peer
    SYS_NET_STATUS_PEER_SENT_FIN, 
} SYS_NET_STATUS;

// *****************************************************************************
/* System NET Event values

  Summary:
    Identifies the event type for which the User Callback is called.

  Remarks:
    None.
*/
typedef enum {
    // NET Socket connected to Peer
    SYS_NET_EVNT_CONNECTED = 0,

    // NET Socket disconnected
    SYS_NET_EVNT_DISCONNECTED,

    // Received Data on NET Socket connected to Peer
    SYS_NET_EVNT_RCVD_DATA,

    // SSL Negotiation Failed
    SYS_NET_EVNT_SSL_FAILED,

    // DNS Resolve Failed
    SYS_NET_EVNT_DNS_RESOLVE_FAILED,

    // Socket Open Failed
    SYS_NET_EVNT_SOCK_OPEN_FAILED,
} SYS_NET_EVENT;

// *****************************************************************************
/* System NET Control Message values

  Summary:
    Identifies the control message for which the User has called the SYS_NET_CtrlMsg().

  Remarks:
    None.
*/
typedef enum {
    // NET Socket should reconnect to Peer, the User is expected to pass pointer to SYS_NET_Config for the configuration of the new Connection.
    SYS_NET_CTRL_MSG_RECONNECT = 0,

    // NET Socket disconnect request from the user
    SYS_NET_CTRL_MSG_DISCONNECT,
} SYS_NET_CTRL_MSG;

// *****************************************************************************
/* System NET Return values

  Summary:
    Identifies the return values for the Sys Net APIs.

  Remarks:
    None.
*/
typedef enum {
    // Success
	SYS_NET_SUCCESS = 0,

    // Failure
	SYS_NET_FAILURE = -1,
	
    // Sys NET Service Down
	SYS_NET_SERVICE_DOWN = -2,

    // Enough space not available in the transmit buffer to send the message. Application should try again later
	SYS_NET_PUT_NOT_READY = -3,
	
    // Sys NET No Data Available for receiving
	SYS_NET_GET_NOT_READY = -4,

	// Sys NET Semaphore Operation of Take/ Release Failed
	SYS_NET_SEM_OPERATION_FAILURE = -5,
		
	// Sys NET Invalid Handle
	SYS_NET_INVALID_HANDLE = -6,
	
} SYS_NET_RESULT;


// *****************************************************************************
// *****************************************************************************
// Section: Initialization functions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
       int32_t SYS_NET_Initialize()

  Summary:
      Returns success/ failure for initialization of data structures of the NET service

  Description:
       This function is used for initializing the data structures of the NET service and is called from within the System Task.

  Returns:
		SYS_NET_SUCCESS - Indicates the data structures were initialized successfully
		SYS_NET_FAILURE - Indicates that it failed to initialize the data structures.

  Example:
       <code>
		if( SYS_NET_Initialize() == SYS_NET_SUCCESS)
		{
		}
       </code>

  Remarks:
		If the Net system service is enabled using MHC, then auto generated code will take care of Net System Service initialization.
  */

int32_t SYS_NET_Initialize();

// *****************************************************************************
/* Function:
       void SYS_NET_Deinitialize()

  Summary:
      Deinitialization of data structures of the NET service

  Description:
       This function is used for freeing the allocated data structures for the NET service.

  Example:
       <code>
		SYS_NET_Deinitialize()
       </code>

  Remarks:
		None
	   */
void SYS_NET_Deinitialize();

// *****************************************************************************
// *****************************************************************************
// Section: Status functions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
       SYS_NET_STATUS SYS_NET_GetStatus ( SYS_MODULE_OBJ object )

  Summary:
      Returns System NET instance status.

  Description:
       This function returns the current status of the System NET instance.

  Precondition:
       SYS_NET_Open should have been called before calling this function

  Parameters:
       object  - SYS NET object handle, returned from SYS_NET_Open<br>

  Returns:
		SYS_NET_STATUS
		
  Example:
       <code>
       // Handle "objSysNet" value must have been returned from SYS_NET_Open.
       if (SYS_NET_GetStatus (objSysNet) == SYS_NET_STATUS_SERVER_AWAITING_CONNECTION)
       {
           // NET system service is initialized and the NET server is ready to accept new connection.
       }
       </code>

  Remarks:
       None.
  */

SYS_NET_STATUS SYS_NET_GetStatus(SYS_MODULE_OBJ obj);


// *****************************************************************************
// *****************************************************************************
// Section: Data Exchange functions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
       int32_t SYS_NET_SendMsg(SYS_MODULE_OBJ obj, uint8_t *buffer, uint16_t len)

  Summary:
      Returns No of Bytes sent to peer using the System NET instance.

  Description:
       This function returns the number of bytes transmitted to the peer.

  Precondition:
       SYS_NET_Open should have been called.

  Parameters:
       object  	- SYS NET object handle, returned from SYS_NET_Open<br>
	   
	   data		- valid data buffer pointer<br>
	   
	   len		- length of the data to be transmitted<br>

  Returns:
		SYS_NET_SERVICE_DOWN - Indicates that the System NET instance is not connected to the peer
		SYS_NET_PUT_NOT_READY - Indicates that the System NET instance Put is NOT ready
		SYS_NET_PUT_BUFFER_NOT_ENOUGH - Indicates that the System NET instance cannot transmit as the available buffer is less than the bytes to be transmitted
		Positive Non-Zero - Indicates the number of bytes transmitted to the peer

  Example:
       <code>
       // Handle "objSysNet" value must have been returned from SYS_NET_Open.
       while(SYS_NET_SendMsg(objSysNet, "hello", 5) <= 0);
       </code>

  Remarks:
       None.
  */
int32_t SYS_NET_SendMsg(SYS_MODULE_OBJ obj, uint8_t *data, uint16_t len);


// *****************************************************************************
/* Function:
       int32_t SYS_NET_RecvMsg(SYS_MODULE_OBJ obj, void *data, uint16_t len)

  Summary:
      Returns No of Bytes received from peer using the System NET instance.

  Description:
       This function returns the number of bytes received from the peer.

  Precondition:
       SYS_NET_Open should have been called.

  Parameters:
       obj  	- SYS NET object handle, returned from SYS_NET_Open<br>
	   
	   data		- valid data buffer pointer<br>
	   
	   len		- length of the data to be transmitted<br>

  Returns:
		SYS_NET_SERVICE_DOWN - Indicates that the System NET instance is not connected to the peer
		SYS_NET_GET_NOT_READY - Indicates that the System NET instance No Data to GET
		Positive Non-Zero - Indicates the number of bytes received from the peer, which may be less than the "len" of the buffer passed as the param.

  Example:
       <code>
       // Handle "objSysNet" value must have been returned from SYS_NET_Open.	   
		int32_t len = 32;
		uint8_t buffer[32] = {0};
		len = SYS_NET_RecvMsg(objSysNet, buffer, len);
		if(len > 0)
		{
		}
       </code>

  Remarks:
       None.
  */

int32_t SYS_NET_RecvMsg(SYS_MODULE_OBJ obj, void *buffer, uint16_t len);

// *****************************************************************************
/* Function:
    void SYS_NET_CALLBACK (uint32_t event, void *data, void* cookie)

   Summary:
    Pointer to a Net system service callback function.

   Description:
    This data type defines a pointer to a Net service callback function, thus
    defining the function signature.  Callback functions may be registered by
    clients of the net service when opening a Net socket via the Initialize call.

   Precondition:
    Is a part of the Net service initialization using the SYS_NET_Open
    function.

   Parameters:
	event	- An event (SYS_NET_EVENT) for which the callback was called.<br>
	data	- Data (if any) related to the Event<br>
    cookie  - A context value, returned untouched to the client when the
                 callback occurs.<br>
				 
   Returns:
    None.

  Example:
    <code>
    void NetServCallback(uint32_t event, void *data, void* cookie, )
	{
		switch(event)
		{
			case SYS_NET_EVNT_CONNECTED:
			{
				SYS_CONSOLE_PRINT("NetServCallback(): Status UP");
				while(SYS_NET_SendMsg(g_NetServHandle, "hello", 5) == 0);
				break;
			}

			case SYS_NET_EVNT_DISCONNECTED:
			{
				SYS_CONSOLE_PRINT("NetServCallback(): Status DOWN");
				break;
			}

			case SYS_NET_EVNT_RCVD_DATA:
			{
				int32_t len = 32;
				uint8_t buffer[32] = {0};
				len = SYS_NET_RecvMsg(g_NetServHandle, buffer, len);
				SYS_CONSOLE_PRINT("NetServCallback(): Data Rcvd = %s", buffer);
				break;
			}
		}
	}
    </code>

  Remarks:
    None.
*/
typedef void (*SYS_NET_CALLBACK)(uint32_t event, void *data, void* cookie);

// *****************************************************************************
// *****************************************************************************
// Section: Setup functions
// *****************************************************************************
// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ SYS_NET_Open (SYS_NET_Config *cfg, 
										SYS_NET_CALLBACK Net_cb, 
										void *cookie)

   Summary:
        Opens a new NET System Service instance.

   Description:
        This function opens the instance of the NET System Service.

   Parameters:
       cfg  		- Configuration for which the NET Socket needs to be opened<br>
	   Net_cb	 	- Function pointer to the Callback to be called in case of an event<br>
       cookie		- Cookie passed as one of the params in the Callback which was registered by the user in SYS_NET_Open<br>

   Returns:	If successful, returns a valid handle to an object. Otherwise, it returns SYS_MODULE_OBJ_INVALID.

   Example:
        <code>

		SYS_NET_Config    	g_NetServCfg;
		SYS_MODULE_OBJ 		g_NetServHandle;

		memset(&g_NetServCfg, 0, sizeof(g_NetServCfg));
		g_NetServCfg.mode = SYS_NET_MODE_CLIENT;
		strcpy(g_NetServCfg.host_name, APP_HOST_NAME);
		g_NetServCfg.port = APP_HOST_PORT;
		g_NetServCfg.enable_tls = 0;
		g_NetServCfg.ip_prot = SYS_NET_IP_PROT_UDP;
		g_NetServHandle = SYS_NET_Open(&g_NetServCfg, NetServCallback, 0);                
        if (g_NetServHandle == SYS_MODULE_OBJ_INVALID)
        {
            // Handle error
        }
        </code>

  Remarks:
        This routine should be called everytime a user wants to open a new NET socket
*/

SYS_MODULE_OBJ SYS_NET_Open(SYS_NET_Config *cfg, SYS_NET_CALLBACK Net_cb, void *cookie);


// *****************************************************************************
/* Function:
   void SYS_NET_Close ( SYS_MODULE_OBJ object )

  Summary:
       Deinitializes the specific instance of the NET System service

  Description:
       This function deinitializes the specific module instance disabling its
       operation. Resets all of the internal data structures and fields for 
	   the specified instance to the default settings.

  Precondition:
       The SYS_NET_Open function should have been called before calling
       this function.

  Parameters:
       object   - SYS NET object handle, returned from SYS_NET_Open<br>

  Returns:
       None.

  Example:
        <code>
        // Handle "objSysNet" value must have been returned from SYS_NET_Open.

        SYS_NET_Close (objSysNet);
        </code>

  Remarks:
       Once the Open operation has been called, the Close operation must be 
	   called before the Open operation can be called again.
*/

void SYS_NET_Close(SYS_MODULE_OBJ);

// *****************************************************************************
/* Function:
    void SYS_NET_Task(SYS_MODULE_OBJ obj)

  Summary:
       Executes the SYS NET service state machine for the instance

   Description:
		This function ensures that the Net system service is able to execute 
		its state machine to process any messages and invoke the user callback 
		for any events.
  
  Precondition:
       SYS_NET_Open should have been called before calling this function

  Parameters:
       obj  - SYS NET object handle, returned from SYS_NET_Open<br><br>

   Returns:
        None

   Example:
        <code>
		// Handle "objSysNet" value must have been returned from SYS_NET_Open.
		while(1)
		{
			...
			SYS_NET_Task(objSysNet);
			...
		}
        </code>

*/

void SYS_NET_Task(SYS_MODULE_OBJ obj);


// *****************************************************************************
/* Function:
       int32_t SYS_NET_CtrlMsg(SYS_MODULE_OBJ obj, SYS_NET_CTRL_MSG msg_type, void *data, uint16_t len)

  Summary:
      Returns success/ failure for the disconnect/ reconnect operation asked by the user.

  Description:
       This function is used for disconnecting or reconnecting to the peer.

  Precondition:
       SYS_NET_Open should have been called.

  Parameters:
       obj  	- SYS NET object handle, returned from SYS_NET_Open<br>
	   msg_type - valid Msg Type SYS_NET_CTRL_MSG<br>
       data		- valid data buffer pointer based on the Msg Type - NULL for DISCONNECT, 
					Pointer to SYS_NET_Config for RECONNECT<br>
	   len		- length of the data buffer the pointer is pointing to<br>

  Returns:
		SYS_NET_SUCCESS - Indicates that the Request was catered to successfully
		SYS_NET_FAILURE - Indicates that the Request failed

  Example:
       <code>
       // Handle "objSysNet" value must have been returned from SYS_NET_Open.	   
		if( SYS_NET_CtrlMsg(objSysNet, SYS_NET_CTRL_MSG_DISCONNECT, NULL, 0) == SYS_NET_SUCCESS)
		{
		}
       </code>

  Remarks:
       None.
  */
int32_t SYS_NET_CtrlMsg(SYS_MODULE_OBJ obj, 
                        SYS_NET_CTRL_MSG msg_type, 
                        void *data, 
                        uint32_t len);


// *****************************************************************************
/* Function:
       int32_t SYS_NET_SetConfigParam(SYS_MODULE_OBJ obj, uint32_t paramType, void *data)

  Summary:
      Returns success on setting a configuration parameter for Net System Service.

  Description:
       This function is currently used for enabling/ disabling the Auto Reconnect feature for the Net Socket.

  Precondition:
       SYS_NET_Open should have been called.

  Parameters:
       obj  	- SYS NET object handle, returned from SYS_NET_Open<br>
	   paramType - Reserved for future use<br>
       data		- 0/ 1 currently used only for enabling/ disabling the auto reconnect feature <br>
	   
  Returns:
		SYS_NET_SUCCESS - Indicates that the Request was catered to successfully

  Example:
       <code>
	   bool auto_reconnect = true;
       // Handle "objSysNet" value must have been returned from SYS_NET_Open.	   
		if( SYS_NET_SetConfigParam(objSysNet, 0, &auto_reconnect)  == SYS_NET_SUCCESS)
		{
		}
       </code>

  Remarks:
       None.
  */
int32_t SYS_NET_SetConfigParam(SYS_MODULE_OBJ obj, 
        uint32_t    paramType,
        void *data);
		
#ifndef SYS_NET_ENABLE_DEBUG_PRINT
#define SYS_NETDEBUG_DBG_PRINT(obj, flow, fmt, ...)
#define SYS_NETDEBUG_INFO_PRINT(obj, flow, fmt, ...)
#define SYS_NETDEBUG_FN_ENTER_PRINT(obj, flow)
#define SYS_NETDEBUG_FN_EXIT_PRINT(obj, flow)
#define SYS_NETDEBUG_ERR_PRINT(obj, flow, fmt, ...) SYS_CONSOLE_Print(SYS_CONSOLE_DEFAULT_INSTANCE, fmt, ##__VA_ARGS__)
#else
#define SYS_NETDEBUG_DBG_PRINT      SYS_APPDEBUG_DBG_PRINT
#define SYS_NETDEBUG_INFO_PRINT     SYS_APPDEBUG_INFO_PRINT
#define SYS_NETDEBUG_FN_ENTER_PRINT     SYS_APPDEBUG_FN_ENTER_PRINT
#define SYS_NETDEBUG_FN_EXIT_PRINT      SYS_APPDEBUG_FN_EXIT_PRINT
#define SYS_NETDEBUG_ERR_PRINT      SYS_APPDEBUG_ERR_PRINT
#endif


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //SYS_NET_H
