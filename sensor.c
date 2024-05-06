/**************************************************************************//**
 *
 * @file sensor.c
 *
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
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


/*void handle_sensor_timer(void);
void on_pulse_edge(void);
system_mode_t system_mode;
static uint32_t get_time();
static unsigned int distance58(uint32_t half_micorseconds);
static unsigned int distance70(uint32_t half_micorseconds);

#if defined(ARDUINO_RASPBERRY_PI_PICO)
static unsigned int distance889(uint32_t half_micorseconds);
static unsigned int distance_thermometer(uint32_t microseconds);
static uint32_t read_temperature_register(void);
static int get_temperature(void);

adc_t volatile *thermometer = (adc_t *)(0x4004c000);
#endif
static uint32_t volatile pulse_start_time = 0;
static uint32_t volatile pulse_end_time;
bool volatile alarm_requested;
bool volatile object_detected;
unsigned int volatile distance;
int volatile speed;
void on_pulse_edge(void);
bool volatile pulse_requested;


typedef enum {
    INITIAL_START,
    POWERING_UP,
    READY,
    ACTIVE_LISTENING,
    ACTIVE_DETECTED,
    QUIESCENT
} sensor_state_t;

sensor_state_t volatile sensor_state;


void initialize_sensor(void) {
    digitalWrite(TRIGGER,0);
    sensor_state = INITIAL_START;
    alarm_requested = false;
    object_detected = false;
    speed = 0;
    register_pin_ISR(1 << ECHO, on_pulse_edge);
#if defined(ARDUINO_AVR_NANO) 
    configure_timer(SENSOR_TIMER,32768);
    register_timer_ISR(SENSOR_TIMER,0, handle_sensor_timer);
#elif defined(ARDUINO_RASPBERRY_PI_PICO)  
    register_timer_ISR(SENSOR_TIMER, 32768, handle_sensor_timer);
    thermometer-> control = (1 << 20) | (4 << 12);
    thermometer-> control |= 3;
#endif
}

void manage_sensor(void) {
    static char buffer[17] = {0};
    if(pulse_requested)
    {
        digitalWrite(TRIGGER, 1);
        pulse_requested = false;
        uint32_t then = get_time();
#if defined(ARDUINO_AVR_NANO)
        while(get_time() - then < 20){

        }
#elif defined (ARDUINO_RASPBERRY_PI_PICO)
        while (get_time()- then < 10) 
        {

        }

#endif 
        digitalWrite(TRIGGER, 0);
    }

    if (sensor_state == QUIESCENT)
    {
        if(object_detected)
        {
            alarm_requested = true;
            uint32_t pulse_time = pulse_end_time - pulse_start_time;
            if(system_mode == SINGLE_PULSE_OPERATION)
            {
#if defined(ARDUINO_AVR_NANO)
                unsigned int wrong_distance = distance58(pulse_time);
                if(wrong_distance > 999)
                    wrong_distance = 999;
                sprintf(buffer, "wrong:    %3ucm", wrong_distance);
                display_string(0, buffer);
                unsigned int old_hardware_distance = distance70(pulse_time);
                if(old_hardware_distance > 999)
                    old_hardware_distance = 999;
                sprintf(buffer, "old hw:   %3ucm", old_hardware_distance);
                display_string(1, buffer);
                distance = old_hardware_distance;
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
                unsigned int wrong_distance = distance58(2 * pulse_time);
                if(wrong_distance > 999)
                    wrong_distance = 999;
                sprintf(buffer, "wrong:  %3ucm", wrong_distance);
                display_string(0, buffer);
                unsigned int old_hardware_distance = distance70(2 * pulse_time);
                if(old_hardware_distance > 999)
                    old_hardware_distance = 999;
                sprintf(buffer, "old hw:  %3ucm", old_hardware_distance);
                display_string(1, buffer);
                unsigned int new_hardware_no_bonus_distance = distance889(pulse_time);
                if(new_hardware_no_bonus_distance > 999)
                    new_hardware_no_bonus_distance = 999;
                sprintf(buffer, "no bonus: %3ucm", new_hardware_no_bonus_distance);
                display_string(2, buffer);
                unsigned int new_hardware_with_bonus_distance = distance_thermometer(pulse_time);
                if(new_hardware_with_bonus_distance > 999)
                    new_hardware_with_bonus_distance = 999;
                sprintf(buffer, "bonus:  %3ucm", new_hardware_no_bonus_distance);
                display_string(3, buffer);
                sprintf(buffer, "%-4lu%11dC", read_temperature_register(), get_temperature());
                display_string(4, buffer);
                distance = new_hardware_no_bonus_distance;
#endif
            }
            else if(system_mode == NORMAL_OPERATIONS)
            {
                static uint32_t previous_pulse_time = 0;
                int speed;
#if defined(ARDUINO_AVR_NANO)
                distance = distance70(pulse_time);
                speed = ((uint64_t)(previous_pulse_time - pulse_time) * 281640625LL) >> 31;
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
                distance = distance_thermometer(pulse_time);
                static int one_second_speed = 0;
                if(one_second_has_elapsed)
                {
                    static unsigned int seconds = 0;
                    seconds++;
                    sprintf(buffer,"%9u",seconds);
                    display_string(6,buffer);
                    static unsigned int previous_distance = UINT16_MAX;
                    if(previous_distance == UINT16_MAX){
                        one_second_speed = 0; 
                    }
                    else
                    {
                        one_second_speed = (int)(previous_distance - distance);
                    }
                    previous_distance = distance;
                    one_second_has_elapsed = false;
                }
                speed = one_second_speed;
#endif 
                if (distance >999)
                    distance =999;
                if (distance >9999)
                    distance = 9999;
                else if (speed < -9999)
                    speed = -9999;
                previous_pulse_time = pulse_time;
                sprintf(buffer, "Distance %3ucm", distance);
                display_string(0, buffer);
                sprintf(buffer, "Speed %5d cm/s", speed);
                display_string(1, buffer);
            }
        }
        else
        {
            sprintf(buffer,"--no detection--");
            display_string(0, buffer);
            buffer[0] ='\0';
            display_string(1,buffer);
#if defined(ARDUINO_RASPBERRY_PI_PICO)
            display_string(2,buffer);
            display_string(3, buffer);
#endif
        }
    }
    else if(sensor_state == READY && system_mode == NORMAL_OPERATIONS)
    {
       pulse_requested = true; 
    }

}





void handle_sensor_timer(void)
{
    switch(sensor_state)
    {
    case INITIAL_START:
        sensor_state = POWERING_UP;
        break;
    case POWERING_UP:
        sensor_state = READY;
        break;
    case ACTIVE_LISTENING:
        object_detected = false;
        sensor_state = QUIESCENT;
        break;
    case ACTIVE_DETECTED:
        object_detected = true;
        sensor_state = QUIESCENT;
        break;
    case QUIESCENT:
        sensor_state = READY;
        break;
    default:
        break;
    }
}

void on_pulse_edge(void)
{
    static bool rising_edge = false;
    if(sensor_state == INITIAL_START || sensor_state == POWERING_UP)
    {
        return;
    }
    rising_edge = digitalRead(ECHO);
    if(rising_edge)
    {
        if (sensor_state == READY)
        {
            reset_timer(SENSOR_TIMER);
#if defined(ARDUINO_RASPBERRY_PI_PICO)
            pulse_start_time = get_time();
#endif
            sensor_state = ACTIVE_LISTENING;
        }
    }
    else
    {
        if(sensor_state == ACTIVE_LISTENING)
        {
            pulse_end_time = get_time();
#if defined(ARDUINO_RASPBERRY_PI_PICO)
#endif
            sensor_state = ACTIVE_DETECTED;
        }
    }
}

static uint32_t get_time(void)
{
#if defined(ARDUINO_AVR_NANO)
    cowpi_timerl6bit_t volatile *timer =(cowpi_timer16bit_t *)(0x80);
    return timer->counter;
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
    cowpi_timer_t volatile *timer = (cowpi_timer_t *)(0x40054000);
    return timer->raw_lower_word;
#endif
}

static unsigned int distance58(uint32_t half_microseconds)
{
    return (unsigned int)(half_microseconds /(2 * 58));
}

static unsigned int distance70(uint32_t half_microseconds)
{
    return (unsigned int)((half_microseconds * 18025L)>> 21);
}

#if defined(ARDUINO_RASPBERRY_PI_PICO)
static uint32_t read_temperature_register()
{
    thermometer->control |=(4 << 12);
    thermometer->control |=(1 << 2);
    while ((thermometer->control &(1 << 8))== 0)
    {

    }
    return thermometer->result;
}

static int get_temperature(void)
{
    uint32_t adc_register_value = read_temperature_register();
    double adc_voltage =3.3 * adc_register_value /4096;
    double celsius = 27 -(adc_voltage -0.706)/ 0.001721;
    return (int)round(celsius);
}
static unsigned int compute_distance_using_adc_value(uint32_t microseconds, uint32_t temperature_register)
{
    return (unsigned int)((uint64_t)microseconds *((256108888LL - 121907LL * (uint64_t)temperature_register))>> 33);
}

static unsigned int distance889(uint32_t microseconds)
{
    return compute_distance_using_adc_value(microseconds, 889);
}

static unsigned int distance_thermometer(uint32_t microseconds)
{
    return compute_distance_using_adc_value(microseconds, read_temperature_register());
}

#endif*/



//Function declarations
/*static void handle_sensor_timer(void);
static void on_pulse_edge(void);
static uint32_t get_time(void);
static unsigned int compute_distance_using_adc_value(uint32_t microseconds, uint32_t temperature_register);
static unsigned int distance_thermometer(uint32_t microseconds);
static uint32_t read_temperature_register(void);
static int get_temperature(void);

static uint32_t volatile pulse_start_time = 0;
static uint32_t volatile pulse_end_time;
bool volatile alarm_requested;
bool volatile object_detected;
unsigned int volatile distance;
int volatile speed;
bool volatile pulse_requested;

typedef enum {
    INITIAL_START,
    POWERING_UP,
    READY,
    ACTIVE_LISTENING,
    ACTIVE_DETECTED,
    QUIESCENT
} sensor_state_t;

sensor_state_t volatile sensor_state;

void initialize_sensor(void) {
    digitalWrite(TRIGGER, 0);
    sensor_state = INITIAL_START;
    register_pin_ISR(1 << ECHO, on_pulse_edge);
    register_timer_ISR(SENSOR_TIMER, 32768, handle_sensor_timer);
    adc_t volatile *thermometer = (adc_t *)(0x4004c000);
    thermometer->control = (1 << 20) | (4 << 12) | 3;
}

void manage_sensor(void) {
    static char buffer[17] = {0};
    if (pulse_requested) {
        digitalWrite(TRIGGER, 1);
        pulse_requested = false;
        uint32_t then = get_time();
        while (get_time() - then < 10) {}
        digitalWrite(TRIGGER, 0);
    }

    if (sensor_state == QUIESCENT && object_detected) {
        alarm_requested = true;
        uint32_t pulse_time = pulse_end_time - pulse_start_time;
        unsigned int no_bonus_distance = compute_distance_using_adc_value(pulse_time, 889);
        if (no_bonus_distance > 999) no_bonus_distance = 999;
        sprintf(buffer, "Distance: %3ucm", no_bonus_distance);
        display_string(0, buffer);

        unsigned int bonus_distance = distance_thermometer(pulse_time);
        if (bonus_distance > 999) bonus_distance = 999;
        sprintf(buffer, "Temp-based dist: %3ucm", bonus_distance);
        display_string(1, buffer);

        sprintf(buffer, "Temp: %dC", get_temperature());
        display_string(2, buffer);

        distance = no_bonus_distance;
    } else if (sensor_state == READY) {
        pulse_requested = true; 
    }
}



static uint32_t get_time(void) {
    cowpi_timer_t volatile *timer = (cowpi_timer_t *)(0x40054000);
    return timer->raw_lower_word;
}

static unsigned int compute_distance_using_adc_value(uint32_t microseconds, uint32_t temperature_register) {
    return (unsigned int)((uint64_t)microseconds * ((256108888LL - 121907LL * temperature_register)) >> 33);
}

static unsigned int distance_thermometer(uint32_t microseconds) {
    return compute_distance_using_adc_value(microseconds, read_temperature_register());
}

static uint32_t read_temperature_register(void) {
    adc_t volatile *thermometer = (adc_t *)(0x4004c000);
    while ((thermometer->control & (1 << 8)) == 0);
    return thermometer->result;
}

static int get_temperature(void) {
    uint32_t adc_register_value = read_temperature_register();
    return (int)round(27 - ((3.3 * adc_register_value / 4096 - 0.706) / 0.001721));
}

void handle_sensor_timer(void) {
    switch(sensor_state) {
    case INITIAL_START:
    case POWERING_UP:
        sensor_state = READY;
        break;
    case ACTIVE_LISTENING:
        object_detected = false;
        sensor_state = QUIESCENT;
        break;
    case ACTIVE_DETECTED:
        object_detected = true;
        sensor_state = QUIESCENT;
        break;
    case QUIESCENT:
        sensor_state = READY;
        break;
    default:
        break;
    }
}


void on_pulse_edge(void) {
    static bool rising_edge = false;
    rising_edge = digitalRead(ECHO);
    if (rising_edge) {
        pulse_start_time = get_time();
        sensor_state = ACTIVE_LISTENING;
    } else {
        pulse_end_time = get_time();
        sensor_state = ACTIVE_DETECTED;
    }
}*/

static void handle_sensor_timer(void);
static void on_pulse_edge(void);
static uint32_t get_time(void);
static unsigned int compute_distance_using_adc_value(uint32_t microseconds, uint32_t temperature_register);
static unsigned int distance_thermometer(uint32_t microseconds);
static uint32_t read_temperature_register(void);
static int get_temperature(void);

static uint32_t volatile pulse_start_time = 0;
static uint32_t volatile pulse_end_time;
bool volatile alarm_requested;
bool volatile object_detected;
unsigned int volatile distance;
int volatile speed;
bool volatile pulse_requested;
static int calculate_speed(uint32_t current_time, uint32_t last_time);


typedef enum {
    INITIAL_START,
    POWERING_UP,
    READY,
    ACTIVE_LISTENING,
    ACTIVE_DETECTED,
    QUIESCENT
} sensor_state_t;

sensor_state_t volatile sensor_state;

void initialize_sensor(void) {
    digitalWrite(TRIGGER, 0);
    sensor_state = INITIAL_START;
    register_pin_ISR(1 << ECHO, on_pulse_edge);
    register_timer_ISR(SENSOR_TIMER, 32768, handle_sensor_timer);
    adc_t volatile *thermometer = (adc_t *)(0x4004c000);
    thermometer->control = (1 << 20) | (4 << 12) | 3;
}

/*void manage_sensor(void) {
    static char buffer[17];  // 增加缓冲区长度以避免溢出
    static uint32_t last_time = 0;  // 用于计算速度的时间记忆
    if (pulse_requested) {
        digitalWrite(TRIGGER, 1);
        pulse_requested = false;
        uint32_t then = get_time();
        while (get_time() - then < 10) {}
        digitalWrite(TRIGGER, 0);
    }

    if (sensor_state == QUIESCENT) {
        if (object_detected) {
            uint32_t pulse_time = pulse_end_time - pulse_start_time;
            distance = compute_distance_using_adc_value(pulse_time, 889);  // 假设温度常量，如果需要可以动态调整
            if (distance > 999) {
                sprintf(buffer, "--no detection--");
                display_string(0, buffer);
                buffer[0] = '\0';  // 清空缓冲区
                display_string(1, buffer);
            } else {
                sprintf(buffer, "Distance: %3u cm", distance);
                display_string(0, buffer);
                speed = calculate_speed(get_time(), last_time);
                sprintf(buffer, "Speed: %4d cm/s", speed);
                display_string(1, buffer);
                last_time = get_time();  // 更新上次时间
            }
        } else {
            sprintf(buffer, "--no detection--");
            display_string(0, buffer);
            buffer[0] = '\0';
            display_string(1, buffer);
        }
    } else if (sensor_state == READY) {
        pulse_requested = true;
    }
}*/
void manage_sensor(void) {
    static char buffer[17];
    static uint32_t distances[5] = {0};  // 用于存储连续的5个测量值
    static int index = 0;
    static int readings_count = 0;

    if (pulse_requested) {
        digitalWrite(TRIGGER, 1);
        pulse_requested = false;
        uint32_t then = get_time();
        while (get_time() - then < 10) {}
        digitalWrite(TRIGGER, 0);
    }

    if (sensor_state == QUIESCENT) {
        if (object_detected) {
            uint32_t pulse_time = pulse_end_time - pulse_start_time;
            uint32_t current_distance = compute_distance_using_adc_value(pulse_time, 889);

            distances[index] = current_distance;
            index = (index + 1) % 5;
            if (readings_count < 5) readings_count++;

            // 计算平均距离
            uint32_t distance_sum = 0;
            for (int i = 0; i < readings_count; i++) {
                distance_sum += distances[i];
            }
            uint32_t average_distance = distance_sum / readings_count;

            // 显示逻辑
            if (average_distance > 999) {
                sprintf(buffer, "--no detection--");
                display_string(0, buffer);
                buffer[0] = '\0';
                display_string(1, buffer);
            } else {
                sprintf(buffer, "Distance: %3u cm", average_distance);
                display_string(0, buffer);
                // 计算速度，只有在获得了足够的读数之后
                if (readings_count == 5) {
                    uint32_t last_time = get_time();  // 获取当前时间
                    int speed = calculate_speed(last_time, pulse_start_time);  // 计算速度
                    sprintf(buffer, "Speed: %5d cm/s", speed);
                    display_string(1, buffer);
                }
            }
        } else {
            sprintf(buffer, "--no detection--");
            display_string(0, buffer);
            buffer[0] = '\0';
            display_string(1, buffer);
        }
    } else if (sensor_state == READY) {
        pulse_requested = true;
    }
}



// 如果先前未定义 calculate_speed，这里提供一个示例定义
static int calculate_speed(uint32_t current_time, uint32_t last_time) {
    if (last_time == 0) return 0;  // 避免除以零
    int time_diff = current_time - last_time;
    int speed = (distance * 1000000) / time_diff;  // 单位转换为 cm/s
    return speed;
}

static uint32_t get_time(void) {
    cowpi_timer_t volatile *timer = (cowpi_timer_t *)(0x40054000);
    return timer->raw_lower_word;
}

static unsigned int compute_distance_using_adc_value(uint32_t microseconds, uint32_t temperature_register) {
    return (unsigned int)((uint64_t)microseconds * ((256108888LL - 121907LL * temperature_register)) >> 33);
}

static unsigned int distance_thermometer(uint32_t microseconds) {
    return compute_distance_using_adc_value(microseconds, read_temperature_register());
}

static uint32_t read_temperature_register(void) {
    adc_t volatile *thermometer = (adc_t *)(0x4004c000);
    while ((thermometer->control & (1 << 8)) == 0);
    return thermometer->result;
}

static int get_temperature(void) {
    uint32_t adc_register_value = read_temperature_register();
    return (int)round(27 - ((3.3 * adc_register_value / 4096 - 0.706) / 0.001721));
}

void handle_sensor_timer(void) {
    switch(sensor_state) {
    case INITIAL_START:
    case POWERING_UP:
        sensor_state = READY;
        break;
    case ACTIVE_LISTENING:
        object_detected = false;
        sensor_state = QUIESCENT;
        break;
    case ACTIVE_DETECTED:
        object_detected = true;
        sensor_state = QUIESCENT;
        break;
    case QUIESCENT:
        sensor_state = READY;
        break;
    default:
        break;
    }
}


void on_pulse_edge(void) {
    static bool rising_edge = false;
    rising_edge = digitalRead(ECHO);
    if (rising_edge) {
        pulse_start_time = get_time();
        sensor_state = ACTIVE_LISTENING;
    } else {
        pulse_end_time = get_time();
        sensor_state = ACTIVE_DETECTED;
    }
}
