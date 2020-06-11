/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
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

#ifndef SYS_APPDEBUG_H
#define SYS_APPDEBUG_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END 


#ifdef SYS_APPDEBUG_ENABLE
//Debug Library 

#define APP_LOG_LVL_DISABLE 	0x0
#define APP_LOG_ERROR_LVL 		0x1
#define APP_LOG_DBG_LVL 		0x2
#define APP_LOG_INFO_LVL 		0x4
#define APP_LOG_FN_EE_LVL 		0x8

// DOM-IGNORE-END


// *****************************************************************************
/* SYS Debug Module Index Number

  Summary:
    Debug System Service index.

  Description:
    This constant defines a symbolic name for the debug system service index.

  Remarks:
    There can only be a single debug system service instance in the system.
*/

#define SYS_APPDEBUG_MAX_NUM_OF_USERS       8
#define SYS_APPDEBUG_MAX_MSG_SIZE           512


// *****************************************************************************
/* SYS Debug Initialize structure

  Summary:
    Defines the data required to initialize the debug system service.

  Description:
    This structure defines the data required to initialize the debug system 
    service.

  Remarks:
    None.
*/

typedef struct
{
    /* Initial system Log level setting. */
    unsigned int                 logLevel;

    /* Initial system Log level setting. */
    unsigned int                 logFlow;

    /* Initial system Log level setting. */
    const char                        *prefixString;
} SYS_APPDEBUG_CONFIG;

typedef struct
{
    SYS_APPDEBUG_CONFIG     cfgInfo;
    bool                    entryValid;
} SYS_APPDEBUG_HANDLE;

typedef enum {
	SYS_APPDEBUG_CTRL_MSG_TYPE_SET_LEVEL,
	SYS_APPDEBUG_CTRL_MSG_TYPE_SET_FLOW,
} SYS_APPDEBUG_CtrlMsgType;

typedef enum {    
	SYS_APPDEBUG_SUCCESS = 0,	///< Success
	SYS_APPDEBUG_FAILURE = -1,    ///< Failure
} SYS_APPDEBUG_RESULT;

extern SYS_APPDEBUG_HANDLE g_asSysAppDebugHandle[SYS_APPDEBUG_MAX_NUM_OF_USERS];

SYS_MODULE_OBJ SYS_APPDEBUG_Initialize( const SYS_MODULE_INDEX index,
                                   const SYS_MODULE_INIT * const init );
SYS_MODULE_OBJ  SYS_APPDEBUG_Open(SYS_APPDEBUG_CONFIG *cfg);
void  SYS_APPDEBUG_Close(SYS_MODULE_OBJ obj);
void SYS_APPDEBUG_PRINT(SYS_MODULE_OBJ obj, 
                        uint32_t flow, 
                        uint32_t level, 
                        const char *function, 
                        uint32_t linenum, 
                        char *msg, ...);
void SYS_APPDEBUG_PRINT_FN_ENTER(SYS_MODULE_OBJ obj, 
                        uint32_t flow, 
                        const char *function, 
                        uint32_t linenum);
void SYS_APPDEBUG_PRINT_FN_EXIT(SYS_MODULE_OBJ obj, 
                        uint32_t flow, 
                        const char *function, 
                        uint32_t linenum);
int32_t	SYS_APPDEBUG_CtrlMsg(SYS_MODULE_OBJ hdl, SYS_APPDEBUG_CtrlMsgType eCtrlMsgType, void *data, uint16_t len);

#define SYS_APPDEBUG_ERR_PRINT(obj, flow, fmt, ...)      SYS_APPDEBUG_PRINT(obj, flow, APP_LOG_ERROR_LVL, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define SYS_APPDEBUG_DBG_PRINT(obj, flow, fmt, ...)      SYS_APPDEBUG_PRINT(obj, flow, APP_LOG_DBG_LVL, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define SYS_APPDEBUG_INFO_PRINT(obj, flow, fmt, ...)     SYS_APPDEBUG_PRINT(obj, flow, APP_LOG_INFO_LVL, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define SYS_APPDEBUG_FN_ENTER_PRINT(obj, flow)       SYS_APPDEBUG_PRINT_FN_ENTER(obj, flow, __FUNCTION__, __LINE__)

#define SYS_APPDEBUG_FN_EXIT_PRINT(obj, flow)    SYS_APPDEBUG_PRINT_FN_EXIT(obj, flow, __FUNCTION__, __LINE__)
#else
#define SYS_APPDEBUG_DBG_PRINT(obj, flow, fmt, ...)

#define SYS_APPDEBUG_INFO_PRINT(obj, flow, fmt, ...)

#define SYS_APPDEBUG_FN_ENTER_PRINT(obj, flow)

#define SYS_APPDEBUG_FN_EXIT_PRINT(obj, flow)

#define SYS_APPDEBUG_ERR_PRINT(obj, flow, fmt, ...) SYS_CONSOLE_Print(SYS_CONSOLE_DEFAULT_INSTANCE, fmt, ##__VA_ARGS__)

#define SYS_APPDEBUG_Open(cfg) 

#define SYS_APPDEBUG_Close(obj)
#endif

        
//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* SYS_APP_DEBUG_H */


/*******************************************************************************
 End of File
 */

