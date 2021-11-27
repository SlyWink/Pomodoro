#include "wdtasks.h"

#define TASK_COUNT 3
  #define TASK_BLINK    0
  #define TASK_BUTTON   1
  #define TASK_POMODORO 2

#define PIN_GREEN  PB1
#define PIN_RED    PB2
#define PIN_BUTTON PB3

#define STEP_READY   0b000
#define STEP_WORK    0b001
#define STEP_PAUSE   0b010
#define STEP_ENDED   0b100
#define STEP_MASK    0b111
#define BUTTON_NONE  (0b00 << 3)
#define BUTTON_SHORT (0b01 << 3)
#define BUTTON_LONG  (0b10 << 3)
#define BUTTON_MASK  (0b11 << 3)

#define TICK_UNIT WDTO_250MS
  #define MS_250 1
  #define SECOND 4
  #define MINUTE (SECOND * 60)

  #define ON_DELAY     2
  #define OFF_READY   12
  #define OFF_RUNNING  8
  #define OFF_ENDED    4

#define MN_WORK  (25 * MINUTE)
#define MN_PAUSE  (5 * MINUTE)
#define MN_BREAK (30 * MINUTE)

WDTASK g_tasks[TASK_COUNT] ;

uint8_t g_flags = 0 ;

#define clearFlag(f) g_flags &= ~(f)
#define setFlag(f)   g_flags |= (f)
#define getFlag(f)   (g_flags & (f))
