/**************************************************************************//**
 *
 * @file user_controls.c
 *
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 *
 * @brief Code to get inputs from the user.
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
#include <string.h>
#include <stdlib.h>
#include "user_controls.h"
#include "shared_variables.h"
#include "outputs.h"
#include "interrupt_support.h"


operation_mode operationMode;
bool volatile Ping;

void initialize_controls(void) {
    Ping = false;
}

void manage_controls(void) {
    //static operation_mode pMode = -1;

    if (cowpi_debounce_byte(cowpi_left_switch_is_in_left_position(), LEFT_SWITCH_LEFT) && cowpi_debounce_byte(cowpi_right_switch_is_in_left_position(),RIGHT_SWITCH_LEFT)){
        operationMode = NORMAL_OPERATION;
    }else if (cowpi_debounce_byte(cowpi_left_switch_is_in_right_position(), LEFT_SWITCH_RIGHT) && cowpi_debounce_byte(cowpi_right_switch_is_in_left_position(),RIGHT_SWITCH_LEFT)){
        operationMode = SINGLE_PULSE_OPERATION;
    }else if (cowpi_debounce_byte(cowpi_left_switch_is_in_right_position(), LEFT_SWITCH_RIGHT) && cowpi_debounce_byte(cowpi_right_switch_is_in_right_position(),RIGHT_SWITCH_RIGHT)){
        operationMode = THRESHOLD_ADJUSTMENT;
    }else if(cowpi_debounce_byte(cowpi_left_switch_is_in_left_position(), LEFT_SWITCH_LEFT) && cowpi_debounce_byte(cowpi_right_switch_is_in_right_position(),RIGHT_SWITCH_RIGHT)){
        operationMode = CONTINUOUS_TONE;
    }


    static bool button_is_pressed = false;
    switch (operationMode)
    {
    case SINGLE_PULSE_OPERATION:
        ;
        bool new_button_position;
        if ((new_button_position = cowpi_debounce_byte(cowpi_left_button_is_pressed(), LEFT_BUTTON_DOWN)) != button_is_pressed){
            button_is_pressed = new_button_position;
            if (button_is_pressed && operationMode == SINGLE_PULSE_OPERATION){
                Ping = true;
            }
        }
        break;
    
    default:
        break;
    }


}

