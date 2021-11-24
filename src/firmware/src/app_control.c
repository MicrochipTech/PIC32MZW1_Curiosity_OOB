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
#include "bsp/bsp.h"
#include "peripheral/wdt/plib_wdt.h"
#include "peripheral/tmr/plib_tmr2.h"
#include "peripheral/ocmp/plib_ocmp2.h"
#include "peripheral/adchs/plib_adchs_common.h"
#include "system/console/sys_console.h"
#include "peripheral/rtcc/plib_rtcc.h"
#include "peripheral/adchs/plib_adchs.h"
#include "peripheral/tmr/plib_tmr3.h"

APP_CONTROL_DATA app_controlData;

void APP_CONTROL_Initialize(void) {
    app_controlData.wifiCtrl.wifiConnected = false;
    app_controlData.wifiCtrl.wifiCtrlValid = false;
    app_controlData.wifiCtrl.wifiCtrlChanged = false;
    app_controlData.serialNumValid=false;
    app_controlData.devSerialStr[0] = '\0'; //to indicate valid serial number when populated from msd_app
    app_controlData.rssiData.assocHandle = 0;

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
    app_controlData.adcData.dataReady = false;
    app_controlData.adcData.adcCount = 0;
    app_controlData.adcData.temp = 0;

    WDT_Enable();
}

static volatile bool rtcc_alarm = false;

void RTCC_Callback(uintptr_t context) {
    rtcc_alarm = true;
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

void ADC_ResultHandler(ADCHS_CHANNEL_NUM channel, uintptr_t context) {
    /* Read the ADC result */
    app_controlData.adcData.adcCount = ADCHS_ChannelResultGet(ADCHS_CH15);
    app_controlData.adcData.dataReady = true;
}

static void setup_rtcc(void) {
    struct tm sys_time;
    struct tm alarm_time;

    // Time setting 31-12-2019 23:59:58 Monday
    sys_time.tm_hour = 0;
    sys_time.tm_min = 0;
    sys_time.tm_sec = 0;

    sys_time.tm_year = 0;
    sys_time.tm_mon = 1;
    sys_time.tm_mday = 1;
    sys_time.tm_wday = 0;

    // Alarm setting 01-01-2020 00:00:05 Tuesday
    alarm_time.tm_hour = 00;
    alarm_time.tm_min = 00;
    alarm_time.tm_sec = 01;

    alarm_time.tm_year = 0;
    alarm_time.tm_mon = 1;
    alarm_time.tm_mday = 1;
    alarm_time.tm_wday = 0;

    RTCC_CallbackRegister(RTCC_Callback, (uintptr_t) NULL);

    if (RTCC_TimeSet(&sys_time) == false) {
        /* Error setting up time */
        SYS_CONSOLE_PRINT("RTCC: "TERM_RED"Error setting time\r\n"TERM_RESET);
        return;
    }
    RTCC_ALARM_MASK mask;
    mask = RTCC_ALARM_MASK_SECOND;

    if (RTCC_AlarmSet(&alarm_time, mask) == false) {
        /* Error setting up alarm */
        SYS_CONSOLE_PRINT("RTCC: "TERM_RED"Error setting alarm\r\n"TERM_RESET);
    }
}

void APP_CONTROL_Tasks(void) {
    WDT_Clear();
    switch (app_controlData.state) {
        case APP_CONTROL_STATE_INIT:
        {
            ADCHS_CallbackRegister(ADCHS_CH15, ADC_ResultHandler, (uintptr_t) NULL);
            TMR3_Start(); /*TMR3 is used for ADC trigger*/

            RTCC_CallbackRegister(RTCC_Callback, (uintptr_t) NULL);
            setup_rtcc();
            indicator_on();
            app_controlData.state = APP_CONTROL_STATE_MONITOR_CONNECTION;
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
            /*Average over 100 ADC samples*/
            static uint32_t adcCountAccumulate = 0;
            static uint16_t adcAccumulateNum = 0;
            if (app_controlData.adcData.dataReady) {
                if (adcAccumulateNum <= APP_CTRL_ADC_AVG_COUNT) {
                    adcCountAccumulate += app_controlData.adcData.adcCount;
                    adcAccumulateNum++;
                } else {
                    adcCountAccumulate = adcCountAccumulate / APP_CTRL_ADC_AVG_COUNT;
                    float input_voltage = (float) adcCountAccumulate * APP_CTRL_ADC_VREF / APP_CTRL_ADC_MAX_COUNT;
                    float temp = ((input_voltage - .7) / .1)*10;
                    app_controlData.adcData.temp = temp;
                    /*For the next averaging cycle*/
                    adcAccumulateNum = 0;
                    adcCountAccumulate = 0;
                    //SYS_CONSOLE_PRINT("Temp=%0.1f\r\n",app_controlData.adcData.temp);
                }
                app_controlData.adcData.dataReady = false;
            }
            app_controlData.state = APP_CONTROL_STATE_RTCC_READ;
            break;
        }
        case APP_CONTROL_STATE_RTCC_READ:
        {
            if (rtcc_alarm) {
                rtcc_alarm = false;
                RTCC_TimeGet(&app_controlData.rtccData.sys_time);
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
