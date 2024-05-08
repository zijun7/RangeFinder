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
int thresholdInput(int threshold_range);
extern unsigned threshold_range = 400; 
static bool buttonPressed = false;

void initialize_controls(void) {
    Ping = false;
    pulse_requested = false;
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

    if (operationMode == SINGLE_PULSE_OPERATION) {
        bool buttonState = cowpi_debounce_byte(cowpi_left_button_is_pressed(), LEFT_BUTTON_DOWN);
        if (buttonState != buttonPressed) {
            buttonPressed = buttonState;
            if (buttonPressed) {
                pulse_requested = true;
            }
        }
    } else if (operationMode == THRESHOLD_ADJUSTMENT) {
            threshold_range = thresholdInput(threshold_range);
        Ping = false;
        pulse_requested = false;
    }

    
    sprintf(buffer, "Range: %d", threshold_range);
    display_string(4, buffer);
    /*if (operationMode != pMode){
        pMode = operationMode;
        sprintf(buffer, "%d  %d", operationMode, pMode);
        display_string(3,buffer);
        sprintf(buffer, "%d", threshold_range);
        display_string(4, buffer);
    }*/
}

int thresholdInput(int threshold_range){

    int a = threshold_range;
    unsigned int threshold;
    sprintf(buffer,"Input threshold");
    display_string(0, buffer);
    sprintf (buffer, "%2d<= input <=%3d", 50, 400);
    display_string(1, buffer);
    char input;
         do{
            threshold = 0;
            do{
                refresh_display();
                while((input = cowpi_debounce_byte(cowpi_get_keypress(), KEYPAD)) != '\0') {};
                do{
                    if (!cowpi_debounce_byte(cowpi_left_switch_is_in_right_position(), LEFT_SWITCH_RIGHT) ||!cowpi_debounce_byte(cowpi_right_switch_is_in_right_position(), RIGHT_SWITCH_RIGHT)) {
                        return a;
                    }
                    input = cowpi_debounce_byte(cowpi_get_keypress(), KEYPAD);
                }while(!(input == '#' || (input >= '0' && input <= '9')));
                if (input >= '0' && input <= '9' && (10 * threshold + (input - '0')) < 1000){
                    threshold = 10 * threshold + (input - '0');
                    sprintf(buffer, "Input     %3dcm", threshold);
                    display_string(2,buffer);
                }
            }while (input != '#');
            if ( threshold < 50 || threshold > 400){
                sprintf (buffer, "Invalid input");
                display_string(2, buffer);
            }
        }while (threshold < 50 || threshold > 400);
        sprintf(buffer,"Threshold  %3dcm", threshold);
        display_string(2, buffer);
        
    a = threshold;
    return threshold;

}
