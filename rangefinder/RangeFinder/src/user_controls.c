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
static char buffer[17] = {0};
unsigned int thresholdInput(void);
extern unsigned threshold_range = 100; 

void initialize_controls(void) {
    Ping = false;
}

void manage_controls(void) {
    static operation_mode pMode = -1;

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
                    pulse_requested = true;
                }
            }
            break;
        case THRESHOLD_ADJUSTMENT:
            if (operationMode != pMode){
                threshold_range = thresholdInput();
            }
            Ping = false;
            pulse_requested = false;
            break;
        case CONTINUOUS_TONE:
            break;
        case NORMAL_OPERATION:
            break;
        default:
            break;
    }

    if (operationMode != pMode){
        pMode = operationMode;
        /*sprintf(buffer, "%d  %d", operationMode, pMode);
        display_string(3,buffer);
        sprintf(buffer, "%d", threshold_range);
        display_string(4, buffer);*/
    }
}

    unsigned int thresholdInput(void){
        buffer[0] = '\0';
        display_string(2, buffer);
        unsigned int range;
        do{
            sprintf(buffer,"Input threshold");
            display_string(0, buffer);
            range = 0;
            char c;
            do{
                refresh_display();
                while((c = cowpi_debounce_byte(cowpi_get_keypress(), KEYPAD)) != '\0') {};
                do{
                    c = cowpi_debounce_byte(cowpi_get_keypress(), KEYPAD);
                    if (!cowpi_debounce_byte(cowpi_left_switch_is_in_right_position(), LEFT_SWITCH_RIGHT) ||!cowpi_debounce_byte(cowpi_right_switch_is_in_right_position(), RIGHT_SWITCH_RIGHT)) {
                        return range;
                    }
                }while(!(c == '#' || (c >= '0' && c <= '9')));
                if (c >= '0' && c <= '9'){
                    range = 10 * range + (c - '0');
                    if (range < 1000){
                        sprintf(buffer, "Range:     %3ucm", range);
                        display_string(1,buffer);
                    }
                    while ((c = cowpi_debounce_byte(cowpi_get_keypress(), KEYPAD)) != '\0') {};
                }
            }while (c != '#');
            if ( range < 50 || range > 400){
                sprintf (buffer, "%2d < rng < %3d", 49, 401);
                display_string(0,buffer);
            }
        }while (range < 50 || range > 400);
        sprintf(buffer,"Threshold  %3ucm", range);
        display_string(1, buffer);
        return range;
    }
