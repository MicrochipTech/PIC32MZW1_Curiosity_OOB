/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_control.c

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

#include "app_control.h"
#include "math.h"

APP_CONTROL_DATA app_controlData;

void APP_CONTROL_Initialize(void) {
    app_controlData.wifiCtrl.wifiConnected = false;
    app_controlData.wifiCtrl.wifiCtrlValid = false;
    app_controlData.wifiCtrl.wifiCtrlChanged = false;
    app_controlData.devSerialStr[0] = '\0'; //to indicate valid serial number when populated from msd_app

    /*Initialize MQTT control data*/
    app_controlData.mqttCtrl.mqttConfigValid = false;
    app_controlData.mqttCtrl.clientId[0] = '\0';
    app_controlData.mqttCtrl.mqttBroker[0] = '\0';
    app_controlData.mqttCtrl.conStat = false;
    app_controlData.state = APP_CONTROL_STATE_INIT;

    /*Initialize switch status*/
    app_controlData.switchData.switchStatus = false;
    if (SWITCH1_STATE_PRESSED == SWITCH1_Get()) {
        app_controlData.switchData.bootSwitch = true;
    } else {
        app_controlData.switchData.bootSwitch = false;
    }

    /*init ADC data*/
    app_controlData.adcData.dataReady=false;
    app_controlData.adcData.adcCount = 0;
    
    WDT_Enable();
}

void softResetDevice(void) {
    bool int_flag = false;

    /*disable interrupts since we are going to do a sysKey unlock*/
    int_flag = (bool) __builtin_disable_interrupts();

    /* unlock system for clock configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    if (int_flag) {
        __builtin_mtc0(12, 0, (__builtin_mfc0(12, 0) | 0x0001)); /* enable interrupts */
    }

    RSWRSTbits.SWRST = 1;
    /*This read is what actually causes the reset*/
    RSWRST = RSWRSTbits.SWRST;

    /*Reference code. We will not hit this due to reset. This is here for reference.*/
    int_flag = (bool) __builtin_disable_interrupts();

    SYSKEY = 0x33333333;

    if (int_flag) /* if interrupts originally were enabled, re-enable them */ {
        __builtin_mtc0(12, 0, (__builtin_mfc0(12, 0) | 0x0001));
    }

}

static void indicator_fast_blink() {
    TMR2_Stop();
    OCMP2_Disable();
    TMR2_PeriodSet(APP_CTRL_OC_TIMER_PERIOD);
    OCMP2_CompareSecondaryValueSet(APP_CTRL_OC_TIMER_PERIOD / 4);
    OCMP2_Enable();
    TMR2_Start();
}

static void indicator_on() {
    TMR2_Stop();
    OCMP2_Disable();
    TMR2_PeriodSet(APP_CTRL_OC_TIMER_PERIOD);
    OCMP2_CompareSecondaryValueSet(APP_CTRL_OC_TIMER_PERIOD);
    OCMP2_Enable();
    TMR2_Start();
}

static void indicator_off() {
    OCMP2_CompareSecondaryValueSet(0);
}

void ADC_ResultHandler(ADCHS_CHANNEL_NUM channel, uintptr_t context)
{
    /* Read the ADC result */
    app_controlData.adcData.adcCount = ADCHS_ChannelResultGet(ADCHS_CH15);    
    app_controlData.adcData.dataReady = true;
}
void APP_CONTROL_Tasks(void) {
    WDT_Clear();
    switch (app_controlData.state) {
        case APP_CONTROL_STATE_INIT:
        {
            ADCHS_CallbackRegister(ADCHS_CH15, ADC_ResultHandler, (uintptr_t)NULL);
            TMR3_Start(); /*TMR3 is used for ADC trigger*/
            app_controlData.state = APP_CONTROL_STATE_MONITOR_CONNECTION;
            indicator_on();
            break;
        }
        case APP_CONTROL_STATE_MONITOR_CONNECTION:
        {
            if (false == app_controlData.wifiCtrl.wifiConnected) {
                indicator_on();
            } else if (false == app_controlData.mqttCtrl.conStat) {
                indicator_fast_blink();
            } else {
                indicator_off();
            }
            app_controlData.state = APP_CONTROL_STATE_MONITOR_SWITCH;
            break;
        }
        case APP_CONTROL_STATE_MONITOR_SWITCH:
        {
            if (SWITCH1_STATE_PRESSED == SWITCH1_Get()) {
                app_controlData.switchData.switchStatus = true;
            } else {
                app_controlData.switchData.switchStatus = false;
            }
            app_controlData.state = APP_CONTROL_STATE_ADC_READ;
            break;
        }
        case APP_CONTROL_STATE_ADC_READ:
        {
            if (app_controlData.adcData.dataReady){
                float input_voltage = (float)app_controlData.adcData.adcCount * APP_CTRL_ADC_VREF / APP_CTRL_ADC_MAX_COUNT;
                float temp=((input_voltage-.6)/.1)*10;
                app_controlData.adcData.temp=(int)temp;
                //SYS_CONSOLE_PRINT("Temp=%d\r\n",app_controlData.adcData.temp);
                app_controlData.adcData.dataReady=false;
            }
            app_controlData.state = APP_CONTROL_STATE_MONITOR_CONNECTION;
            break;
        }
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
