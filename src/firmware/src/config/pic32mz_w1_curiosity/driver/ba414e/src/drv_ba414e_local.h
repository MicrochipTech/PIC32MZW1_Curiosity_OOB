/*******************************************************************************
  BA414E Crypto Driver Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_ba414e_local.h

  Summary:
    BA414E Driver Interface Header File

  Description:
    This device driver provides a simple interface to manage the asymmetric 
    cryptography module on Microchip's PIC32MZ1025W104132 family of 
    microcontrollers.  This file provides the interface definition for this 
    driver.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2012-2018 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS  WITHOUT  WARRANTY  OF  ANY  KIND,
EITHER EXPRESS  OR  IMPLIED,  INCLUDING  WITHOUT  LIMITATION,  ANY  WARRANTY  OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A  PARTICULAR  PURPOSE.
IN NO EVENT SHALL MICROCHIP OR  ITS  LICENSORS  BE  LIABLE  OR  OBLIGATED  UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,  BREACH  OF  WARRANTY,  OR
OTHER LEGAL  EQUITABLE  THEORY  ANY  DIRECT  OR  INDIRECT  DAMAGES  OR  EXPENSES
INCLUDING BUT NOT LIMITED TO ANY  INCIDENTAL,  SPECIAL,  INDIRECT,  PUNITIVE  OR
CONSEQUENTIAL DAMAGES, LOST  PROFITS  OR  LOST  DATA,  COST  OF  PROCUREMENT  OF
SUBSTITUTE  GOODS,  TECHNOLOGY,  SERVICES,  OR  ANY  CLAIMS  BY  THIRD   PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE  THEREOF),  OR  OTHER  SIMILAR  COSTS.
*******************************************************************************/
//DOM-IGNORE-END

#ifndef _DRV_BA414E_LOCAL_H_
#define _DRV_BA414E_LOCAL_H_

#include <stdint.h>
#include "configuration.h"
#include "osal/osal.h"
#include "system/system_module.h"


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

// Shared crypto memory base address
#define DRV_BA414E_SCMEM_BASE              __CRYPTO1SCM_BASE
// Shared crypto memory size
#define DRV_BA414E_SCMEM_SIZE              (2432)
// Shared crypto memory end address
#define DRV_BA414E_SCMEM_END               (DRV_BA414E_SCMEM_BASE + DRV_BA414E_SCMEM_SIZE)
// Shared crypto memory maximum slot number
#define DRV_BA414E_MAX_SLOT_NUM            0x20
// Shared crypto memory size (logical shift)
#define DRV_BA414E_SCM_SLOT_LS             6
// Crypto memory alignment requirement mask
#define DRV_BA414E_ALIGNMENT_MASK          0x3
// Microcode memory size
#define DRV_BA414E_MAX_uCODE_SIZE          1432
        
        
        
// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef enum 
{
    DRV_BA414E_INITIALIZE = 0,
    DRV_BA414E_READY,
    DRV_BA414E_PREPARING,
    DRV_BA414E_WAITING,
    DRV_BA414E_PROCESSING        
}DRV_BA414E_STATEs;
        
typedef enum
{
    DRV_BA414E_OP_NONE = 0,
    DRV_BA414E_OP_ECDSA_SIGN,
    DRV_BA414E_OP_ECDSA_VERIFY,
    DRV_BA414E_OP_PRIM_ECC_POINT_DOUBLE,
    DRV_BA414E_OP_PRIM_ECC_POINT_ADDITION,
    DRV_BA414E_OP_PRIM_ECC_POINT_MULTIPLICATION,
    DRV_BA414E_OP_PRIM_ECC_CHECK_POINT_ON_CURVE,
    DRV_BA414E_OP_PRIM_MOD_ADDITION,
    DRV_BA414E_OP_PRIM_MOD_SUBTRACTION,
    DRV_BA414E_OP_PRIM_MOD_MULTIPLICATION,
    DRV_BA414E_OP_PRIM_MOD_EXP,            
            
}DRV_BA414E_OPERATIONS;

typedef struct
{
    const DRV_BA414E_ECC_DOMAIN * domain;
    uint8_t * R;
    uint8_t * S;
    const uint8_t * privateKey;
    const uint8_t * k;
    const uint8_t * msgHash;
    int msgHashSz;    
}DRV_BA414E_ecdsaSignOpParams;

typedef struct
{
    const DRV_BA414E_ECC_DOMAIN * domain;
    uint8_t * R;
    uint8_t * S;
    const uint8_t * publicKeyX;
    const uint8_t * publicKeyY;
    const uint8_t * msgHash;
    int msgHashSz;    
}DRV_BA414E_ecdsaVerifyOpParams;

typedef struct
{
    const DRV_BA414E_ECC_DOMAIN * domain;
    uint8_t * outX;
    uint8_t * outY;
    const uint8_t * p1X;
    const uint8_t * p1Y;

}DRV_BA414E_primEccPointDoubleOpParams;

typedef struct
{
    const DRV_BA414E_ECC_DOMAIN * domain;
    uint8_t * outX;
    uint8_t * outY;
    const uint8_t * p1X;
    const uint8_t * p1Y;
    const uint8_t * p2X;
    const uint8_t * p2Y;

}DRV_BA414E_primEccPointAdditionOpParams;

typedef struct
{
    const DRV_BA414E_ECC_DOMAIN * domain;
    uint8_t * outX;
    uint8_t * outY;
    const uint8_t * p1X;
    const uint8_t * p1Y;
    const uint8_t * k;

}DRV_BA414E_primEccPointMultiplicationOpParams;

typedef struct
{
    const DRV_BA414E_ECC_DOMAIN * domain;
    const uint8_t * p1X;
    const uint8_t * p1Y;

}DRV_BA414E_primEccCheckPointOnCurveOpParams;

typedef struct
{
    DRV_BA414E_OPERAND_SIZE opSize;    // Hardware operand size
    uint8_t * c;
    const uint8_t * p;
    const uint8_t * a;
    const uint8_t * b;
}DRV_BA414E_primModOperationParams;

typedef struct
{
    DRV_BA414E_OPERAND_SIZE opSize;    // Hardware operand size
    uint8_t * C;
    const uint8_t * n;
    const uint8_t * M;
    const uint8_t * e;
}DRV_BA414E_primModExpOpParams;


typedef struct
{
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
    OSAL_SEM_DECLARE(clientBlock);
#endif
    union {
        DRV_BA414E_ecdsaSignOpParams ecdsaSignParams;
        DRV_BA414E_ecdsaVerifyOpParams ecdsaVerifyParams;
        DRV_BA414E_primEccPointDoubleOpParams eccPointDoubleParams;
        DRV_BA414E_primEccPointAdditionOpParams eccPointAdditionParams;
        DRV_BA414E_primEccPointMultiplicationOpParams eccPointMultiplicationParams;
        DRV_BA414E_primEccCheckPointOnCurveOpParams eccCheckPointOnCurveParams;
        DRV_BA414E_primModOperationParams modOperationParams;
        DRV_BA414E_primModExpOpParams modExpParams;        
    };
    DRV_BA414E_CALLBACK callback;
    uintptr_t context;
    DRV_BA414E_OP_RESULT blockingResult;

    
    DRV_IO_INTENT ioIntent;
    DRV_BA414E_OPERATIONS currentOp : 8;
    uint8_t inUse;
}DRV_BA414E_ClientData;
        
typedef struct 
{
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
    OSAL_SEM_DECLARE(clientListSema);
    OSAL_SEM_DECLARE(clientAction);
    OSAL_SEM_DECLARE(wfi);
#endif
    DRV_BA414E_ClientData * currentClient;
    SYS_STATUS status : 8;
    DRV_BA414E_STATEs state : 8;
    uint8_t inited;
    uint8_t doneInterrupt;
    uint8_t errorInterrupt;
    uint32_t lastStatus;
}DRV_BA414E_OperationalData;
        


typedef enum
{
    BA414E_OPSZ_128 = 2,
    BA414E_OPSZ_192 = 3,
    BA414E_OPSZ_256 = 4,
    BA414E_OPSZ_320 = 5,
    BA414E_OPSZ_384 = 6,
    BA414E_OPSZ_448 = 7,
    BA414E_OPSZ_512 = 8,
} BA414E_OP_SIZE;

typedef enum
{
    BA414E_OPC_PRIM_MOD_ADD = 0x01,
    BA414E_OPC_PRIM_MOD_SUB = 0x02,
    BA414E_OPC_PRIM_MOD_MULT = 0x03,
    BA414E_OPC_PRIM_MOD_RED_ODD_N = 0x04,
    BA414E_OPC_PRIM_MOD_DIV = 0x05,
    BA414E_OPC_PRIM_MOD_INV_ODD_N = 0x06,
    BA414E_OPC_PRIM_MULT = 0x08,
    BA414E_OPC_PRIM_MOD_INV_EVEN_N = 0x09,
    BA414E_OPC_RSA_MOD_EXP = 0x10,
    BA414E_OPC_RSA_PRIV_KEY_GEN = 0x11,
    BA414E_OPC_CRT_KEY_PARAM_GEN = 0x12,
    BA414E_OPC_CRT_DECRYPT = 0x13,
    BA414E_OPC_RSA_ENCRYPT = 0x14,
    BA414E_OPC_RSA_DECRYPT = 0x15,
    BA414E_OPC_RSA_SIGN = 0x16,
    BA414E_OPC_RSA_VERIFY = 0x17,
    BA414E_OPC_DSA_KEY_GEN = 0x18,
    BA414E_OPC_DSA_SIGN = 0x19,
    BA414E_OPC_DSA_VERIFY = 0x1A,
    BA414E_OPC_SRP_CLIENT_SESSION_KEY_GEN = 0x1C,
    BA414E_OPC_PRIM_ECC_POINT_DOUBLE = 0x20,           
    BA414E_OPC_PRIM_ECC_POINT_ADDITION = 0x21,           
    BA414E_OPC_PRIM_ECC_POINT_MULTI = 0x22,           
    BA414E_OPC_PRIM_ECC_POINT_CHECK_AB = 0x23,           
    BA414E_OPC_PRIM_ECC_POINT_CHECK_N = 0x24,           
    BA414E_OPC_PRIM_ECC_POINT_CHECK_COUPLE_LESS_PRIME = 0x25,           
    BA414E_OPC_PRIM_ECC_POINT_CHECK_POINT_ON_CURVE = 0x26,           
    BA414E_OPC_PRIM_ECC_POINT_CURVE25519_POINT_MULT = 0x28,           
    BA414E_OPC_PRIM_ECC_POINT_ED25519_XRECOVER = 0x29,           
    BA414E_OPC_PRIM_ECC_POINT_ED25519_SCALARMULT = 0x2A,           
    BA414E_OPC_PRIM_ECC_POINT_ED25519_CHECKVALID = 0x2B,           
    BA414E_OPC_PRIM_ECC_POINT_ED25519_CHECK_POINT_ON_CURVE = 0x2C,           
    BA414E_OPC_ECC_ECDSA_SIGN = 0x30,
    BA414E_OPC_ECC_ECDSA_VERIFY = 0x31,
    BA414E_OPC_ECC_ECDSA_DOMAIN_PARAM_VALID = 0x32,
    BA414E_OPC_ECC_ECKCDSA_PUBLIC_KEY_GEN = 0x33,
    BA414E_OPC_ECC_ECKCDSA_SIGN = 0x34,
    BA414E_OPC_ECC_ECKCDSA_VERIFY = 0x35,
} BA414E_OP_CODES;

typedef enum
{
    BA414E_ECDSA_SLOT_P = 0,
    BA414E_ECDSA_SLOT_N = 1,
    BA414E_ECDSA_SLOT_GX = 2,
    BA414E_ECDSA_SLOT_GY = 3,
    BA414E_ECDSA_SLOT_A = 4,
    BA414E_ECDSA_SLOT_B = 5,
    BA414E_ECDSA_SLOT_PRIV_KEY = 6,
    BA414E_ECDSA_SLOT_K = 7,
    BA414E_ECDSA_SLOT_X0 = 8,
    BA414E_ECDSA_SLOT_Y0 = 9,
    BA414E_ECDSA_SLOT_R = 0xA,
    BA414E_ECDSA_SLOT_S = 0xB,
    BA414E_ECDSA_SLOT_H = 0xC,
    BA414E_ECDSA_SLOT_W = 0xD,
    BA414E_ECDSA_SLOT_P1X = 0xE,
    BA414E_ECDSA_SLOT_P1Y = 0xF,           
} BA414E_ECDSA_SLOTS;

typedef enum
{
    BA414E_ECCP_SLOT_P = 0,
    BA414E_ECCP_SLOT_N = 1,
    BA414E_ECCP_SLOT_GX = 2,
    BA414E_ECCP_SLOT_GY = 3,
    BA414E_ECCP_SLOT_A = 4,
    BA414E_ECCP_SLOT_B = 5,
    BA414E_ECCP_SLOT_P1X = 6,
    BA414E_ECCP_SLOT_P1Y = 7,
    BA414E_ECCP_SLOT_P2X = 8,
    BA414E_ECCP_SLOT_P2Y = 9,
    BA414E_ECCP_SLOT_P3X = 0xA,
    BA414E_ECCP_SLOT_P3Y = 0xB,
    BA414E_ECCP_SLOT_K = 0xC,
} BA414_ECC_PRIM_SLOTS;

typedef enum
{
    BA414E_MODP_SLOT_P = 0,
    BA414E_MODP_SLOT_A = 1,
    BA414E_MODP_SLOT_B = 2,
    BA414E_MODP_SLOT_C = 3,
} BA414E_MODP_PRIM_SLOTS;

typedef enum
{
    BA414E_RSA_MODEXP_n = 0,
    BA414E_RSA_MODEXP_e = 1,
    BA414E_RSA_MODEXP_M = 2,
    BA414E_RSA_MODEXP_C = 3,
} BA414E_RSA_MODEXP_SLOTS;


typedef union {
    __PKCOMMANDbits_t s;
    uint32_t v;
}BA414E_PKCOMMANDbits;

typedef union {
    __PKSTATUSbits_t s;
    uint32_t v;
}BA414E__PKSTATUSbits;

typedef union {
    __PKCONFIGbits_t s;
    uint32_t v;
}BA414E__PKCONFIGbits;

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END



#endif
