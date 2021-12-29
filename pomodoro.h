#include "wdtasks.h"

#define TASK_COUNT 3

#define PIN_GREEN  PB1
#define PIN_RED    PB3
#define PIN_BUTTON PB2

#define STEP_MASK0   0b00001111
#define STEP_MASK1   0b00000111
  #define STEP_READY   0b00000000
  #define STEP_WORK    0b00000001
  #define STEP_PAUSE   0b00000010
  #define STEP_BREAK   0b00000100
  #define STEP_ENDING  0b00001000

#define BUTTON_MASK  0b00110000
  #define BUTTON_NONE  0b00000000
  #define BUTTON_SHORT 0b00010000
  #define BUTTON_LONG  0b00100000

#define TICK_UNIT WDTO_250MS
  #define MS_250 1
  #define SECOND (MS_250 * 4)
  #define MINUTE (SECOND * 60)

  #define ON_DELAY     (2 * MS_250)
  #define OFF_READY    (1 * SECOND)
  #define OFF_RUNNING  (2 * SECOND)
  #define OFF_ENDING   (2 * MS_250)

#define TIME_ENDING  (30 * SECOND)
#define TIME_WORK0   (25 * MINUTE)
#define TIME_WORK1   (TIME_WORK0 - TIME_ENDING)
#define TIME_PAUSE0  (5 * MINUTE)
#define TIME_PAUSE1  (TIME_PAUSE0 - TIME_ENDING)
#define TIME_BREAK0  (30 * MINUTE)
#define TIME_BREAK1  (TIME_BREAK0 - TIME_ENDING)
#define TIME_START   (1 * MINUTE)
#define TIME_PWDOWN  (2 * MS_250)
#define TIME_BTSHORT (1 * MS_250)
#define TIME_BTLONG  (1 * SECOND)

WDTASK g_tasks[TASK_COUNT] ;

uint8_t g_flags = 0 ;

#define clearFlag(f) g_flags &= ~(f)
#define setFlag(f)   g_flags |= (f)
#define getFlag(f)   (g_flags & (f))

#define ledOff()   PORTB |= _BV(PIN_GREEN) | _BV(PIN_RED)
#define ledRed()   PORTB &= ~(_BV(PIN_GREEN))
#define ledGreen() PORTB &= ~(_BV(PIN_RED))
