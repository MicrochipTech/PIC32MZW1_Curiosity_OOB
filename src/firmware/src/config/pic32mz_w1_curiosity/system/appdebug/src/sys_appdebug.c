/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

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

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "../sys_appdebug.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

//Debug Library
#include "system_config.h"

#define SYS_APPDEBUG_PRINT_BUFFER_SIZE  512
static OSAL_SEM_HANDLE_TYPE     g_SysAppDebugSemaphore;	/* Semaphore for Critical Section */
SYS_APPDEBUG_HANDLE g_asSysAppDebugHandle[SYS_APPDEBUG_MAX_NUM_OF_USERS];
static char printBuff[SYS_APPDEBUG_PRINT_BUFFER_SIZE] SYS_CMD_BUFFER_DMA_READY;
static int printBuffPtr = 0;


void* SYS_APPDEBUG_AllocHandle()
{
	uint8_t i = 0;

	OSAL_SEM_Pend(&g_SysAppDebugSemaphore, OSAL_WAIT_FOREVER);
	
	for(i = 0; i < SYS_APPDEBUG_MAX_NUM_OF_USERS; i++)
	{
		if(g_asSysAppDebugHandle[i].entryValid == false)
		{
            g_asSysAppDebugHandle[i].entryValid = true;
			OSAL_SEM_Post(&g_SysAppDebugSemaphore);
			return &g_asSysAppDebugHandle[i];
		}
	}
    OSAL_SEM_Post(&g_SysAppDebugSemaphore);
    return NULL;
}

void SYS_APPDEBUG_FreeHandle(SYS_MODULE_OBJ obj)
{
	SYS_APPDEBUG_HANDLE	*hdl = (SYS_APPDEBUG_HANDLE *)obj;

	OSAL_SEM_Pend(&g_SysAppDebugSemaphore, OSAL_WAIT_FOREVER);
    hdl->entryValid = false;
    OSAL_SEM_Post(&g_SysAppDebugSemaphore);
}

SYS_MODULE_OBJ  SYS_APPDEBUG_Open(SYS_APPDEBUG_CONFIG *cfg)
{
    SYS_APPDEBUG_HANDLE  	*hdl = NULL;    	
    
    if(cfg->logLevel > 0xf)
    {
        SYS_CONSOLE_MESSAGE("APPDEBUG_SRVC: Invalid :LogLevel\r\n");
        return SYS_MODULE_OBJ_INVALID;
    }
    
	/* 
	** Allocate Handle - hdl 
	*/
	hdl = SYS_APPDEBUG_AllocHandle();
    if(hdl == NULL)
    {
        SYS_CONSOLE_MESSAGE("APPDEBUG_SRVC: Failed to allocate Handle\r\n");
        return SYS_MODULE_OBJ_INVALID;        
    }
    memcpy(&hdl->cfgInfo, cfg, sizeof(SYS_APPDEBUG_CONFIG));
    
    return (SYS_MODULE_OBJ)hdl;
}

void  SYS_APPDEBUG_Close(SYS_MODULE_OBJ obj)
{
    SYS_APPDEBUG_FreeHandle(obj);
}

void SYS_APPDEBUG_PRINT(SYS_MODULE_OBJ obj, uint32_t flow, uint32_t level, const char *function, uint32_t linenum, char *msg, ...)
{
	SYS_APPDEBUG_HANDLE	*hdl = (SYS_APPDEBUG_HANDLE *)obj;
    if(obj == SYS_MODULE_OBJ_INVALID)
        return ;

    if((flow & hdl->cfgInfo.logFlow ) && (level & hdl->cfgInfo.logLevel))
    {
        char tmpBuf[SYS_APPDEBUG_PRINT_BUFFER_SIZE] = {0};
        int len = 0;
        size_t padding = 0;
        const char  *levelStr[3] = {"ERR", "DBG", "INFO"};
        va_list args;

        SYS_CONSOLE_PRINT("%s: (%d)(%s)(%d)(%s) ", 
                hdl->cfgInfo.prefixString, 
                SYS_TMR_TickCountGet(),  
                function, 
                linenum, 
                levelStr[level>>1]);

        va_start( args, msg );
        len = vsnprintf(tmpBuf, SYS_APPDEBUG_PRINT_BUFFER_SIZE, msg, args);
        va_end( args );

        if (len > 0 && len < SYS_APPDEBUG_PRINT_BUFFER_SIZE)
        {
            tmpBuf[len] = '\0';

            if (len + printBuffPtr >= SYS_APPDEBUG_PRINT_BUFFER_SIZE)
            {
                printBuffPtr = 0;
            }

            strcpy(&printBuff[printBuffPtr], tmpBuf);
            SYS_CONSOLE_MESSAGE(&printBuff[printBuffPtr]);
            padding = len % 4;

            if (padding > 0)
            {
                padding = 4 - padding;
            }

            printBuffPtr += len + padding;
        }
    }
}

void SYS_APPDEBUG_PRINT_FN_ENTER(SYS_MODULE_OBJ obj, uint32_t flow, const char *function, uint32_t linenum)
{
	SYS_APPDEBUG_HANDLE	*hdl = (SYS_APPDEBUG_HANDLE *)obj;
    if(obj == SYS_MODULE_OBJ_INVALID)
        return ;

    if((flow & hdl->cfgInfo.logFlow ) && (APP_LOG_FN_EE_LVL & hdl->cfgInfo.logLevel))
    {
        SYS_CONSOLE_PRINT("%s: (%d)(%s)(%d): Enter\r\n", hdl->cfgInfo.prefixString, 
                SYS_TMR_TickCountGet(),  function, linenum);
    }
}

void SYS_APPDEBUG_PRINT_FN_EXIT(SYS_MODULE_OBJ obj, uint32_t flow, const char *function, uint32_t linenum)
{
	SYS_APPDEBUG_HANDLE	*hdl = (SYS_APPDEBUG_HANDLE *)obj;
    if(obj == SYS_MODULE_OBJ_INVALID)
        return ;

    if((flow & hdl->cfgInfo.logFlow ) && (APP_LOG_FN_EE_LVL & hdl->cfgInfo.logLevel))
    {
        SYS_CONSOLE_PRINT("%s: (%d)(%s)(%d): Exit\r\n", hdl->cfgInfo.prefixString, 
                SYS_TMR_TickCountGet(),  function, linenum);
    }
}

bool SYS_APPDEBUG_Command_Process(int argc,char *argv[])
{
	bool ret =0 ;

	/*Example command: sysappdebug set flow 0x4*/
    if((argc >= 2) && (!strcmp((char*)argv[1],"set")))
    {
        /*Set the required config to ConfigData
          * NOTE: The content set here will be save on "save" command to NVM
          * Applied to MAC on "apply" command*/
        if(((argv[2] == NULL)) || (!strcmp("?",argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysappdebug set level <level_value>");
            SYS_CONSOLE_PRINT("\n\r\t* sysappdebug set level ? :: Get level options");
        }
        else if ((argv[3] == NULL))
            SYS_CONSOLE_PRINT("\n\rArguments Missing");
        else if(!strcmp("level",argv[2]))
        {
            if(!strcmp("?",argv[3]))
            {
                SYS_CONSOLE_PRINT("\n\rsysappdebug set level <VALUE>");
                SYS_CONSOLE_PRINT("\n\rVALUE : is a valid bitmask of DISABLE(0), ERR(1), DBG(2), INFO(3), FN_EE(4)");
                return 0;
            }
            else
            {
                int hdlCnt = 0;
                int temp = strtoul( argv[3], 0, 16);
                
                SYS_CONSOLE_PRINT("\n\rLevel is 0x%x", temp);
            	OSAL_SEM_Pend(&g_SysAppDebugSemaphore, OSAL_WAIT_FOREVER);
                for(hdlCnt = 0; hdlCnt < SYS_APPDEBUG_MAX_NUM_OF_USERS; hdlCnt++)
                {
                    if(g_asSysAppDebugHandle[hdlCnt].entryValid == true)
                        g_asSysAppDebugHandle[hdlCnt].cfgInfo.logLevel = temp;
                }
                OSAL_SEM_Post(&g_SysAppDebugSemaphore);
            }    
        }
	}
    else if((argc >= 2) && (!strcmp((char*)argv[1],"get")))
    {
        if((argv[2] == NULL) || (!strcmp("?",argv[2])))
        {
            SYS_CONSOLE_PRINT("\n\rFollowing commands supported");
            SYS_CONSOLE_PRINT("\n\r\t* sysappdebug get info");            
            return 0;
        }        
        else if(!strcmp((char*)argv[2],"info"))
        {
            int hdlCnt = 0;
            for(hdlCnt = 0; hdlCnt < SYS_APPDEBUG_MAX_NUM_OF_USERS; hdlCnt++)
            {
                if(g_asSysAppDebugHandle[hdlCnt].entryValid == true)
                {
            		SYS_CONSOLE_PRINT("[%i] Level = 0x%x ; Flow = 0x%x\r\n", hdlCnt,
                            g_asSysAppDebugHandle[hdlCnt].cfgInfo.logLevel, 
                            g_asSysAppDebugHandle[hdlCnt].cfgInfo.logFlow);                    
                }
            }
            return 0;
        }
    }    
    else if((argc >= 2) && (!strcmp((char*)argv[1],"read")))
    {
        if((argv[2] == NULL) || (!strcmp((char*)argv[2],"?")))
        {
            SYS_CONSOLE_MESSAGE("\n\rFollowing read Command:");
            SYS_CONSOLE_MESSAGE("\n\r\t* sysappdebug read memory");
            return 0;
        }        
        else if((!strcmp((char*)argv[2],"memory")))
        {
			if((argv[3] == NULL) || (!strcmp((char*)argv[3],"?")))
			{
				SYS_CONSOLE_MESSAGE("\n\rFollowing read Command:");
				SYS_CONSOLE_MESSAGE("\n\r\t* sysappdebug read memory <VALUE> <TYPE> <LEN>");
				SYS_CONSOLE_MESSAGE("\n\r\t* VALUE: The memory address from which to read");
				SYS_CONSOLE_MESSAGE("\n\r\t* TYPE: The value type to be read -  - uint32_t (1), uin16_t (2), uint8_t(3), string(4)");
				SYS_CONSOLE_MESSAGE("\n\r\t* LEN: Length of the bytes to be read - Valid only for TYPE 4");
				SYS_CONSOLE_MESSAGE("\n\r\t* sysappdebug read memory 0x8001ab6c 1");
				SYS_CONSOLE_MESSAGE("\n\r\t* sysappdebug read memory 0x8001ab6c 4 9");
				return 0;
			}
			else
			{
                unsigned char type = strtoul( argv[4], 0, 10);
                switch(type)
                {
                    case 1:
                    {
                        uint32_t *mem_addr = (uint32_t *)strtoul( argv[3], 0, 16);
                        SYS_CONSOLE_PRINT("0x%x", *mem_addr);                    
                    }
                    break;

                    case 2:
                    {
                        uint16_t *mem_addr = (uint16_t *)strtoul( argv[3], 0, 16);
                        SYS_CONSOLE_PRINT("0x%x", *mem_addr);                    
                    }
                    break;

                    case 3:
                    {
                        uint8_t *mem_addr = (uint8_t *)strtoul( argv[3], 0, 16);
                        SYS_CONSOLE_PRINT("0x%x", *mem_addr);                    
                    }
                    break;

                    case 4:
                    {
                        uint8_t *mem_addr = (uint8_t *)strtoul( argv[3], 0, 16);
                        int len = strtoul( argv[5], 0, 10);
                        int cnt = 0;
                        for (cnt = 0; cnt < len; cnt++)
                        {
                            SYS_CONSOLE_PRINT("0x%x ", mem_addr[cnt]);
                        }
                    }
                    break;

                }
                SYS_CONSOLE_MESSAGE("\r\n");
                return 0;
			}
        }
    }
    else {
		SYS_CONSOLE_MESSAGE("*** Command Processor: unknown command. ***");
		ret = 1;
	}

	return ret;
}

void SYS_APPDEBUG_CMDProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
	
	SYS_APPDEBUG_Command_Process(argc,argv);
	return;
}

void SYS_APPDEBUG_CMDHelp(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv){
	
	SYS_CONSOLE_PRINT("SysAppDebug commands:\r\n");
    SYS_CONSOLE_PRINT("\n1) sysappdebug set ?\t\t-- To get list of sysappdebug set commands\t\r\n");
    SYS_CONSOLE_PRINT("\n2) sysappdebug get ?\t\t-- To get list of sysappdebug get commands like flow, level\r\n");
    SYS_CONSOLE_PRINT("\n2) sysappdebug read ?\t\t-- To get list of sysappdebug command to read from memory address\r\n");   
	return;
}


static const SYS_CMD_DESCRIPTOR    SysAppDebugCmdTbl[]=
{
    {"sysappdebug",     SYS_APPDEBUG_CMDProcessing,              ": SysAppDebug commands processing"},
    {"sysappdebughelp",  SYS_APPDEBUG_CMDHelp,			  		 ": SysAppDebug commands help "},
};

SYS_MODULE_OBJ SYS_APPDEBUG_Initialize( const SYS_MODULE_INDEX index,
                                   const SYS_MODULE_INIT * const init )
{
	/* 
	** Create Semaphore for ensuring the SYS NET APIs are re-entrant 
	*/
    if(OSAL_SEM_Create(&g_SysAppDebugSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1) != OSAL_RESULT_TRUE)
    {
        SYS_CONSOLE_MESSAGE("APPDEBUG_SRVC: Failed to Initialize Service as Semaphore NOT created\r\n");
        return SYS_APPDEBUG_FAILURE;        
    }
    
    if (!SYS_CMD_ADDGRP(SysAppDebugCmdTbl, sizeof(SysAppDebugCmdTbl)/sizeof(*SysAppDebugCmdTbl), "sysappdebug", ": APP Debug commands"))
    {
        SYS_ERROR(SYS_ERROR_ERROR, "Failed to create SysAppDebug Commands\r\n", 0);
		return false;
    }	
    return SYS_MODULE_OBJ_STATIC;
}

int32_t	SYS_APPDEBUG_CtrlMsg(SYS_MODULE_OBJ obj, SYS_APPDEBUG_CtrlMsgType eCtrlMsgType, void *data, uint16_t len)
{
	SYS_APPDEBUG_HANDLE	*hdl = (SYS_APPDEBUG_HANDLE *)obj;
    switch(eCtrlMsgType)
    {
        case SYS_APPDEBUG_CTRL_MSG_TYPE_SET_LEVEL:
        {
            hdl->cfgInfo.logLevel = *((uint32_t *)data);
        }
        break;
        
        case SYS_APPDEBUG_CTRL_MSG_TYPE_SET_FLOW:
        {
            hdl->cfgInfo.logFlow = *((uint32_t *)data);            
        }
        break;
    }
    return SYS_APPDEBUG_SUCCESS;
}
/*******************************************************************************
 End of File
 */
