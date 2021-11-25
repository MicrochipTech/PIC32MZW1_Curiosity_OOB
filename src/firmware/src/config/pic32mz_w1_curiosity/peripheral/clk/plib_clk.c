/*******************************************************************************
  SYS CLK Static Functions for Clock System Service

  Company:
    Microchip Technology Inc.

  File Name:
    plib_clk.c

  Summary:
    SYS CLK static function implementations for the Clock System Service.

  Description:
    The Clock System Service provides a simple interface to manage the
    oscillators on Microchip microcontrollers. This file defines the static
    implementation for the Clock System Service.

  Remarks:
    Static functions incorporate all system clock configuration settings as
    determined by the user via the Microchip Harmony Configurator GUI.
    It provides static version of the routines, eliminating the need for an
    object ID or object handle.

    Static single-open interfaces also eliminate the need for the open handle.

*******************************************************************************/

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

// *****************************************************************************
// *****************************************************************************
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include "device.h"
#include "plib_clk.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void CLK_Initialize( void )

  Summary:
    Initializes hardware and internal data structure of the System Clock.

  Description:
    This function initializes the hardware and internal data structure of System
    Clock Service.

  Remarks:
    This is configuration values for the static version of the Clock System
    Service module is determined by the user via the MHC GUI.

    The objective is to eliminate the user's need to be knowledgeable in the
    function of the 'configuration bits' to configure the system oscillators.
*/

static void DelayMs ( uint32_t delay_ms)
{
    uint32_t startCount, endCount;
    /* Calculate the end count for the given delay */
    endCount=(CORE_TIMER_FREQ/1000)*delay_ms;
    startCount=_CP0_GET_COUNT();
    while((_CP0_GET_COUNT()-startCount)<endCount);
}

 void wifi_spi_write(unsigned int spi_addr, unsigned int data)
{
    unsigned int  addr_bit, data_bit, bit_idx;
    unsigned int cs_high, clk_high, en_bit_bang;
    unsigned int *wifi_spi_ctrl_reg = (unsigned int *)0xBF8C8028;
    clk_high = 0x1 ;
    cs_high  = 0x2;
    en_bit_bang  = 0x1 << 31;
    addr_bit = 0; data_bit = 0;

    *wifi_spi_ctrl_reg = en_bit_bang | cs_high ;
    *wifi_spi_ctrl_reg = (en_bit_bang | cs_high | clk_high );
     *wifi_spi_ctrl_reg = (en_bit_bang);
     *wifi_spi_ctrl_reg = (en_bit_bang | clk_high);

    for (bit_idx=0;bit_idx<=7;bit_idx++) {
        addr_bit = (spi_addr>>(7-bit_idx)) & 0x1;
        *wifi_spi_ctrl_reg = (en_bit_bang | (addr_bit << 2 ));               // Falling edge of clk
        *wifi_spi_ctrl_reg = (en_bit_bang | (addr_bit << 2 ) | clk_high);    // Rising edge of clk
    }

    for (bit_idx=0;bit_idx<=15;bit_idx++) {
        data_bit = (data>>(15-bit_idx)) & 0x1;
        *wifi_spi_ctrl_reg = (en_bit_bang | (data_bit << 2 ));                // Falling edge of clk with data bit
        *wifi_spi_ctrl_reg = (en_bit_bang | (data_bit << 2 ) | clk_high);     // Rising edge of clk
    }

    *wifi_spi_ctrl_reg = (en_bit_bang | cs_high | clk_high); // Rising edge of clk
    *wifi_spi_ctrl_reg = 0;                                // Set the RF override bit and CS_n high
}

unsigned int wifi_spi_read(unsigned int spi_addr)
{
    unsigned int  addr_bit, bit_idx, read_data;
    unsigned int cs_high, clk_high, cmd_high, en_bit_bang;
    unsigned int *wifi_spi_ctrl_reg = (unsigned int *)0xBF8C8028;

    clk_high = 0x1 ;
    cs_high  = 0x2;
    cmd_high = 0x8;
    en_bit_bang  = 0x1 << 31;
    addr_bit = 0;


    *wifi_spi_ctrl_reg = (en_bit_bang | cs_high);            // Set the RF override bit and CS_n high
    *wifi_spi_ctrl_reg = (en_bit_bang | cs_high | clk_high); // Rising edge of clk
    *wifi_spi_ctrl_reg = ((en_bit_bang)| cmd_high | (0x1 << 2));                // Falling edge of clk with CS going low and command bit 1
    *wifi_spi_ctrl_reg = ((en_bit_bang | cmd_high | (0x1 << 2) | clk_high));     // Falling edge of clk with CS going low and command bit 1

    for (bit_idx=0;bit_idx<=7;bit_idx++) {
        addr_bit = (spi_addr>>(7-bit_idx)) & 0x1;
        *wifi_spi_ctrl_reg = ((en_bit_bang | cmd_high | (addr_bit << 2 )));               // Falling edge of clk
        *wifi_spi_ctrl_reg =((en_bit_bang | cmd_high | (addr_bit << 2 ) | clk_high));    // Rising edge of clk
    }

    for (bit_idx=0;bit_idx<=16;bit_idx++) {
        *wifi_spi_ctrl_reg = ((en_bit_bang | cmd_high ));               // Falling edge of clk
        *wifi_spi_ctrl_reg = ((en_bit_bang | cmd_high | clk_high));     // Rising edge of clk
    }

    *wifi_spi_ctrl_reg = 0;                                // Set the RF override bit and CS_n high

    read_data = *wifi_spi_ctrl_reg ; //soc_reg_rd(0xBF8C8130,15,0) & 0xFFFF;
    return read_data;
}

void CLK_Initialize( void )
{
    volatile unsigned int *PLLDBG = (unsigned int*) 0xBF8000E0;
    volatile unsigned int *PMDRCLR = (unsigned int *) 0xBF8000B4;
	volatile unsigned int *RFSPICTL = (unsigned int *) 0xBF8C8028;

    /* unlock system for clock configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    if(((DEVID & 0x0FF00000) >> 20) == PIC32MZW1_B0)
    {
		CFGCON2  |= 0x300; // Start with POSC Turned OFF
		/* if POSC was on give some time for POSC to shut off */
		DelayMs(2);
		// Read counter part is there only for debug and testing, or else not needed, so use ifdef as needed
		wifi_spi_write(0x85, 0x00F0); /* MBIAS filter and A31 analog_test */ //if (wifi_spi_read (0x85) != 0xF0) {Error, Stop};
		wifi_spi_write(0x84, 0x0001); /* A31 Analog test */// if (wifi_spi_read (0x84) != 0x1) {Error, Stop};
		wifi_spi_write(0x1e, 0x510); /* MBIAS reference adjustment */ //if (wifi_spi_read (0x1e) != 0x510) {Error, Stop};
		wifi_spi_write(0x82, 0x6400); /* XTAL LDO feedback divider (1.3+v) */ //if (wifi_spi_read (0x82) != 0x6000) {Error, Stop};

		/* Enable POSC */
		CFGCON2  &= 0xFFFFFCFF; // enable POSC

		/* Wait for POSC ready */
		while(!(CLKSTAT & 0x00000004)) ;

		/*Configure SPLL*/
		CFGCON3 = 10;
		CFGCON0bits.SPLLHWMD = 1;

		/* SPLLCON = 0x01496869 */
		/* SPLLBSWSEL   = 1   */
		/* SPLLPWDN     = PLL_ON     */
		/* SPLLPOSTDIV1 = 4 */
		/* SPLLFLOCK    = NO_ASSERT    */
		/* SPLLRST      = NO_ASSERT      */
		/* SPLLFBDIV    = 100  */
		/* SPLLREFDIV   = 5   */
		/* SPLLICLK     = POSC     */
		/* SPLL_BYP     = NO_BYPASS     */
		SPLLCON = 0x1464041;

        /* OSWEN    = SWITCH_COMPLETE    */
		/* SOSCEN   = ON   */
		/* UFRCEN   = USBCLK   */
		/* CF       = NO_FAILDET       */
		/* SLPEN    = IDLE    */
		/* CLKLOCK  = UNLOCKED  */
		/* NOSC     = SPLL     */
		/* WAKE2SPD = SELECTED_CLK */
		/* DRMEN    = NO_EFFECT    */
		/* FRCDIV   = OSC_FRC_DIV_1   */
		OSCCON = 0x102;

		OSCCONSET = _OSCCON_OSWEN_MASK;  /* request oscillator switch to occur */

		while( OSCCONbits.OSWEN );
        DelayMs(5);

		/* Configure EWPLL */
		/* EWPLLBSWSEL   = 6 */
		/* EWPLLPWDN     = PLL_ON */
		/* EWPLLPOSTDIV1 = 32 */
		/* EWPLLFLOCK    = NO_FORCE */
		/* EWPLLRST      = NORESET_EWPLL */
		/* EWPLLFBDIV    = 800 */
		/* EWPLLREFDIV   = 20 */
		/* EWPLLICLK     = POSC */
		/* ETHCLKOUTEN   = ENABLED */
		/* EWPLL_BYP     = NO_BYPASS */
		EWPLLCON = 0x15320206 ^ 0x0438080c;
		CFGCON0bits.ETHPLLHWMD = 1;
		while(!((*PLLDBG) & 0x4));

		/* Configure UPLL */
		/* UPLLBSWSEL   = 5 */
		/* UPLLPWDN     = PLL_ON */
		/* UPLLPOSTDIV1 = 10 */
		/* UPLLFLOCK    = NOFORCE_LOCK */
		/* UPLLRST      = DEASSERT_RST */
		/* UPLLFBDIV    = 24 */
		/* UPLLREFDIV   = 1 */
		/* UPLL_BYP     = UPLL */
		UPLLCON = 0x404180a5;

		/* Power down the BTPLL */
		BTPLLCONbits.BTPLLPWDN = 1;

        *(PMDRCLR)  = 0x1000;
    }
    else if(((DEVID & 0x0FF00000) >> 20) == PIC32MZW1_A1)
    {

		CFGCON2  |= 0x300; // Start with POSC Turned OFF
		DelayMs(2);

		/* make sure we properly reset SPI to a known state */
		*RFSPICTL = 0x80000022;
		/* now wifi is properly reset enable POSC */
		CFGCON2  &= 0xFFFFFCFF; // enable POSC

		DelayMs(2);

        /* make sure we properly take out of reset */
        *RFSPICTL = 0x80000002;

        wifi_spi_write(0x85, 0x00F0); // MBIAS filter and A31 analog_test
        wifi_spi_write(0x84, 0x0001); // A31 Analog test
        wifi_spi_write(0x1e, 0x510); // MBIAS reference adjustment
        wifi_spi_write(0x82, 0x6000); // XTAL LDO feedback divider (1.3+v)

		 /* Wait for POSC ready */
        while(!(CLKSTAT & 0x00000004)) ;

    	OSCCONbits.FRCDIV = 0;

		CFGCON3 = 10;
        CFGCON0bits.SPLLHWMD = 1;
		/* SPLLCON = 0x01496869 */
		/* SPLLBSWSEL   = 1   */
		/* SPLLPWDN     = PLL_ON     */
		/* SPLLPOSTDIV1 = 4 */
		/* SPLLFLOCK    = NO_ASSERT    */
		/* SPLLRST      = NO_ASSERT      */
		/* SPLLFBDIV    = 100  */
		/* SPLLREFDIV   = 5   */
		/* SPLLICLK     = POSC     */
		/* SPLL_BYP     = NO_BYPASS     */
		SPLLCON = 0x1464041;


		/* Configure UPLL */
		/* UPLLBSWSEL   = 5 */
		/* UPLLPWDN     = PLL_ON */
		/* UPLLPOSTDIV1 = 10 */
		/* UPLLFLOCK    = NOFORCE_LOCK */
		/* UPLLRST      = DEASSERT_RST */
		/* UPLLFBDIV    = 24 */
		/* UPLLREFDIV   = 1 */
		/* UPLL_BYP     = UPLL */
		UPLLCON = 0x404180a5;

		/* Configure EWPLL */
		/* EWPLLBSWSEL   = 6 */
		/* EWPLLPWDN     = PLL_ON */
		/* EWPLLPOSTDIV1 = 32 */
		/* EWPLLFLOCK    = NO_FORCE */
		/* EWPLLRST      = NORESET_EWPLL */
		/* EWPLLFBDIV    = 800 */
		/* EWPLLREFDIV   = 20 */
		/* EWPLLICLK     = POSC */
		/* ETHCLKOUTEN   = ENABLED */
		/* EWPLL_BYP     = NO_BYPASS */
		EWPLLCON = 0x15320206 ^ 0x438080c;
		CFGCON0bits.ETHPLLHWMD = 1;
		while(!((*PLLDBG) & 0x4));

		/* Power down the BTPLL */
		BTPLLCONbits.BTPLLPWDN = 1;

		/* ETHPLLPOSTDIV2 = a */
		/* SPLLPOSTDIV2   = 0 */
		/* BTPLLPOSTDIV2  = 0 */
		CFGCON3 = 10;

		/* OSWEN    = SWITCH_COMPLETE    */
		/* SOSCEN   = ON   */
		/* UFRCEN   = USBCLK   */
		/* CF       = NO_FAILDET       */
		/* SLPEN    = IDLE    */
		/* CLKLOCK  = UNLOCKED  */
		/* NOSC     = SPLL     */
		/* WAKE2SPD = SELECTED_CLK */
		/* DRMEN    = NO_EFFECT    */
		/* FRCDIV   = OSC_FRC_DIV_1   */
		OSCCON = 0x102;

		OSCCONSET = _OSCCON_OSWEN_MASK;  /* request oscillator switch to occur */

		Nop();
		Nop();

		while( OSCCONbits.OSWEN );        /* wait for indication of successful clock change before proceeding */
	}

  

    /* Peripheral Module Disable Configuration */

    CFGCON0bits.PMDLOCK = 0;

    PMD1 = 0x20008800;
    PMD2 = 0x780d0f;
    PMD3 = 0x18010214;

    CFGCON0bits.PMDLOCK = 1;

    /* Lock system since done with clock configuration */
    SYSKEY = 0x33333333;
}
