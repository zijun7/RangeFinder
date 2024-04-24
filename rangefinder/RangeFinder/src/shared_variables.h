/**************************************************************************//**
 *
 * @file shared_variables.h
 *
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 *
 * @brief Types and variables that need to be shared among the .c files.
 *
 ******************************************************************************/

/*
 * RangeFinder GroupLab assignment and starter code (c) 2023 Christopher A. Bohn
 * RangeFinder solution (c) the above-named students
 */

#include <CowPi.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef SHARED_VARIABLES_H
#define SHARED_VARIABLES_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined (ARDUINO_AVR_NANO)
static int const TRIGGER = 2;
static int const ECHO = 3;
static int const BUZZER = 13;
#elif defined (ARDUINO_RASPBERRY_PI_PICO)
static int const TRIGGER = 17;
static int const ECHO = 16;
static int const BUZZER = 22;
#endif // WHICH BOARD IS BEING USED?



/*** DO NOT PLACE ANY CODE ABOVE THIS LINE! ***/

/* IF YOU NEED TO DECLARE ANY ENUMS OR STRUCTS THAT
   ARE USED BY MORE THAN ONE .c FILE, PLACE THEM HERE */

#if defined (ARDUINO_RASPBERRY_PI_PICO)
/**
 * @brief Structure to access analog-digital converter's registers.
 */
typedef struct {
    uint32_t control;       //<! control and status register (20:16 RROBIN, 14:12 AINSEL, 10 ERR_STICKY, 9 ERR, 8 READY, 3 START_MANY, 2 START_ONCE, 1 TS_EN, 0 EN)
    uint32_t result;        //!< 12-bit result of last conversion
    // skip over the remaining registers for now
} adc_t;
#endif //PICO



/* IF YOU NEED TO CREATE ANY GLOBAL VARIABLES
   THAT MUST BE SHARED BETWEEN .c FILES, THEN:
    - DECLARE THEM IN *ONE AND ONLY ONE* .c FILE AS A volatile VARIABLE
      example:   unsigned long volatile foo;
    - DECLARE THEM HERE AS AN extern volatile VARIABLE
      example:   extern unsigned long volatile foo;
*/




/*** DO NOT PLACE ANY CODE BELOW THIS LINE! ***/

#ifdef __cplusplus
} // extern "C"
#endif

#endif //SHARED_VARIABLES_H
