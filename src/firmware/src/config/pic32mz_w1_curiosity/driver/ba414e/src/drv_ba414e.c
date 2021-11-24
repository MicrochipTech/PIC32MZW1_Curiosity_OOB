/********************************************************************************
  BA414E Crypto Driver Dynamic implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    drv_ba414e.c

  Summary:
    Source code for the BA414E Crypto driver dynamic 
    implementation.

  Description:
    This file contains the source code for the dynamic implementation of the
    BA414E Crypto driver.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2018 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute Software
only when embedded on a Microchip microcontroller or digital  signal  controller
that is integrated into your product or third party  product  (pursuant  to  the
sublicense terms in the accompanying license agreement).

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdlib.h>
#include <string.h>

#include "configuration.h"
#include "driver/ba414e/drv_ba414e.h"
#include "drv_ba414e_local.h"
#include "osal/osal.h"
#include "system/int/sys_int.h"
#include <stdio.h>
#include <stdlib.h>

extern char * dbgBufferPtr;
extern uint32_t debugBufferSize;

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

static DRV_BA414E_OperationalData opData = {0};
static DRV_BA414E_ClientData clientData[DRV_BA414E_NUM_CLIENTS];

static const uint32_t init_ucode_array[810]={
    0x10032004,0x48013e00,0x5a800d20,0x09a80202,0x011a8090,0x60287805,0xba022780,0xb2e02fa8,0x0cee0070,
    0x8021e00a,0xd8039a00,0xf5802d60,0x13c8023a,0x013d8050,0x60141804,0xf2013e80,0x50201408,0x14460540,
    0x8070e01d,0x48078205,0x8c804fe0,0x13f804fe,0x013f8050,0x2059b804,0xf6035a80,0xf3204118,0x11b603af,
    0x8167e059,0xf8167e05,0x9e712014,0x00144011,0x10854441,0x80001702,0x41c51100,0x10000017,0x0a490012,
    0x4024a005,0x17200000,0x010000a0,0x03678918,0x40007893,0x44001730,0x01f30209,0xb1800015,0x00040002,
    0x800d9e24,0x610001e2,0x4d10005c,0xc007ce08,0x26c60000,0x54001000,0x0a003640,0xe45c580a,0x80e9c29a,
    0x7ab18c38,0x07ad1cc3,0x8173001c,0x8007ab2c,0x23c67ad3,0x023c67ea,0x080d4b7a,0xc6c0d4b0,0x00054001,
    0x0000a003,0x640e45c5,0x80a80e9e,0x11330e01,0xe11430e0,0x5e115026,0x51c2927c,0x4080a920,0x00054001,
    0x00009c29,0xa4086a00,0x3640e460,0x05d50004,0x000270a4,0x90212800,0xd9039180,0x1cd40010,0x000a0036,
    0x40001e24,0x610005e2,0x4d10001c,0xc007d808,0x26c60000,0x54001000,0x09c58078,0x93440057,0x89184004,
    0x73001f40,0x609b1800,0x01802994,0x0014044a,0x003e4401,0x90092444,0x1a004600,0x00400027,0x0a490212,
    0x8026dcc0,0x0a02d140,0x015057c0,0x00270a49,0x02123401,0x5c580789,0x34000078,0x91840047,0x3001f406,
    0x09b19cc0,0x0a02d100,0x158040cd,0x00044015,0x00804045,0xa0046730,0x0240b430,0x05910054,0x42191106,
    0x80145000,0x04421911,0x0640e469,0x073e4175,0x10044421,0x91106801,0x19120680,0x26dc8003,0x000200a6,
    0x7300250e,0x40000540,0x0174a180,0x00100009,0x00158040,0xcc01570a,0x49c80030,0x249fa863,0x0a11eb1b,
    0x30a11c80,0x030000c0,0x90500040,0x00240056,0x01033005,0x5c292720,0x00c0927c,0x418c2847,0x2000c000,
    0x30240000,0x150005d2,0x8072001e,0x24610001,0xe24d1000,0x1cc0078e,0x4c26c672,0x000c0137,0x89180013,
    0x73001f38,0x009b19c8,0x0000004c,0x00030241,0x01005025,0xd4001000,0x09c580a8,0x0e903917,0x0a490016,
    0x7c400839,0x244021f1,0x0030e111,0x08020439,0xf1093000,0x51089284,0x01e88020,0xe4ab59c7,0x46148001,
    0x20441c90,0x072401c9,0x0072401c,0x98028400,0xa0007e81,0x80900000,0x05400150,0x00540015,0x00054001,
    0x50005400,0x17ea0048,0x8172001f,0xb0812224,0x00018050,0x9edc6022,0x69fb1002,0x269c8000,0x0005c580,
    0xa80e9c29,0xa4021221,0x5b442263,0x15b00041,0x00844004,0xa0142000,0x05eac612,0x205eb470,0x2201c800,
    0x78f2c234,0x77ab180d,0x4b7ec040,0xd4b00004,0x00027300,0x1c800d45,0xb4000150,0x00540430,0x000a014f,
    0xad675108,0x44010e01,0x48442110,0x01480509,0x00434030,0xf624798b,0x450096d8,0x85262bf4,0x005a0142,
    0x4010d00c,0x34045362,0x9298c150,0x098d8852,0x62bf4006,0x20142004,0x02b59d74,0x615c8002,0x00048120,
    0x8467711a,0xd72e01c8,0x0028400a,0x0004010d,0x1086d885,0x262bf000,0x0601bd50,0x00500434,0x030f6247,
    0x98b47624,0x798b4771,0xa99c6ad0,0x900d8a4a,0x63050000,0x50043000,0x05004c00,0x00771af0,0x00050043,
    0x42201f20,0x010005f2,0x0810025e,0xdc602269,0xfb100226,0xac59d000,0x80000150,0x11dc8000,0x002200a6,
    0x44c3a00a,0x60000400,0x02020011,0x00440201,0x011b8014,0x5cc00720,0x0341d344,0x02200517,0x3001c800,
    0xd074c000,0x15000540,0x0b000080,0x80040005,0x00804046,0xe0051730,0x01c80094,0x77c00015,0x0005400f,
    0x00008080,0x04001100,0x804046e0,0x05173001,0xc800d084,0x91008801,0x45cc0072,0x00342129,0x883e014f,
    0x40449e24,0x610009e2,0x4710029e,0x34b08d19,0xe34c08f1,0xdedcd097,0x2deb4b09,0x72de34c0,0x8f31eacb,
    0x08d2deb4,0xc09931c3,0x4e78d342,0x64c78d2c,0x264b70d3,0x9e34c09b,0x2dcc0072,0x00252128,0x083c0002,
    0x00005400,0x14001140,0x1b7b72cc,0x8107ad18,0xc8107300,0x1c800946,0xf1edcb32,0x041eb472,0x2119cc00,
    0x7200251b,0xc7d00463,0x007d0246,0x38800004,0x00028053,0xd1084401,0x0e014240,0x44900437,0x8930400a,
    0x7ab2cc00,0x07ad18c1,0x007ab2c8,0x04b7ad1c,0xc40878d2,0xc234b70d,0x35e34b09,0x731c34d7,0x8f1825c7,
    0x8059c000,0x28053d10,0x844010e0,0x1484030d,0xc80078d3,0x4c48978d,0x18c0817a,0xb3484cd7,0xad38c891,
    0x7ab1826c,0x67ad1c26,0xcd7ab2cc,0x0817ad34,0x274e78d1,0xc23c778d,0x3025cb7b,0x738094d7,0xad34094d,
    0x78d30264,0xb78d3823,0x4678d2c2,0x6cc78d34,0xc489a49f,0x392827ce,0x00274c72,0x001e3ce2,0x0119c800,
    0x7ea4088c,0xd7ad3027,0x4b72001f,0x38808f30,0x00014421,0xa0214806,0xf540017f,0x62025cb7,0xad3025cb,
    0x72000000,0x17ab3025,0xcb7ec202,0x5cb00004,0x00028053,0xd1084401,0x0e014844,0x21500148,0x05090043,
    0x4030d011,0x47ab34c8,0x917ad1cc,0x88a7ab18,0x814d7ad1,0xc26c778f,0x2c80c678,0xf3084c77,0xab1825cb,
    0x7ad38264,0xc7ab1c25,0xc67ad348,0x0c67ce00,0x23ce78d3,0x826cd7f6,0x2084c77a,0xd1884c77,0xce006700,
    0x72001e3c,0x720135fa,0x902232de,0xb4d0991d,0xc8007ce2,0x0234d000,0x05108680,0x85201bd5,0x0005edcc,
    0x32041eb4,0x632041cc,0x00720025,0x1bc7d004,0xc8007d02,0x46308000,0x04000280,0x53d10844,0x010e0148,
    0x4030dc80,0x07ab18c8,0x917ad1cc,0x0817ab2c,0x23467ec4,0x023c6730,0x01c80094,0x6f1eac60,0x272deb47,
    0x08f1deac,0xb31225eb,0x4c12441f,0x30008f19,0xe34b08d2,0xdedcd221,0x19eb4622,0x119e3470,0x9731c800,
    0x7b72c804,0x77ad2c80,0x477cc202,0x5c600005,0x108680af,0xd4001000,0x0a014f44,0x21100438,0x05211085,
    0x40052014,0x24010d00,0xc340451e,0xac632245,0xeb473220,0x9eacb215,0x19eb4602,0x519e3472,0x031de34b,
    0x2132deac,0xc2231deb,0x4708f1de,0x34609919,0xc8007ab1,0x823477ec,0x40264c73,0x001c8009,0x46f1eacc,
    0x0992deb4,0xb0972de3,0x4609919e,0xac730441,0xeb4d3144,0x1f30008d,0x2dc80078,0xd1c80477,0x8d2c804d,
    0x7ab1823c,0xc7ad1c88,0x4b72001f,0x30808f18,0x00014421,0xa02bf500,0x04000280,0x53d10844,0x010e0142,
    0x40449004,0x37893040,0x0a7ab2cc,0x0007ad18,0xc1007ab2,0xc804b7ad,0x1880467a,0xb1cc4087,0xad34c400,
    0x78d2c234,0xb78d1c26,0xc778d2c2,0x5cc70d39,0xe3c60971,0xe0167716,0x02a03a70,0xa69e24a0,0x0069cc00,
    0x78f2c254,0xa4005110,0x847ea004,0x8817ec20,0x48894010,0xd01457ea,0x0048817e,0xc20089a7,0xea08089a,
    0x7ec2808c,0xb4005001,0x0074615c,0x80020004,0x8120dcea,0x673ac80e,0x11cb8072,0x000a1002,0x80010145,
    0x44219f40,0x13140940,0x0178d28c,0x48178f2c,0xc4817ab3,0x0254a7ad,0x3425cb78,0xd38c5027,0x8f1cc502,
    0x7ab3825c,0xe7ad1c23,0xca78f282,0x6cc7ea04,0x26cc7ad2,0xc650878d,0x3423ce78,0xf3823ce7,0xea0826cd,
    0x7ad1c274,0xe78d3025,0xcc7ea288,0x0477ec24,0x264a0000,0x500c3405,0x14000140,0x31501430,0x00040002,
    0x71602a03,0xa70a4dea,0x9412211e,0xb1512231,0xeac70285,0x1eb4802a,0x55eacc02,0x229eb4d0,0x911deace,
    0x0224deb4,0xc09b31e3,0xc708f21e,0x34809939,0xcc0078f1,0xc2447442,0x12016700,0x009c580a,0x80e9c293,
    0x7aa50088,0x87ac4808,0x8a7ab300,0xa147ad2c,0x08937ab3,0x4261278f,0x3025cc78,0xd3425cd7,0xab3826cd,
    0x73001eac,0xe09b39cc,0x007aa542,0x74e7aa50,0x26957aa5,0x026147ac,0x54274c78,0x93800137,0x3001ea94,
    0x09c51cc0,0x00002102,0x1440a110,0x31440e46,0x011e7200,0x000087aa,0x5008947a,0xa540a157,0xab380a95,
    0x73001eac,0xe09b39cc,0x0078d382,0x64e73001,0xc800d503,0x5ea86016,0x55cc0050,0x005ea860,0x2655cc00,
    0x50004000,0x271602a0,0x3a70a4de,0xaa20224d,0x10044030,0xe04dd401,0x0d109040,0x52d11928,0x12f90043,
    0x44241011,0x64464a04,0xf94012d1,0x09040469,0x1192813e,0x51004403,0x0e04dd40,0x10d10904,0x052d1192,
    0x812f9004,0x34424101,0x1644649f,0x20010009,0xf2081002,0x9f201100,0x0df20910,0x02d004b4,0x42410143,
    0x4464a04a,0x04012d10,0x90404791,0x192813e5,0x004b4424,0x1f380044,0x89f20800,0x089f2010,0x0089f389,
    0x04489004,0xb4424101,0x124464a0,0x4f94012d,0x10900040,0x1d185720,0x00800120,0x481014b4,0x464a04be,
    0x812f9090,0x04464a04,0xa072e01c,0x80028400,0xa0004012,0xd1086814,0x18000150,0x00400027,0x1602a03a,
    0x70a4deaa,0x20224d02,0x0240a110,0x30a8105d,0x020640a1,0x5030c810,0x5d020840,0xa31030e8,0x1239000a,
    0x40239011,0x480145cc,0x00720035,0x11544021,0x10894045,0x20051730,0x01c800d4,0x45540014,0x401500cb,
    0x81375004,0xb4032e04,0xe8440110,0x0c381375,0x00434424,0x1014b446,0x4a04a040,0x10d10868,0x14180001,
    0x78d1cc00,0x878f20c0,0x087ab1c8,0x1477ad20,0x85487ab2,0x4c5897ad,0x28c1817c,0xc2424477,0x8f1c2447,
    0x78f2024c,0xa78d2424,0xca7ea002,0x4477ec20,0x64897ea0,0x424c87ec,0x24638973,0x00000017,0x8d1cc008,
    0x78f20c00,0x87ab2823,0xc77ad242,0x4477ab1c,0xc4817ad2,0x0c08178d,0x1c23c778,0xd2024487,0x3001e3c8,
    0x09321e3c,0xa08f29fa,0x820911df,0xb0309321,0xfa8a0952,0x5fb0b095,0x1dcc0000,0x005fa811,0x2201fb09,
    0x12221111,0x17c80800,0x227ea28c,0x48173000,0x000178d1,0xcc0087ce,0x24c00878,0xf2023c77,0xcc042447,
    0x44041109,0x17cc04c0,0x0073001f,0xa8910221,0xcc000000,0x5fa8b102,0x25cc0000,0x021f3023,0x0021f38a,
    0x30021f30,0x330205cc,0x00000200,0x00144441,0xf20a0008,0x9e414304,0x29fa8112,0x801fb091,0x2821cc00,
    0x00004000,0x28053d02,0x0240a1d0,0x30e805c5,0xc8000200,0x0c001716,0x01c09271,0x44cc0924,0x0039009a,
    0x40429039,0x38017dcc,0x00944550,0x21a40a1d,0x030d8026,0xdcc00400,0x19008a40,0x42d03938,0x017dcc00,
    0x00021000,0xc4022d01,0x0b80119c,0xc0000021,0x000d4022,0xd010b801,0x7dcc0094,0x454c0003,0x02414001,
    0x00008080,0x070a68c0,0x0171601c,0x0927144c,0xc0924086,0x9028b40c,0x36009b73,0x001000c4,0x0235011c,
    0x40e4e005,0xf7300100,0x0a402350,0x11b8017d,0xcc000300,0x0c000302,0x42014f40,0x821029b4,0x0c7a0171,
    0x72001020,0x240a7103,0x1c805c5c,0x80040271,0x00438052,0x1009e400,0x52014240,0x10d00c34,0x04536292,
    0x98c15008,0xed885262,0xbf720000,0x80030005,0xc5807024,0x9c513302,0x49000e40,0x269011c4,0x0e4e005f,
    0x73000000,0x84007100,0x8a4046e0,0x05173001,0xc800d445,0x4c000302,0x41400100,0x00808008,0x078a01d6,
    0x40009008,0x04046e01,0xc8030010,0x0824010e,0x0142d88b,0x26340500,0x05400100,0x00400015,0x00054001,
};

#define TUPLE_COUNT         90


// *****************************************************************************
// *****************************************************************************
// Section: Static functions
// *****************************************************************************
// *****************************************************************************
static inline void DRV_BA414E_DirectCopy(void * dst, const void * src, uint32_t dstLen, uint32_t srcLen)
{
    uint32_t numBytes = srcLen;
    if (srcLen > dstLen)
    {
        numBytes = dstLen;
    }
    uint32_t numWords = numBytes >> 2;
    uint32_t * pDst = (uint32_t *)dst;
    uint32_t * pSrc = (uint32_t *)src;
    uint32_t counter = 0;
    memset(dst, 0, dstLen);
    for (counter = 0; counter < numWords; counter++)
    {
        *(pDst++) = *(pSrc++);
    }
    if ((numBytes & 0x3) != 0)
    {
        uint8_t * p8Dst = (uint8_t*)pDst;
        uint8_t * p8Src = (uint8_t*)pSrc;
        for (counter = 0; counter < (numBytes & 0x3); counter++)
        {
            *(p8Dst++) = *(p8Src++);
        }
    }

}

static inline void DRV_BA414E_ReverseWordCopy(void * dst, const void * src, uint32_t dstLen, uint32_t srcLen)
{
    uint32_t numBytes = srcLen;
    if (srcLen > dstLen)
    {
        numBytes = dstLen;
    }
    uint32_t numWords = numBytes >> 2;
    uint32_t * pDst = (uint32_t *)dst + (numWords-1);
    if ((numBytes & 0x3) != 0)
    {
        pDst++;
    }
    uint32_t * pSrc = (uint32_t *)src;
    uint32_t counter = 0;
    memset(dst, 0, dstLen);
    for (counter = 0; counter < numWords; counter++)
    {
        *(pDst--) = *(pSrc++);
    }
    if ((numBytes & 0x3) != 0)
    {
        uint8_t * p8Dst = (uint8_t*)pDst;
        uint8_t * p8Src = (uint8_t*)pSrc;
        for (counter = 0; counter < (numBytes & 0x3); counter++)
        {
            *(p8Dst++) = *(p8Src++);
        }
    }
}

static inline void DRV_BA414E_ReverseBytesCopy(void * dst, const void * src, uint32_t dstLen, uint32_t srcLen)
{
    uint32_t numBytes = srcLen;
    if (srcLen > dstLen)
    {
        numBytes = dstLen;
    }
    uint32_t numWords = numBytes >> 2;
    uint32_t * pDst = (uint32_t *)dst;
    uint32_t * pSrc = (uint32_t *)src;
    uint32_t counter = 0;
    memset(dst, 0, dstLen);
    for (counter = 0; counter < numWords; counter++)
    {
        uint32_t tmp = *(pSrc++);

        *(pDst++) = ((tmp & 0xff000000) >> 24) |
                    ((tmp & 0x00ff0000) >> 8) |
                    ((tmp & 0x0000ff00) << 8) |
                    ((tmp & 0x000000ff) << 24);
    }
    if ((numBytes & 0x3) != 0)
    {
        uint32_t tmp = 0;
        uint8_t * p8Dst = (uint8_t*)&tmp;
        uint8_t * p8Src = (uint8_t*)pSrc;
        for (counter = 0; counter < (numBytes & 0x3); counter++)
        {
            *(p8Dst++) = *(p8Src++);
        }
        *(pDst++) = ((tmp & 0xff000000) >> 24) |
            ((tmp & 0x00ff0000) >> 8) |
            ((tmp & 0x0000ff00) << 8) |
            ((tmp & 0x000000ff) << 24);
    }
}

static inline void DRV_BA414E_ReverseBytesWordsCopy(void * dst, const void * src, uint32_t dstLen, uint32_t srcLen)
{
    uint32_t numBytes = srcLen;
    if (srcLen > dstLen)
    {
        numBytes = dstLen;
    }
    uint8_t * pDst = (uint8_t *)dst + numBytes - 1;
    uint8_t * pSrc = (uint8_t *)src;
    uint32_t counter = 0;
    memset(dst, 0, dstLen);
    for (counter = 0; counter < numBytes; counter++)
    {
        *(pDst--) = *(pSrc++);
    }
}

void DRV_BA414E_MemCopy(void * dst, const void * src, uint32_t dstLen, uint32_t srcLen, uint8_t reverseWords, uint8_t reverseBytes, uint8_t packEnd)
{
    uint8_t __attribute__((aligned(16))) slotMax[DRV_BA414E_MAX_KEY_SIZE];
    memset(slotMax, 0, DRV_BA414E_MAX_KEY_SIZE);
    memset(dst, 0, dstLen);
    uint32_t numBytes = srcLen;
    if (srcLen > dstLen)
    {
        numBytes = dstLen;
    }
    if ((reverseWords == 0) && (reverseBytes == 0))
    {
        DRV_BA414E_DirectCopy(slotMax, src, dstLen, srcLen);
    }
    else if ((reverseWords == 1) && (reverseBytes == 0))
    {
        DRV_BA414E_ReverseWordCopy(slotMax, src, dstLen, srcLen);
    }
    else if ((reverseWords == 0) && (reverseBytes == 1))
    {
        DRV_BA414E_ReverseBytesCopy(slotMax, src, dstLen, srcLen);
    }
    else if ((reverseWords == 1) && (reverseBytes == 1))
    {
        DRV_BA414E_ReverseBytesWordsCopy(slotMax, src, dstLen, srcLen);
    }
    if (packEnd == 0)
    {
        memcpy(dst, slotMax, numBytes);
    }
    else
    {
        uint8_t * pDst = (uint8_t *)dst;
        pDst += (dstLen - numBytes);
        memcpy(pDst, slotMax, numBytes);
    }

}

static uint32_t DRV_BA414E_getSlotAddr(uint8_t slot_num)
{
    uint32_t addr = 0ul;

    if ( slot_num < DRV_BA414E_MAX_SLOT_NUM ) {
        addr = (uint32_t)(DRV_BA414E_SCMEM_BASE) + ((uint32_t)(slot_num) << (DRV_BA414E_SCM_SLOT_LS));
    }

    return addr;
}


static void DRV_BA414E_copyToScm4(const void* pdata,
                      uint16_t numBytes, 
                      uint8_t slot_num,
                      uint8_t swapBytes,
                      uint8_t swapWords,
                      uint8_t packToBack)
{
    uint8_t __attribute__((aligned(16))) tempBuffer[DRV_BA414E_MAX_KEY_SIZE];
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.v = PKCOMMAND;
    uint32_t slotSize = cmd.s.OPSIZE * 8;

    DRV_BA414E_MemCopy(tempBuffer, pdata, slotSize, numBytes, swapBytes, swapWords, packToBack);

    uint32_t addr = DRV_BA414E_getSlotAddr(slot_num);
    uint32_t counter;
    uint32_t *pSrc = (uint32_t *)tempBuffer;
    uint32_t *pDst = (uint32_t *)addr;
    for (counter = 0; counter < (slotSize >> 2); counter++)
    {
        *(pDst++) = *(pSrc++);
    }
}

static void DRV_BA414E_copyFromScm2(void* pdata, 
                                    uint32_t numBytes,
                                    uint8_t slotNum, 
                                    uint8_t swapBytes,
                                    uint8_t swapWords,
                                    uint8_t packToBack)
{
    uint8_t __attribute__((aligned(16))) tempBuffer[DRV_BA414E_MAX_KEY_SIZE];
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.v = PKCOMMAND;
    uint32_t slotSize = cmd.s.OPSIZE * 8;
    uint32_t addr = DRV_BA414E_getSlotAddr(slotNum);
    uint32_t counter;
    uint32_t *pSrc = (uint32_t *)addr;
    uint32_t *pDst = (uint32_t *)tempBuffer;
    for (counter = 0; counter < (slotSize >> 2); counter++)
    {
        *(pDst++) = *(pSrc++);
    }
    numBytes = min(numBytes, slotSize);
    DRV_BA414E_MemCopy(pdata, tempBuffer, numBytes, slotSize, swapBytes, swapWords, packToBack);
        
}

static void DRV_BA414E_scmClear(void)
{
    uint32_t addr = (DRV_BA414E_SCMEM_BASE);

    while ( addr < (DRV_BA414E_SCMEM_END) ) {
        *(uint32_t*)(addr) = 0x0;
        addr += 4ul;
    }
}

static void DRV_BA414E_ucmemInit(void)
{
    uint32_t i, j;
    uint32_t * uCodeMem;
    uint32_t * inPtr = (uint32_t *)init_ucode_array;

    uCodeMem = (uint32_t*)(__CRYPTO1UCM_BASE | 0x20000000);
        
    for (i = 0; i < TUPLE_COUNT; i++)
    {
        uCodeMem[0] = inPtr[0] >> 14;
        uCodeMem[1] = inPtr[0] << 4;
        uCodeMem[1] |= inPtr[1] >> 28;
        uCodeMem[2] = inPtr[1] >> 10;
        uCodeMem[3] = inPtr[1] << 8;
        uCodeMem[3] |= inPtr[2] >> 24;
        uCodeMem[4] = inPtr[2] >> 6;
        uCodeMem[5] = inPtr[2] << 12;
        uCodeMem[5] |= inPtr[3] >> 20;
        uCodeMem[6] = inPtr[3] >> 2;
        uCodeMem[7] = inPtr[3] << 16;
        uCodeMem[7] |= inPtr[4] >> 16;
        uCodeMem[8] = inPtr[4] << 2;
        uCodeMem[8] |= inPtr[5] >> 30;
        uCodeMem[9] = inPtr[5] >> 12;
        uCodeMem[10] = inPtr[5] << 6;
        uCodeMem[10] |= inPtr[6] >> 26;
        uCodeMem[11] = inPtr[6] >> 8;
        uCodeMem[12] = inPtr[6] << 10;
        uCodeMem[12] |= inPtr[7] >> 22;
        uCodeMem[13] = inPtr[7] >> 4;
        uCodeMem[14] = inPtr[7] << 14;
        uCodeMem[14] |= inPtr[8] >> 18;
        uCodeMem[15] = inPtr[8];
        for (j = 0; j < 16; j++)
        {
            uCodeMem[j] &= 0x3FFFF;
        }
        
        inPtr += 9;
        uCodeMem += 16;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: BA414E Driver Interface State Handlers
// *****************************************************************************
// *****************************************************************************

void DRV_BA414E_InitialStateHandler()
{
    DRV_BA414E_ucmemInit();
    DRV_BA414E_scmClear();
}


// *****************************************************************************
// *****************************************************************************
// Section: BA414E Driver Interface Implementations
// *****************************************************************************
// *****************************************************************************



SYS_MODULE_OBJ DRV_BA414E_Initialize
(
    const SYS_MODULE_INDEX index,
    const SYS_MODULE_INIT * const init
)
{
    SYS_MODULE_OBJ ret = 0;
    if ((index != 0) || (opData.inited == 1))
    {
        ret = SYS_MODULE_OBJ_INVALID;
    }
    else
    {
        memset(&opData, 0, sizeof(DRV_BA414E_OperationalData));
        opData.inited = 1;
        opData.state = DRV_BA414E_INITIALIZE;
        opData.status = SYS_STATUS_UNINITIALIZED;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
        OSAL_SEM_Create(&opData.clientListSema, OSAL_SEM_TYPE_BINARY, 1, 1);
        OSAL_SEM_Create(&opData.wfi, OSAL_SEM_TYPE_BINARY, DRV_BA414E_NUM_CLIENTS, 0);
    #if !defined(DRV_BA414_RTOS_TASK_DELAY)
        OSAL_SEM_Create(&opData.clientAction, OSAL_SEM_TYPE_COUNTING, DRV_BA414E_NUM_CLIENTS, 0);
    #endif
#endif        
        memset(&clientData, 0, sizeof(clientData));
        int counter;
        for (counter = 0; counter < DRV_BA414E_NUM_CLIENTS; counter++)
        {
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
            OSAL_SEM_Create(&(clientData[counter].clientBlock), OSAL_SEM_TYPE_BINARY, 1, 0);
#endif            
        }
        
        ret = (SYS_MODULE_OBJ)&opData;
    }
    return ret;
}

void DRV_BA414E_Deinitialize( SYS_MODULE_OBJ object)
{
    if (object == (SYS_MODULE_OBJ)&opData)
    {
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
        OSAL_SEM_Delete(&opData.clientListSema);
        OSAL_SEM_Delete(&opData.wfi);
    #if !defined(DRV_BA414_RTOS_TASK_DELAY)
        OSAL_SEM_Delete(&opData.clientAction);
    #endif
#endif        
        int counter;
        for (counter = 0; counter < DRV_BA414E_NUM_CLIENTS; counter++)
        {
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
            OSAL_SEM_Delete(&(clientData[counter].clientBlock));
#endif            
        }
        memset(&clientData, 0, sizeof(clientData));
        memset(&opData, 0, sizeof(DRV_BA414E_OperationalData));
    }
}

DRV_HANDLE DRV_BA414E_Open( const SYS_MODULE_INDEX index, 
        const DRV_IO_INTENT ioIntent)
{
    DRV_HANDLE ret = DRV_HANDLE_INVALID;
    if (index == 0)
    {
#if !defined(DRV_BA414E_RTOS_STACK_SIZE)
        if ((ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
#endif
        {        
            if ((ioIntent & DRV_IO_INTENT_WRITE) == DRV_IO_INTENT_WRITE)
            {
                uint8_t lookingForExclusive = 0;
                if ((ioIntent & DRV_IO_INTENT_EXCLUSIVE) == DRV_IO_INTENT_EXCLUSIVE)
                {
                    lookingForExclusive = 1;
                }
                uint8_t found = -1;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Pend(&opData.clientListSema, OSAL_WAIT_FOREVER);
#endif                
                int counter;
                for (counter = 0; counter < DRV_BA414E_NUM_CLIENTS; counter++)
                {
                    if (clientData[counter].inUse == 0)
                    {
                        found = counter;
                    }
                    else
                    {
                        if (lookingForExclusive == 1 || ((clientData[counter].ioIntent & DRV_IO_INTENT_EXCLUSIVE) == DRV_IO_INTENT_EXCLUSIVE))
                        {
                            found = -1;
                            break;
                        }
                    }
                }
                if (found != -1)
                {
                    clientData[found].inUse = 1;
                    clientData[found].ioIntent = ioIntent;
                    ret = (DRV_HANDLE)&(clientData[found]);
                }
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientListSema);
#endif                                
            }
        }  
        
    }
        
    return ret;
}

void DRV_BA414E_Close( const DRV_HANDLE handle)
{
    if (handle != DRV_HANDLE_INVALID)
    {
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
        OSAL_SEM_Pend(&opData.clientListSema, OSAL_WAIT_FOREVER);
#endif                
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData*)handle;
        cd->ioIntent = 0;
        cd->inUse = 0;                
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
        OSAL_SEM_Post(&opData.clientListSema);
#endif                                
    }           
}


SYS_STATUS DRV_BA414E_Status( SYS_MODULE_OBJ object)
{
    SYS_STATUS ret = SYS_STATUS_ERROR;
    if (object == (SYS_MODULE_OBJ)&opData)
    {
        ret = opData.status;
    }
    return ret;
}


void DRV_BA414E_InterruptHandler()
{
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
    OSAL_SEM_PostISR(&opData.wfi);
#endif
    opData.doneInterrupt = 1;
    opData.lastStatus = PKSTATUS;
    PKCONTROL = 0;
    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1);
}

void DRV_BA414E_ErrorInterruptHandler()
{
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
    OSAL_SEM_PostISR(&opData.wfi);
#endif
    opData.errorInterrupt = 1;
    opData.lastStatus = PKSTATUS;
    PKCONTROL = 0;
    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1_FAULT);    
}

void DRV_BA414E_StartOp()
{
    PKCONTROL = 0;
    //snprintf(dbgBufferPtr, debugBufferSize, "%s\r\n%s: PKSTATUS %08X Done %d Error %d\r\n", dbgBufferPtr, __FUNCTION__, PKSTATUS, opData.doneInterrupt, opData.errorInterrupt);
    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1);
    opData.doneInterrupt = 0;
    opData.errorInterrupt = 0;
    //snprintf(dbgBufferPtr, debugBufferSize, "%s\r\n%s: PKSTATUS %08X Done %d Error %d\r\n", dbgBufferPtr, __FUNCTION__, PKSTATUS, opData.doneInterrupt, opData.errorInterrupt);
    if (SYS_INT_SourceStatusGet(INT_SOURCE_CRYPTO1) != 0)
    {
        //snprintf(dbgBufferPtr, debugBufferSize, "%s\r\n%s: Int still triggered\r\n", dbgBufferPtr, __FUNCTION__);
        PKCONTROL = 0;
        SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1_FAULT);
        SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1);        
    }
    //snprintf(dbgBufferPtr, debugBufferSize, "%s\r\n%s: PKSTATUS %08X Done %d Error %d\r\n", dbgBufferPtr, __FUNCTION__, PKSTATUS, opData.doneInterrupt, opData.errorInterrupt);
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1);
    //snprintf(dbgBufferPtr, debugBufferSize, "%s\r\n%s: PKSTATUS %08X Done %d Error %d\r\n", dbgBufferPtr, __FUNCTION__, PKSTATUS, opData.doneInterrupt, opData.errorInterrupt);
    PKCONTROL = 1;
}

void DRV_BA414E_PrepareEcdsaSign(DRV_BA414E_ClientData * cd)
{
    uint32_t len = cd->ecdsaSignParams.domain->keySize;

    DRV_BA414E_scmClear();
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.s.OPERATION = BA414E_OPC_ECC_ECDSA_SIGN;
    cmd.s.OPSIZE = cd->ecdsaSignParams.domain->opSize;
    cmd.s.CALCR2 = 1;
    PKCOMMAND = cmd.v;
    PKCONFIG = 0;
    
    
    DRV_BA414E_copyToScm4(cd->ecdsaSignParams.domain->primeField, len, BA414E_ECDSA_SLOT_P, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaSignParams.domain->order, len, BA414E_ECDSA_SLOT_N, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaSignParams.domain->generatorX, len, BA414E_ECDSA_SLOT_GX, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaSignParams.domain->generatorY, len, BA414E_ECDSA_SLOT_GY, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaSignParams.domain->a, len, BA414E_ECDSA_SLOT_A, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaSignParams.domain->b, len, BA414E_ECDSA_SLOT_B, 0, 0, 0);    
    
    DRV_BA414E_copyToScm4(cd->ecdsaSignParams.privateKey, len, BA414E_ECDSA_SLOT_PRIV_KEY, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaSignParams.k, len, BA414E_ECDSA_SLOT_K, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaSignParams.msgHash, cd->ecdsaSignParams.msgHashSz, BA414E_ECDSA_SLOT_H, 1, 1, 0);
    opData.doneInterrupt = 0;
    opData.errorInterrupt = 0;
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1);
    PKCONTROL = 1;
}

void DRV_BA414E_PrepareEcdsaVerify(DRV_BA414E_ClientData * cd)
{
    uint32_t len = cd->ecdsaVerifyParams.domain->keySize;

    DRV_BA414E_scmClear();
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.s.OPERATION = BA414E_OPC_ECC_ECDSA_VERIFY;
    cmd.s.OPSIZE = cd->ecdsaVerifyParams.domain->opSize;
    cmd.s.CALCR2 = 1;
    PKCOMMAND = cmd.v;
    PKCONFIG = 0;
        
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.domain->primeField, len, BA414E_ECDSA_SLOT_P, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.domain->order, len, BA414E_ECDSA_SLOT_N, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.domain->generatorX, len, BA414E_ECDSA_SLOT_GX, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.domain->generatorY, len, BA414E_ECDSA_SLOT_GY, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.domain->a, len, BA414E_ECDSA_SLOT_A, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.domain->b, len, BA414E_ECDSA_SLOT_B, 0, 0, 0);    
    
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.publicKeyX, len, BA414E_ECDSA_SLOT_X0, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.publicKeyY, len, BA414E_ECDSA_SLOT_Y0, 0, 0, 0);
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.R, len, BA414E_ECDSA_SLOT_R, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.S, len, BA414E_ECDSA_SLOT_S, 0, 0, 0);
    
    DRV_BA414E_copyToScm4(cd->ecdsaVerifyParams.msgHash, cd->ecdsaSignParams.msgHashSz, BA414E_ECDSA_SLOT_H, 1, 1, 0);
    opData.doneInterrupt = 0;
    opData.errorInterrupt = 0;    
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1);    
    PKCONTROL = 1;
}

void DRV_BA414E_PrimEccPointDouble(DRV_BA414E_ClientData * cd)
{
    uint32_t len = cd->eccPointDoubleParams.domain->keySize;

    DRV_BA414E_scmClear();
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.s.OPERATION = BA414E_OPC_PRIM_ECC_POINT_DOUBLE;
    cmd.s.OPSIZE = cd->eccPointDoubleParams.domain->opSize;
    cmd.s.CALCR2 = 1;
    BA414E__PKCONFIGbits cfg = {{0}};
    cfg.s.OPPTRA = BA414E_ECCP_SLOT_P1X;
    cfg.s.OPPTRC = BA414E_ECCP_SLOT_P3X;
    PKCONFIG = cfg.v;
    PKCOMMAND = cmd.v;
    
    DRV_BA414E_copyToScm4(cd->eccPointDoubleParams.domain->primeField, len, BA414E_ECCP_SLOT_P, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointDoubleParams.domain->order, len, BA414E_ECCP_SLOT_N, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointDoubleParams.domain->generatorX, len, BA414E_ECCP_SLOT_GX, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointDoubleParams.domain->generatorY, len, BA414E_ECCP_SLOT_GY, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointDoubleParams.domain->a, len, BA414E_ECCP_SLOT_A, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointDoubleParams.domain->b, len, BA414E_ECCP_SLOT_B, 0, 0, 0);    
    
    DRV_BA414E_copyToScm4(cd->eccPointDoubleParams.p1X, len, BA414E_ECCP_SLOT_P1X, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointDoubleParams.p1Y, len, BA414E_ECCP_SLOT_P1Y, 0, 0, 0);    
    
    opData.doneInterrupt = 0;
    opData.errorInterrupt = 0;    
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1);    
    PKCONTROL = 1;    
}

void DRV_BA414E_PrimEccPointAddition(DRV_BA414E_ClientData * cd)
{
    uint32_t len = cd->eccPointDoubleParams.domain->keySize;

    DRV_BA414E_scmClear();
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.s.OPERATION = BA414E_OPC_PRIM_ECC_POINT_ADDITION;
    cmd.s.OPSIZE = cd->eccPointAdditionParams.domain->opSize;
    cmd.s.CALCR2 = 1;
    BA414E__PKCONFIGbits cfg = {{0}};
    cfg.s.OPPTRA = BA414E_ECCP_SLOT_P1X;
    cfg.s.OPPTRB = BA414E_ECCP_SLOT_P2X;
    cfg.s.OPPTRC = BA414E_ECCP_SLOT_P3X;
    PKCONFIG = cfg.v;
    PKCOMMAND = cmd.v;
    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.domain->primeField, len, BA414E_ECCP_SLOT_P, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.domain->order, len, BA414E_ECCP_SLOT_N, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.domain->generatorX, len, BA414E_ECCP_SLOT_GX, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.domain->generatorY, len, BA414E_ECCP_SLOT_GY, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.domain->a, len, BA414E_ECCP_SLOT_A, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.domain->b, len, BA414E_ECCP_SLOT_B, 0, 0, 0);    
    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.p1X, len, BA414E_ECCP_SLOT_P1X, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.p1Y, len, BA414E_ECCP_SLOT_P1Y, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.p2X, len, BA414E_ECCP_SLOT_P2X, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointAdditionParams.p2Y, len, BA414E_ECCP_SLOT_P2Y, 0, 0, 0);    
    
    opData.doneInterrupt = 0;
    opData.errorInterrupt = 0;    
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1);    
    PKCONTROL = 1;    
}

void DRV_BA414E_PrimEccPointMultiplication(DRV_BA414E_ClientData * cd)
{
    uint32_t len = cd->eccPointDoubleParams.domain->keySize;

    DRV_BA414E_scmClear();
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.s.OPERATION = BA414E_OPC_PRIM_ECC_POINT_MULTI;
    cmd.s.OPSIZE = cd->eccPointAdditionParams.domain->opSize;
    cmd.s.CALCR2 = 1;
    BA414E__PKCONFIGbits cfg = {{0}};
    cfg.s.OPPTRA = BA414E_ECCP_SLOT_P1X;
    cfg.s.OPPTRB = BA414E_ECCP_SLOT_K;
    cfg.s.OPPTRC = BA414E_ECCP_SLOT_P3X;
    PKCONFIG = cfg.v;
    PKCOMMAND = cmd.v;
    
    DRV_BA414E_copyToScm4(cd->eccPointMultiplicationParams.domain->primeField, len, BA414E_ECCP_SLOT_P, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointMultiplicationParams.domain->order, len, BA414E_ECCP_SLOT_N, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointMultiplicationParams.domain->generatorX, len, BA414E_ECCP_SLOT_GX, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointMultiplicationParams.domain->generatorY, len, BA414E_ECCP_SLOT_GY, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointMultiplicationParams.domain->a, len, BA414E_ECCP_SLOT_A, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointMultiplicationParams.domain->b, len, BA414E_ECCP_SLOT_B, 0, 0, 0);    
    
    DRV_BA414E_copyToScm4(cd->eccPointMultiplicationParams.p1X, len, BA414E_ECCP_SLOT_P1X, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointMultiplicationParams.p1Y, len, BA414E_ECCP_SLOT_P1Y, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccPointMultiplicationParams.k, len, BA414E_ECCP_SLOT_K, 0, 0, 0);    
    
    opData.doneInterrupt = 0;
    opData.errorInterrupt = 0;    
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1);    
    PKCONTROL = 1;    
}


void DRV_BA414E_PrimEccCheckPointOnCurve(DRV_BA414E_ClientData * cd)
{
    uint32_t len = cd->eccPointDoubleParams.domain->keySize;

    DRV_BA414E_scmClear();
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.s.OPERATION = BA414E_OPC_PRIM_ECC_POINT_CHECK_POINT_ON_CURVE;
    cmd.s.OPSIZE = cd->eccPointAdditionParams.domain->opSize;
    cmd.s.CALCR2 = 1;
    BA414E__PKCONFIGbits cfg = {{0}};
    cfg.s.OPPTRA = BA414E_ECCP_SLOT_P1X;
    PKCONFIG = cfg.v;
    PKCOMMAND = cmd.v;
    
    DRV_BA414E_copyToScm4(cd->eccCheckPointOnCurveParams.domain->primeField, len, BA414E_ECCP_SLOT_P, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccCheckPointOnCurveParams.domain->order, len, BA414E_ECCP_SLOT_N, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccCheckPointOnCurveParams.domain->generatorX, len, BA414E_ECCP_SLOT_GX, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccCheckPointOnCurveParams.domain->generatorY, len, BA414E_ECCP_SLOT_GY, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccCheckPointOnCurveParams.domain->a, len, BA414E_ECCP_SLOT_A, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccCheckPointOnCurveParams.domain->b, len, BA414E_ECCP_SLOT_B, 0, 0, 0);    
    
    DRV_BA414E_copyToScm4(cd->eccCheckPointOnCurveParams.p1X, len, BA414E_ECCP_SLOT_P1X, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->eccCheckPointOnCurveParams.p1Y, len, BA414E_ECCP_SLOT_P1Y, 0, 0, 0);    
    
    opData.doneInterrupt = 0;
    opData.errorInterrupt = 0;    
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceEnable(INT_SOURCE_CRYPTO1);    
    PKCONTROL = 1;    
}

void DRV_BA414E_PrimModAddition(DRV_BA414E_ClientData * cd, BA414E_OP_CODES op)
{
    uint32_t len = cd->modOperationParams.opSize * 8;

    DRV_BA414E_scmClear();
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.s.OPERATION = op;
    cmd.s.OPSIZE = cd->modOperationParams.opSize;
    cmd.s.CALCR2 = 1;
    BA414E__PKCONFIGbits cfg = {{0}};
    cfg.s.OPPTRA = BA414E_MODP_SLOT_A;
    cfg.s.OPPTRB = BA414E_MODP_SLOT_B;
    cfg.s.OPPTRC = BA414E_MODP_SLOT_C;
    PKCONFIG = cfg.v;
    PKCOMMAND = cmd.v;
    DRV_BA414E_copyToScm4(cd->modOperationParams.p, len, BA414E_MODP_SLOT_P, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->modOperationParams.a, len, BA414E_MODP_SLOT_A, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->modOperationParams.b, len, BA414E_MODP_SLOT_B, 0, 0, 0);    

    DRV_BA414E_StartOp();
}

void DRV_BA414E_PrimModExp(DRV_BA414E_ClientData * cd)
{
    uint32_t len = cd->modExpParams.opSize * 8;

    DRV_BA414E_scmClear();
    BA414E_PKCOMMANDbits cmd = {{0}};
    cmd.s.OPERATION = BA414E_OPC_RSA_MOD_EXP;
    cmd.s.OPSIZE = cd->modExpParams.opSize;
    cmd.s.CALCR2 = 1;
    BA414E__PKCONFIGbits cfg = {{0}};
    cfg.s.OPPTRA = BA414E_RSA_MODEXP_M;
    cfg.s.OPPTRB = BA414E_RSA_MODEXP_e;
    cfg.s.OPPTRC = BA414E_RSA_MODEXP_C;
    PKCONFIG = cfg.v;
    PKCOMMAND = cmd.v;
    
    DRV_BA414E_copyToScm4(cd->modExpParams.n, len, BA414E_RSA_MODEXP_n, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->modExpParams.M, len, BA414E_RSA_MODEXP_M, 0, 0, 0);    
    DRV_BA414E_copyToScm4(cd->modExpParams.e, len, BA414E_RSA_MODEXP_e, 0, 0, 0);    
    
    DRV_BA414E_StartOp();   
}


void DRV_BA414E_Prepare(DRV_BA414E_ClientData * cd)
{
    PKCONTROL = 0;   
    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1);
    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1);
    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1_FAULT);
    opData.doneInterrupt = 0;
    opData.errorInterrupt = 0;    
    switch(cd->currentOp)
    {
        case DRV_BA414E_OP_ECDSA_SIGN:
            DRV_BA414E_PrepareEcdsaSign(cd);
            break;            
        case DRV_BA414E_OP_ECDSA_VERIFY:
            DRV_BA414E_PrepareEcdsaVerify(cd);
            break;            
        case DRV_BA414E_OP_PRIM_ECC_POINT_DOUBLE:
            DRV_BA414E_PrimEccPointDouble(cd);
            break;            
        case DRV_BA414E_OP_PRIM_ECC_POINT_ADDITION:
            DRV_BA414E_PrimEccPointAddition(cd);
            break;            
        case DRV_BA414E_OP_PRIM_ECC_POINT_MULTIPLICATION:
            DRV_BA414E_PrimEccPointMultiplication(cd);
            break;            
        case DRV_BA414E_OP_PRIM_ECC_CHECK_POINT_ON_CURVE:
            DRV_BA414E_PrimEccCheckPointOnCurve(cd);
            break;            
        case DRV_BA414E_OP_PRIM_MOD_ADDITION:
            DRV_BA414E_PrimModAddition(cd, BA414E_OPC_PRIM_MOD_ADD);
            break;            
        case DRV_BA414E_OP_PRIM_MOD_SUBTRACTION:
            DRV_BA414E_PrimModAddition(cd, BA414E_OPC_PRIM_MOD_SUB);
            break;            
        case DRV_BA414E_OP_PRIM_MOD_MULTIPLICATION:
            DRV_BA414E_PrimModAddition(cd, BA414E_OPC_PRIM_MOD_MULT);
            break;
        case DRV_BA414E_OP_PRIM_MOD_EXP:
            DRV_BA414E_PrimModExp(cd);
            break;
        case DRV_BA414E_OP_NONE:
        default:
            break;
    }
}
void DRV_BA414E_ProcessEcdsaSign(DRV_BA414E_ClientData * cd)
{
    BA414E__PKSTATUSbits currentStatus;
    uint32_t len = cd->ecdsaSignParams.domain->keySize;
    currentStatus.v = opData.lastStatus;
    
    cd->currentOp = DRV_BA414E_OP_NONE;
    if ((opData.errorInterrupt == 1) || (currentStatus.s.SIGINVAL == 1))
    {
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_ERROR, cd->context);
        }
    }
    else if (opData.doneInterrupt == 1)
    {
        DRV_BA414E_copyFromScm2(cd->ecdsaSignParams.R, len, BA414E_ECDSA_SLOT_R, 0, 0, 0);
        DRV_BA414E_copyFromScm2(cd->ecdsaSignParams.S, len, BA414E_ECDSA_SLOT_S, 0, 0, 0);
        
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_SUCCESS, cd->context);
        }
    }
}

void DRV_BA414E_ProcessEcdsaVerify(DRV_BA414E_ClientData * cd)
{
    BA414E__PKSTATUSbits currentStatus;
    currentStatus.v = opData.lastStatus;
    
    cd->currentOp = DRV_BA414E_OP_NONE;
    if ((currentStatus.s.SIGINVAL == 1))
    {
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_SIGN_VERIFY_FAIL, cd->context);
        }        
    }
    else if ((opData.errorInterrupt == 1))
    {
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_ERROR, cd->context);
        }
    }
    else if (opData.doneInterrupt == 1)
    {
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_SUCCESS, cd->context);
        }
    }
}

void DRV_BA414E_ProcessPrimEccPointDouble(DRV_BA414E_ClientData * cd)
{

    BA414E__PKSTATUSbits currentStatus;
    currentStatus.v = opData.lastStatus;
    uint32_t len = cd->eccPointDoubleParams.domain->keySize;
    
    cd->currentOp = DRV_BA414E_OP_NONE;
    if ((opData.errorInterrupt == 1))
    {
        DRV_BA414E_OP_RESULT opRes = (currentStatus.s.PXINF == 0) ? DRV_BA414E_OP_ERROR : DRV_BA414E_OP_ERROR_POINT_AT_INFINITY;
        if (cd->callback != 0)
        {            
            (cd->callback)(opRes, cd->context);
        }
    }
    else if (opData.doneInterrupt == 1)
    {
        DRV_BA414E_OP_RESULT opRes = (currentStatus.s.PXINF == 0) ? DRV_BA414E_OP_SUCCESS : DRV_BA414E_OP_POINT_AT_INFINITY;
        DRV_BA414E_copyFromScm2(cd->eccPointDoubleParams.outX, len, BA414E_ECCP_SLOT_P3X, 0, 0, 0);
        DRV_BA414E_copyFromScm2(cd->eccPointDoubleParams.outY, len, BA414E_ECCP_SLOT_P3Y, 0, 0, 0);
        
        if (cd->callback != 0)
        {            
            (cd->callback)(opRes, cd->context);
        }
    }  
}

void DRV_BA414E_ProcessPrimEccPointAddition(DRV_BA414E_ClientData * cd)
{

    uint32_t len = cd->eccPointAdditionParams.domain->keySize;
    BA414E__PKSTATUSbits currentStatus;
    currentStatus.v = opData.lastStatus;
    
    cd->currentOp = DRV_BA414E_OP_NONE;
    if ((opData.errorInterrupt == 1))
    {
        DRV_BA414E_OP_RESULT opRes = (currentStatus.s.PXINF == 0) ? DRV_BA414E_OP_ERROR : DRV_BA414E_OP_ERROR_POINT_AT_INFINITY;
        if (cd->callback != 0)
        {            
            (cd->callback)(opRes, cd->context);
        }
    }
    else if (opData.doneInterrupt == 1)
    {
        DRV_BA414E_OP_RESULT opRes = (currentStatus.s.PXINF == 0) ? DRV_BA414E_OP_SUCCESS : DRV_BA414E_OP_POINT_AT_INFINITY;
        DRV_BA414E_copyFromScm2(cd->eccPointAdditionParams.outX, len, BA414E_ECCP_SLOT_P3X, 0, 0, 0);
        DRV_BA414E_copyFromScm2(cd->eccPointAdditionParams.outY, len, BA414E_ECCP_SLOT_P3Y, 0, 0, 0);
        
        if (cd->callback != 0)
        {            
            (cd->callback)(opRes, cd->context);
        }
    }  
}

void DRV_BA414E_ProcessPrimEccPointMultiplication(DRV_BA414E_ClientData * cd)
{

    uint32_t len = cd->eccPointMultiplicationParams.domain->keySize;
    BA414E__PKSTATUSbits currentStatus;
    currentStatus.v = opData.lastStatus;
    
    cd->currentOp = DRV_BA414E_OP_NONE;
    if ((opData.errorInterrupt == 1))
    {
        BA414E__PKSTATUSbits currentStatus;
        currentStatus.v = opData.lastStatus;
        DRV_BA414E_OP_RESULT opRes = (currentStatus.s.PXINF == 0) ? DRV_BA414E_OP_ERROR : DRV_BA414E_OP_ERROR_POINT_AT_INFINITY;
        if (cd->callback != 0)
        {            
            (cd->callback)(opRes, cd->context);
        }
    }
    else if (opData.doneInterrupt == 1)
    {
        DRV_BA414E_OP_RESULT opRes = (currentStatus.s.PXINF == 0) ? DRV_BA414E_OP_SUCCESS : DRV_BA414E_OP_POINT_AT_INFINITY;
        DRV_BA414E_copyFromScm2(cd->eccPointMultiplicationParams.outX, len, BA414E_ECCP_SLOT_P3X, 0, 0, 0);
        DRV_BA414E_copyFromScm2(cd->eccPointMultiplicationParams.outY, len, BA414E_ECCP_SLOT_P3Y, 0, 0, 0);
        
        if (cd->callback != 0)
        {            
            (cd->callback)(opRes, cd->context);
        }
    }  
}

void DRV_BA414E_ProcessEccCheckPointOnCurve(DRV_BA414E_ClientData * cd)
{
    BA414E__PKSTATUSbits currentStatus;
    currentStatus.v = opData.lastStatus;
    
    cd->currentOp = DRV_BA414E_OP_NONE;
    if ((currentStatus.s.PXNOC == 1))
    {
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_POINT_NOT_ON_CURVE, cd->context);
        }        
    }
    else if ((opData.errorInterrupt == 1))
    {
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_ERROR, cd->context);
        }
    }
    else if (opData.doneInterrupt == 1)
    {
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_SUCCESS, cd->context);
        }
    }
}

void DRV_BA414E_ProcessPrimModOp(DRV_BA414E_ClientData * cd)
{

    uint32_t len = cd->modOperationParams.opSize * 8;
//     bytesToString(domain.a, domain.keySize, 4, 16);
//    snprintf(dbgBufferPtr, debugBufferSize, "%s\r\n%s: A: \n\r%s", dbgBufferPtr, __FUNCTION__, byteString);    
    //snprintf(dbgBufferPtr, debugBufferSize, "%s\r\n%s: PKSTATUS %08X Done %d Error %d\r\n", dbgBufferPtr, __FUNCTION__, PKSTATUS, opData.doneInterrupt, opData.errorInterrupt);
    cd->currentOp = DRV_BA414E_OP_NONE;
    if ((opData.errorInterrupt == 1))
    {
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_ERROR, cd->context);
        }
    }
    else if (opData.doneInterrupt == 1)
    {
        DRV_BA414E_copyFromScm2(cd->modOperationParams.c, len, BA414E_MODP_SLOT_C, 0, 0, 0);
        
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_SUCCESS, cd->context);
        }
    }  
}

void DRV_BA414E_ProcessRsaModExp(DRV_BA414E_ClientData * cd)
{

    uint32_t len = cd->modExpParams.opSize * 8;
    
    cd->currentOp = DRV_BA414E_OP_NONE;
    if ((opData.errorInterrupt == 1))
    {
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_ERROR, cd->context);
        }
    }
    else if (opData.doneInterrupt == 1)
    {
        DRV_BA414E_copyFromScm2(cd->modExpParams.C, len, BA414E_RSA_MODEXP_C, 0, 0, 0);
        
        if (cd->callback != 0)
        {            
            (cd->callback)(DRV_BA414E_OP_SUCCESS, cd->context);
        }
    }  
}

void DRV_BA414E_Process(DRV_BA414E_ClientData * cd)
{
    PKCONTROL = 0;    
    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1);
    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1_FAULT);
    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1);
    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1_FAULT);
    switch(cd->currentOp)
    {
        case DRV_BA414E_OP_ECDSA_SIGN:
            DRV_BA414E_ProcessEcdsaSign(cd);
            break;            
        case DRV_BA414E_OP_ECDSA_VERIFY:
            DRV_BA414E_ProcessEcdsaVerify(cd);
            break;
        case DRV_BA414E_OP_PRIM_ECC_POINT_DOUBLE:
            DRV_BA414E_ProcessPrimEccPointDouble(cd);
            break;
        case DRV_BA414E_OP_PRIM_ECC_POINT_ADDITION:
            DRV_BA414E_ProcessPrimEccPointAddition(cd);
            break;
        case DRV_BA414E_OP_PRIM_ECC_POINT_MULTIPLICATION:
            DRV_BA414E_ProcessPrimEccPointMultiplication(cd);
            break;
        case DRV_BA414E_OP_PRIM_ECC_CHECK_POINT_ON_CURVE:
            DRV_BA414E_ProcessEccCheckPointOnCurve(cd);
            break;
        case DRV_BA414E_OP_PRIM_MOD_ADDITION:
        case DRV_BA414E_OP_PRIM_MOD_SUBTRACTION:
        case DRV_BA414E_OP_PRIM_MOD_MULTIPLICATION:
            DRV_BA414E_ProcessPrimModOp(cd);
            break;
        case DRV_BA414E_OP_PRIM_MOD_EXP:
            DRV_BA414E_ProcessRsaModExp(cd);
            break;
        case DRV_BA414E_OP_NONE:
        default:
            break;
    }

}

void DRV_BA414E_Tasks(SYS_MODULE_OBJ obj)
{
    if (obj == (SYS_MODULE_OBJ)&opData)
    {
        switch (opData.state)
        {
            case DRV_BA414E_INITIALIZE:
                DRV_BA414E_InitialStateHandler();
                opData.state = DRV_BA414E_READY;
                break;
            case DRV_BA414E_READY:
            {
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Pend(&opData.clientAction, OSAL_WAIT_FOREVER);
#endif
                int counter;
                for (counter = 0; counter < DRV_BA414E_NUM_CLIENTS; counter++)
                {
                    if (clientData[counter].currentOp != DRV_BA414E_OP_NONE)
                    {                        
                        opData.state = DRV_BA414E_PREPARING;
                        opData.currentClient = &clientData[counter];
                        break;
                    }
                }
            }
            break;
            case DRV_BA414E_PREPARING:
            {
                int counter;
                for (counter = 0; counter < DRV_BA414E_NUM_CLIENTS; counter++)
                {
                    if (clientData[counter].currentOp != DRV_BA414E_OP_NONE)
                    {
                        opData.state = DRV_BA414E_WAITING;
                        DRV_BA414E_Prepare(&clientData[counter]);
                        break;
                    }
                }                
            }   
            break;
            case DRV_BA414E_WAITING:
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Pend(&opData.wfi, OSAL_WAIT_FOREVER);
#endif
                if ((opData.doneInterrupt == 1) || (opData.errorInterrupt == 1))
                {
                    opData.state = DRV_BA414E_PROCESSING;
                }
                break;
            case DRV_BA414E_PROCESSING:
                DRV_BA414E_Process(opData.currentClient);
                opData.state = DRV_BA414E_READY;                
                break;
            default:
                break;
        }
    }
}

void DRV_BA414_BlockingCallback(DRV_BA414E_OP_RESULT result, uintptr_t context)
{
    DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData*)context;
    cd->blockingResult = result;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
    OSAL_SEM_Post(&cd->clientBlock);
#endif
}

DRV_BA414E_OP_RESULT DRV_BA414_BlockingHelper(DRV_BA414E_ClientData * cd)
{
    cd->context = (uintptr_t)cd;
    cd->callback = DRV_BA414_BlockingCallback;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
    OSAL_SEM_Post(&opData.clientAction);
    OSAL_SEM_Pend(&cd->clientBlock, OSAL_WAIT_FOREVER);
#endif
    return cd->blockingResult;
}

DRV_BA414E_OP_RESULT DRV_BA414E_ECDSA_Sign(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * R,
    uint8_t * S,
    const uint8_t * privateKey,
    const uint8_t * k,
    const uint8_t * msgHash,
    int msgHashSz,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context  
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->ecdsaSignParams, 0, sizeof(DRV_BA414E_ecdsaSignOpParams));
            cd->ecdsaSignParams.domain = domain;
            cd->ecdsaSignParams.R = R;
            cd->ecdsaSignParams.S = S;
            cd->ecdsaSignParams.privateKey = privateKey;
            cd->ecdsaSignParams.k = k;
            cd->ecdsaSignParams.msgHash = msgHash;
            cd->ecdsaSignParams.msgHashSz = msgHashSz;
            cd->currentOp = DRV_BA414E_OP_ECDSA_SIGN;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }
    return ret;
}




DRV_BA414E_OP_RESULT DRV_BA414E_ECDSA_Verify(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    const uint8_t * publicKeyX,
    const uint8_t * publicKeyY,
    uint8_t * R,
    uint8_t * S,
    const uint8_t * msgHash,
    int msgHashSz,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context  
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->ecdsaVerifyParams, 0, sizeof(DRV_BA414E_ecdsaVerifyOpParams));
            cd->ecdsaVerifyParams.domain = domain;
            cd->ecdsaVerifyParams.R = R;
            cd->ecdsaVerifyParams.S = S;
            cd->ecdsaVerifyParams.publicKeyX = publicKeyX;
            cd->ecdsaVerifyParams.publicKeyY = publicKeyY;
            cd->ecdsaVerifyParams.msgHash = msgHash;
            cd->ecdsaVerifyParams.msgHashSz = msgHashSz;
            cd->currentOp = DRV_BA414E_OP_ECDSA_VERIFY;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }
    return ret;
    
}


DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccPointDouble(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * outX,
    uint8_t * outY,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->eccPointDoubleParams, 0, sizeof(DRV_BA414E_primEccPointDoubleOpParams));
            cd->eccPointDoubleParams.domain = domain;
            cd->eccPointDoubleParams.outX = outX;
            cd->eccPointDoubleParams.outY = outY;
            cd->eccPointDoubleParams.p1X = p1X;
            cd->eccPointDoubleParams.p1Y = p1Y;
            cd->currentOp = DRV_BA414E_OP_PRIM_ECC_POINT_DOUBLE;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }   
    return ret;

}

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccPointAddition(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * outX,
    uint8_t * outY,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    const uint8_t * p2X,
    const uint8_t * p2Y,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->eccPointAdditionParams, 0, sizeof(DRV_BA414E_primEccPointAdditionOpParams));
            cd->eccPointAdditionParams.domain = domain;
            cd->eccPointAdditionParams.outX = outX;
            cd->eccPointAdditionParams.outY = outY;
            cd->eccPointAdditionParams.p1X = p1X;
            cd->eccPointAdditionParams.p1Y = p1Y;
            cd->eccPointAdditionParams.p2X = p2X;
            cd->eccPointAdditionParams.p2Y = p2Y;
            cd->currentOp = DRV_BA414E_OP_PRIM_ECC_POINT_ADDITION;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }   
    return ret;    
}

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccPointMultiplication(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    uint8_t * outX,
    uint8_t * outY,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    const uint8_t * k,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->eccPointMultiplicationParams, 0, sizeof(DRV_BA414E_primEccPointMultiplicationOpParams));
            cd->eccPointMultiplicationParams.domain = domain;
            cd->eccPointMultiplicationParams.outX = outX;
            cd->eccPointMultiplicationParams.outY = outY;
            cd->eccPointMultiplicationParams.p1X = p1X;
            cd->eccPointMultiplicationParams.p1Y = p1Y;
            cd->eccPointMultiplicationParams.k = k;
            cd->currentOp = DRV_BA414E_OP_PRIM_ECC_POINT_MULTIPLICATION;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }   
    return ret;
    
}

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_EccCheckPointOnCurve(
    const DRV_HANDLE handle,
    const DRV_BA414E_ECC_DOMAIN * domain,
    const uint8_t * p1X,
    const uint8_t * p1Y,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->eccCheckPointOnCurveParams, 0, sizeof(DRV_BA414E_primEccCheckPointOnCurveOpParams));
            cd->eccCheckPointOnCurveParams.domain = domain;
            cd->eccCheckPointOnCurveParams.p1X = p1X;
            cd->eccCheckPointOnCurveParams.p1Y = p1Y;
            cd->currentOp = DRV_BA414E_OP_PRIM_ECC_CHECK_POINT_ON_CURVE;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }   
    return ret;
    
}

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModAddition(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * c,
    const uint8_t * p,
    const uint8_t * a,
    const uint8_t * b,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->modOperationParams, 0, sizeof(DRV_BA414E_primModOperationParams));
            cd->modOperationParams.opSize = opSize;
            cd->modOperationParams.c = c;
            cd->modOperationParams.p = p;
            cd->modOperationParams.a = a;
            cd->modOperationParams.b = b;
            cd->currentOp = DRV_BA414E_OP_PRIM_MOD_ADDITION;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }   
    return ret;
    
}

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModSubtraction(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * c,
    const uint8_t * p,
    const uint8_t * a,
    const uint8_t * b,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->modOperationParams, 0, sizeof(DRV_BA414E_primModOperationParams));
            cd->modOperationParams.opSize = opSize;
            cd->modOperationParams.c = c;
            cd->modOperationParams.p = p;
            cd->modOperationParams.a = a;
            cd->modOperationParams.b = b;
            cd->currentOp = DRV_BA414E_OP_PRIM_MOD_SUBTRACTION;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }   
    return ret;
    
}

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModMultiplication(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * c,
    const uint8_t * p,
    const uint8_t * a,
    const uint8_t * b,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->modOperationParams, 0, sizeof(DRV_BA414E_primModOperationParams));
            cd->modOperationParams.opSize = opSize;
            cd->modOperationParams.c = c;
            cd->modOperationParams.p = p;
            cd->modOperationParams.a = a;
            cd->modOperationParams.b = b;
            cd->currentOp = DRV_BA414E_OP_PRIM_MOD_MULTIPLICATION;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }   
    return ret;  
}

DRV_BA414E_OP_RESULT DRV_BA414E_PRIM_ModExponentiation(
    const DRV_HANDLE handle,
    DRV_BA414E_OPERAND_SIZE opSize,    // Hardware operand size
    uint8_t * C,
    const uint8_t * n,
    const uint8_t * M,
    const uint8_t * e,
    DRV_BA414E_CALLBACK callback,
    uintptr_t context          
)
{
    DRV_BA414E_RESULT ret = DRV_BA414E_OP_ERROR;
    
    if (handle != DRV_HANDLE_INVALID)    
    {
        DRV_BA414E_ClientData * cd = (DRV_BA414E_ClientData *)handle;
        if (cd->currentOp == DRV_BA414E_OP_NONE)
        {
            memset(&cd->modExpParams, 0, sizeof(DRV_BA414E_primModExpOpParams));
            cd->modExpParams.opSize = opSize;
            cd->modExpParams.C = C;
            cd->modExpParams.n = n;
            cd->modExpParams.M = M;
            cd->modExpParams.e = e;
            cd->currentOp = DRV_BA414E_OP_PRIM_MOD_EXP;
            if ((cd->ioIntent & DRV_IO_INTENT_NONBLOCKING) == DRV_IO_INTENT_NONBLOCKING)
            {
                cd->callback = callback;
                cd->context = context;
#if defined(DRV_BA414E_RTOS_STACK_SIZE)
                OSAL_SEM_Post(&opData.clientAction);
#endif
                ret = DRV_BA414E_OP_PENDING;
            }
            else
            {
                ret = DRV_BA414_BlockingHelper(cd);
            }
        }
    }   
    return ret;  
    
}
