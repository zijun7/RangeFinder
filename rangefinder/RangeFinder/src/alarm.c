/**************************************************************************//**
 *
 * @file alarm.c
 *
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 *
 * @brief Code to manage the piezodisc and LEDs.
 *
 ******************************************************************************/

/*
 * RangeFinder GroupLab assignment and starter code (c) 2023 Christopher A. Bohn
 * RangeFinder solution (c) the above-named students
 */

#include <CowPi.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "alarm.h"
#include "shared_variables.h"
#include "outputs.h"
#include "interrupt_support.h"


const unsigned int on_period = 100;          // a made-up number, probably not the value you want to use
volatile unsigned int total_period = 50000;
void TimerInterruptHandler(void);
bool volatile Ping;
bool shouldSoundAlarm = false; 
bool shouldIlluminate = false;
static unsigned int ISR_count = 0;
//const unsigned long volatile one_second_has_elapsed = false;

void initialize_alarm(void) {
    threshold_range = 400;
    register_timer_ISR(ALARM_TIMER, 500, TimerInterruptHandler);
}

void manage_alarm(void) {
    unsigned int total_period_milliseconds;
    if (distance < 10){
        total_period_milliseconds = 125;
    }else if (distance < 25){
        total_period_milliseconds = 250;
    }else if (distance < 50){
        total_period_milliseconds = 500;
    }else if (distance < 100){
        total_period_milliseconds = 750;
    }else if (distance < 150){
        total_period_milliseconds = 1000;
    }else if (distance < 200){
        total_period_milliseconds = 1500;
    }else if (distance < 250){
        total_period_milliseconds = 2000;
    }else {
        total_period_milliseconds = 2500;
    }
    total_period = total_period_milliseconds * 2;
}

void TimerInterruptHandler(void){

    ISR_count = ISR_count + 1;
    if (ISR_count > total_period){
        ISR_count = 0;
    }

    static bool buzzerState = false;
    buzzerState = !buzzerState;


    switch(operationMode){
        case NORMAL_OPERATION:
            if (object_detected){
                shouldSoundAlarm = (distance < threshold_range);
                shouldIlluminate = true;
            }else if(ISR_count < total_period){
                shouldIlluminate = false;
                shouldSoundAlarm = false;
            }else {
                ISR_count = 0;
            }
            break;
        case SINGLE_PULSE_OPERATION:
            if (Ping){
                shouldSoundAlarm = (distance <= threshold_range);
                shouldIlluminate = true;
                ISR_count = 0;
                Ping = false;
            } else if (ISR_count >= on_period){
                shouldSoundAlarm = false;
                shouldIlluminate = false;
            }
            break;
        case CONTINUOUS_TONE:
            shouldSoundAlarm = true;
            shouldIlluminate = false;
            ISR_count = 0;
            break;
        case THRESHOLD_ADJUSTMENT:
            shouldSoundAlarm = false;
            shouldIlluminate = false;
            break;
        default:
            break;
    }

    if (shouldSoundAlarm){
        digitalWrite(BUZZER, buzzerState);
    }

    if (shouldIlluminate){
        cowpi_illuminate_right_led();
    }else{
        cowpi_deluminate_right_led();
    }

}
