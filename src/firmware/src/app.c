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

#include "app.h"
#include "app_commands.h"
#include <wolfssl/ssl.h>
#include <tcpip/src/hash_fnv.h>
#include "system/debug/sys_debug.h"

void *APP_Calloc(size_t num, size_t size) {
    void *p = NULL;

    if (num != 0 && size != 0) {
        p = OSAL_Malloc(size * num);

        if (p != NULL) {
            memset(p, 0, size * num);
        }
    }
    return p;
}

void APP_Initialize(void) {
    APP_Commands_Init();
}

void APP_Tasks(void) {
    return;
}
/*******************************************************************************
 End of File
 */

