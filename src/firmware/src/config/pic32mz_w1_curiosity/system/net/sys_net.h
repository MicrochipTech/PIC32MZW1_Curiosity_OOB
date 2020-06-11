// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
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

#define SYS_NET_MAX_NUM_OF_SOCKETS	2		// Number of Instances Supported by the NET System Service
#define SYS_NET_MODE_CLIENT	0				// Client Mode Value
#define SYS_NET_MODE_SERVER	1				// Server Mode Value
#define SYS_NET_MAX_HOSTNAME_LEN   64    	// Max Host Name Length

#define SYS_NET_IP_PROT_UDP     0			// TCP - Ip Protocol Value
#define SYS_NET_IP_PROT_TCP     1			// UDP - Ip Protocol Value

#define SYS_NET_DEFAULT_TLS_ENABLE      0	// TLS Disabled by default
#define SYS_NET_DEFAULT_AUTO_RECONNECT  1	// Auto Reconnect Enabled by default
#define SYS_NET_DEFAULT_NET_INTF        0	// Interface 0 by default

/* App Debug Print Flows */
#define NET_CFG         0x1
#define NET_DATA        0x2
        
// *****************************************************************************
/* System NET Instance Configuration

  Summary:
    Used for passing on the configuration related to the Net Socket that needs 
	to be opened via the Sys Net Service

  Remarks:
    None.
*/
typedef struct {
	uint8_t mode; // Net Socket Mode to Open - SYS_NET_MODE_CLIENT(0)/  SYS_NET_MODE_SERVER(1) 
	uint8_t	intf; // WiFi or Eth Interface to be used for Opening the socket
	uint16_t port; // Net Server Port
	bool enable_reconnect; // Reconnect in case of disconnection happening - 1(Reconnect Enabled)/ 0(Reconnect Disabled)
    bool enable_tls; // Net Socket with 1(TLS Enabled)/ 0(TLS Disabled)
    uint8_t ip_prot; // Socket IP Protocol - SYS_NET_IP_PROT_UDP(0) or SYS_NET_IP_PROT_TCP(1)
	char	host_name[SYS_NET_MAX_HOSTNAME_LEN]; // Host Name - could have the server name or IP
} SYS_NET_Config;

extern const SYS_NET_Config g_sSysNetConfig; //Data structure which has the MHC configuration for the NET service. 


// *****************************************************************************
/* Function:
    void ( * SYS_NET_CALLBACK ) (uint32_t event, void *data, void* cookie)

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
	event	- An event (SYS_NET_EVENT) for which the callback was called.
	data	- Data (if any) related to the Event
    cookie  - A context value, returned untouched to the client when the
                 callback occurs.
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
/* System NET Instance Status

  Summary:
    Identifies the current status of the Sys Net Instance.

  Remarks:
    None.
*/
typedef enum {
    SYS_NET_STATUS_IDLE = 0, 			// Net Instance is Idle/ Not in Use
    SYS_NET_STATUS_LOWER_LAYER_DOWN, 	// Lower Layer is Down
    SYS_NET_STATUS_RESOLVING_DNS, 		// Resolving DNS of NET Server for the Client to connect
    SYS_NET_STATUS_DNS_RESOLVED, 		// Net Server IP Available for the Client to connect
    SYS_NET_STATUS_SERVER_AWAITING_CONNECTION, // Net Server Awaiting Connection
    SYS_NET_STATUS_CLIENT_CONNECTING, // Net Client connecting to Server
#ifdef SYS_NET_TLS_ENABLED	
    SYS_NET_STATUS_WAIT_FOR_SNTP, // Net Client Waiting for SNTP Time Stamp
    SYS_NET_STATUS_TLS_NEGOTIATING, // Net Client Starting TLS Negotiations
	SYS_NET_STATUS_TLS_NEGOTIATION_FAILED, // Net Instance TLS Negotiation Failed
#endif	
    SYS_NET_STATUS_CONNECTED, // Net Instance connected to the peer
	SYS_NET_STATUS_SOCK_OPEN_FAILED, // Net Instance Failed to open socket
	SYS_NET_STATUS_DNS_RESOLVE_FAILED, // Net Instance Failed to Resolve DNS
    SYS_NET_STATUS_DISCONNECTED, // Net Instance in disconnected state
    SYS_NET_STATUS_PEER_SENT_FIN, // Net Instance received FIN from peer
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
/* Function:
    SYS_MODULE_OBJ SYS_NET_Open (SYS_NET_Config *cfg, 
										SYS_NET_CALLBACK Net_cb, 
										void *cookie)

   Summary:
        Initializes the System NET service.

   Description:
        This function initializes the instance of the System NET Service.

   Parameters:
       cfg    		- Configuration for which the NET Socket needs to be opened

       Net_cb     	- Function pointer to the Callback to be called in case of an event
	   
	   cookie		- Cookie passed as one of the params in the Callback which was registered by the user in SYS_NET_Open

   Returns:
        If successful, returns a valid handle to an object. Otherwise, it
        returns SYS_MODULE_OBJ_INVALID.

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
       Deinitializes the specific module instance of the SYS NET service

  Description:
       This function deinitializes the specific module instance disabling its
       operation. Resets all of the internal
       data structures and fields for the specified instance to the default settings.

  Precondition:
       The SYS_NET_Open function should have been called before calling
       this function.

  Parameters:
       object   - SYS NET object handle, returned from SYS_NET_Open

  Returns:
       None.

  Example:
        <code>
        // Handle "objSysNet" value must have been returned from SYS_NET_Open.

        SYS_NET_Close (objSysNet);
        </code>

  Remarks:
       Once the Initialize operation has been called, the De-initialize
       operation must be called before the Initialize operation can be called
       again.
*/

void SYS_NET_Close(SYS_MODULE_OBJ);

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
       object  - SYS NET object handle, returned from SYS_NET_Open

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
/* Function:
    void SYS_NET_Task(SYS_MODULE_OBJ obj)

   Description:
		This function ensures that the Networking service is able to execute its state machine to process any messages and invoke the user callback for any events.
  
  Precondition:
       SYS_NET_Open should have been called before calling this function

  Parameters:
       obj  - SYS NET object handle, returned from SYS_NET_Open

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
       int32_t SYS_NET_SendMsg(SYS_MODULE_OBJ obj, uint8_t *buffer, uint16_t len)

  Summary:
      Returns No of Bytes sent to peer using the System NET instance.

  Description:
       This function returns the number of bytes transmitted to the peer.

  Precondition:
       SYS_NET_Open should have been called.

  Parameters:
       object  	- SYS NET object handle, returned from SYS_NET_Open
	   
	   data		- valid data buffer pointer
	   
	   len		- length of the data to be transmitted

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
       obj  	- SYS NET object handle, returned from SYS_NET_Open
	   
	   data		- valid data buffer pointer
	   
	   len		- length of the data to be transmitted

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
       int32_t SYS_NET_CtrlMsg(SYS_MODULE_OBJ obj, void *data, uint16_t len)

  Summary:
      Returns success/ failure for the disconnect/ reconnect operation asked by the user.

  Description:
       This function is used for disconnecting or reconnecting to the peer.

  Precondition:
       SYS_NET_Open should have been called.

  Parameters:
       obj  	- SYS NET object handle, returned from SYS_NET_Open
	   
	   msg_type - valid Msg Type - SYS_NET_CTRL_MSG
      
       data		- valid data buffer pointer based on the Msg Type - NULL for DISCONNECT, Pointer to SYS_NET_Config for RECONNECT
	   
	   len		- length of the data buffer the pointer is pointing to

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
       This function is not to be called by the Application code.
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
  */
void SYS_NET_Deinitialize();

int32_t SYS_NET_SetConfigParam(SYS_MODULE_OBJ obj, 
        uint32_t    paramType,
        void *data);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //SYS_NET_H
