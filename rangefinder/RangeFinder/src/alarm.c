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
    threshold_range = 100;
    register_timer_ISR(ALARM_TIMER, 500, TimerInterruptHandler);
}

void manage_alarm(void) {

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
            shouldSoundAlarm = false;
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
