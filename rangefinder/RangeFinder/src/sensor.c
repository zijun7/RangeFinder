/**************************************************************************//**
 *
 * @file sensor.c
 *
 * @author (DajieQiu)
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 *
 * @brief Code to manage the distance sensor.
 *
 ******************************************************************************/

/*
 * RangeFinder GroupLab assignment and starter code (c) 2023 Christopher A. Bohn
 * RangeFinder solution (c) the above-named students
 */

#include <CowPi.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sensor.h"
#include "shared_variables.h"
#include "outputs.h"
#include "interrupt_support.h"



static int calculate_speed(uint32_t current_distance, uint32_t last_distance, uint32_t current_time, uint32_t last_time);
static void StateChange(void);
static void pulse_edge(void);
static uint32_t get_time(void);
static unsigned int compute_distance(uint32_t microseconds, uint32_t temperature_register);
extern operation_mode operationMode;
bool volatile alarm_requested;
bool volatile object_detected;
unsigned int volatile distance;
int volatile speed;
bool volatile pulse_requested;
static uint32_t volatile pulse_start = 0;
static uint32_t volatile pulse_end;


typedef enum {
    START,
    UP,
    READY,
    LISTENING,
    DETECTED,
    QUIESCENT
} sensor_state_t;

sensor_state_t volatile sensor_state;

void initialize_sensor(void) {
    digitalWrite(TRIGGER, 0);
    sensor_state = START;
    register_pin_ISR(1 << ECHO, pulse_edge);
    register_timer_ISR(SENSOR_TIMER, 32768, StateChange);
}


void manage_sensor(void) {
    static char buffer[17];  
    static uint32_t last_time = 0;  
    static uint32_t last_distance = 0;  
    static bool first_measurement = true;  

    if (pulse_requested) {
        digitalWrite(TRIGGER, 1);
        pulse_requested = false;
        uint32_t time = get_time();
        while (get_time() - time < 10) {}  
        digitalWrite(TRIGGER, 0);
    }

    if (sensor_state == QUIESCENT) {
        
        if (object_detected) {
            uint32_t pulse_time = pulse_end - pulse_start;
            if (operationMode == SINGLE_PULSE_OPERATION) {
                distance = compute_distance(pulse_time, 889); 

                if (distance > 999) {
                    sprintf(buffer, "--no detection--");
                    display_string(0, buffer);
                    display_string(1, ""); 
                } else {
                    sprintf(buffer, "Distance: %3u cm", distance);
                    display_string(0, buffer);
                    display_string(1, ""); 
                }
                sensor_state = READY; // Wait for next pulse request
            } else {
                distance = compute_distance(pulse_time, 889); 
                if (distance > 999) {
                    sprintf(buffer, "--no detection--");
                    display_string(0, buffer);
                    buffer[0] = '\0';  // Clear buffer
                    display_string(1, buffer);
                } else {
                    sprintf(buffer, "Distance: %3u cm", distance);
                    display_string(0, buffer);
                    if (!first_measurement) {
                        uint32_t current_time = get_time();
                        speed = calculate_speed(distance, last_distance, current_time, last_time);
                        sprintf(buffer, "Speed: %4d cm/s", speed);
                        display_string(1, buffer);
                    } else {
                        sprintf(buffer, "Speed: -- cm/s");
                        display_string(1, buffer);
                        first_measurement = false;
                    }
                    last_time = get_time();
                    last_distance = distance;
                }
            }
        } else {
            sprintf(buffer, "--no detection--");
            display_string(0, buffer);
            buffer[0] = '\0';
            display_string(1, buffer);
        }
    } else if (sensor_state == READY && operationMode == NORMAL_OPERATION) {
        pulse_requested = true;
    }
}


static unsigned int compute_distance(uint32_t microseconds, uint32_t temperature_register) {
    return (unsigned int)((uint64_t)microseconds * ((256108888LL - 121907LL * 889)) >> 33);
}

static int calculate_speed(uint32_t current_distance, uint32_t last_distance, uint32_t current_time, uint32_t last_time) {
    if (last_time == 0 || current_time == last_time) return 0;
    int distance_diff = current_distance - last_distance;
    int time_diff = current_time - last_time;
    return (distance_diff * 1000000) / time_diff;
}

static uint32_t get_time(void) {
    cowpi_timer_t volatile *timer = (cowpi_timer_t *)(0x40054000);
    return timer->raw_lower_word;
}


void StateChange(void) {
    if (sensor_state == START || sensor_state == UP || sensor_state == QUIESCENT) {
        sensor_state = READY;
    } else if (sensor_state == LISTENING) {
        object_detected = false;
        sensor_state = QUIESCENT;
    } else if (sensor_state == DETECTED) {
        object_detected = true;
        sensor_state = QUIESCENT;
    }
}



void pulse_edge(void) {
    bool rising_edge = digitalRead(ECHO);
    rising_edge ? (pulse_start = get_time(), sensor_state = LISTENING) :
                  (pulse_end = get_time(), sensor_state = DETECTED);
}

