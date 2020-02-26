#include "wdtasks.h"

#define TASK_COUNT 3
  #define TASK_BLINK    0
  #define TASK_BUTTON   1
  #define TASK_POMODORO 2

#define PIN_GREEN  PB1
#define PIN_RED    PB2
#define PIN_BUTTON PB3

#define POMO_BUTTON_SHORT _BV(6)
#define POMO_BUTTON_LONG  _BV(7)

#define POMO_LEVELS 0b00000011
#define POMO_READY  0b00000100
#define POMO_WORK   0b00001000
#define POMO_PAUSE  0b00010000
#define POMO_ENDED  0b00100000

#define POMO_START  0
#define POMO_READY1 (1 | POMO_READY)
#define POMO_WORK1  (1 | POMO_WORK)
#define POMO_PAUSE1 (1 | POMO_PAUSE)
#define POMO_READY2 (2 | POMO_READY)
#define POMO_WORK2  (2 | POMO_WORK)
#define POMO_PAUSE2 (2 | POMO_PAUSE)
#define POMO_READY3 (3 | POMO_READY)
#define POMO_WORK3  (3 | POMO_WORK)
#define POMO_PAUSE3 (3 | POMO_PAUSE)
#define POMO_READY4 (4 | POMO_READY)
#define POMO_WORK4  (4 | POMO_WORK)
#define POMO_PAUSE4 (4 | POMO_PAUSE)

#define POMO_STEPS  (POMO_READY | POMO_WORK | POMO_PAUSE)

typedef enum {
  BUTTON_NONE, BUTTON_SHORT, BUTTON_LONG
} T_BUTTON ;


#define TICK_UNIT WDTO_250MS
  #define MS_250 1
  #define SECOND 4
  #define MINUTE (SECOND * 60)

  #define READY_OFF  10
  #define RUNNING_ON  1
  #define RUNNING_OFF 8
  #define ENDED_ON    2
  #define ENDED_OFF   4

#define MN_WORK  (25 * MINUTE)
#define MN_PAUSE  (5 * MINUTE)
#define MN_BREAK (30 * MINUTE)

WDTASK g_tasks[TASK_COUNT] ;

struct {
  uint8_t flags ;
  uint8_t state ;
} g_var = { 0, POMO_READY1 } ;


#define isPause() (g_var.state & POMO_PAUSE)
#define isWork() (g_var.state & POMO_WORK)
#define isReady() (g_var.state & POMO_READY)
#define isEnded() (g_var.state & POMO_ENDED)
