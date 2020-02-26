#include "pomodoro.h"
#define WDTASKS_LIGHT_SLEEP


T_BUTTON readButton(void) {
  T_BUTTON l_value ;

  if (g_var.flags & POMO_BUTTON_SHORT) l_value = BUTTON_SHORT ;
    else if (g_var.flags & POMO_BUTTON_LONG) l_value = BUTTON_LONG ;
      else l_value = BUTTON_NONE ;
  g_var.flags &= ~(POMO_BUTTON_SHORT | POMO_BUTTON_LONG) ;
  return l_value ;
}


void setLeds(uint8_t p_on) {
  if (p_on) {
    switch(g_var.state & POMO_STEPS) {
      case POMO_READY:
        PORTB |= (_BV(PIN_GREEN) | _BV(PIN_RED)) ;
        break ;
      case POMO_WORK:
        PORTB |= _BV(PIN_RED) ;
        break ;
      case POMO_PAUSE:
        PORTB |= _BV(PIN_GREEN) ;
        break ;
    }
  } else
    PORTB &= ~(_BV(PIN_GREEN) | _BV(PIN_RED)) ;
}


void cbTkBlink(void) {
  static uint8_t l_state = 0 ;
  static uint8_t l_count = 0 ;

  if (g_var.state != l_state) {
    setLeds(0) ;
    l_count = 0 ;
    l_state = g_var.state ;
  }

  if (isReady()) {
    uint8_t l_ticks = ((g_var.state & POMO_LEVELS) + 1) * 2 ;
    if (l_count < l_ticks) setLeds((l_count & 1) == 0) ;
      else if (l_count < READY_OFF) setLeds(0) ;
        else { l_count = 0 ; return ; }
  } else if (isEnded()) {
    if (l_count < ENDED_ON) setLeds(1) ;
      else if (l_count < ENDED_OFF) setLeds(0) ;
        else { l_count = 0 ; return ; }
  } else {
    if (l_count < RUNNING_ON) setLeds(1) ;
      else if (l_count < RUNNING_OFF) setLeds(0) ;
        else { l_count = 0 ; return ; }
  }
  l_count++ ;
}


void cbTkButton(void) {
  static uint8_t l_count = 0 ;

  if (PINB & _BV(PIN_BUTTON)) { // Button released
    g_var.flags &= ~(POMO_BUTTON_SHORT | POMO_BUTTON_LONG) ;
    if (l_count >= (SECOND * 2))
      g_var.flags |= POMO_BUTTON_LONG ;
    else if (l_count)
      g_var.flags |= POMO_BUTTON_SHORT ;
    l_count = 0 ;
  } else
    l_count++ ;
}


void cbTkPomodoro(void) {
  static uint8_t l_previous = 0 ;
  static uint16_t l_elapsed = 0 ;

  T_BUTTON l_button = readButton() ;

  if (l_button == BUTTON_LONG) {
    if (isReady()) {} // ETEINDRE
      else g_var.state |= POMO_ENDED ;
  }

  uint16_t l_limit = 0 ;
  switch(g_var.state & POMO_STEPS) {
    case POMO_WORK :
      l_limit = MN_WORK ;
      break ;
    case POMO_PAUSE :
      if (g_var.state & POMO_PAUSE4) l_limit = MN_BREAK ;
        else l_limit = MN_PAUSE ;
      break ;
  }
  if (l_elapsed >= l_limit) g_var.state |= POMO_ENDED ;

  if ((l_button == BUTTON_SHORT) && isEnded()) {
    uint8_t l_level = (g_var.state & POMO_LEVELS) ;
    switch (g_var.state & POMO_STEPS) {
      case POMO_READY :
        g_var.state = l_level | POMO_WORK ;
        break ;
      case POMO_WORK :
        g_var.state = l_level | POMO_PAUSE ;
        break ;
      case POMO_PAUSE :
        if (++l_level >= 4) l_level = 0 ;
        g_var.state = POMO_READY | l_level ;
        break ;
      default :
        g_var.state = POMO_READY ;
    }
  }

  if (g_var.state != l_previous) {
    l_elapsed = 0 ;
    l_previous = g_var.state ;
  } else
    l_elapsed++ ;
}


void Init_Pins(void) {
  DDRB = _BV(PIN_GREEN) | _BV(PIN_RED) ;
  PORTB = ~(_BV(PIN_GREEN) | _BV(PIN_RED) | _BV(PIN_BUTTON)) ;
}


void Init_Tasks(void) {
  WdSched_Init(g_tasks,TASK_COUNT,TICK_UNIT) ;

  WdTask_Init(TASK_BLINK,MS_250,&cbTkBlink) ;
  WdTask_Init(TASK_BUTTON,MS_250,&cbTkButton) ;
  WdTask_Init(TASK_POMODORO,SECOND,&cbTkPomodoro) ;
}


int main(void) {
  Init_Pins() ;
  Init_Tasks() ;
  for (;;) {
    WdSched_Run() ;
  }
  return 1 ;
}
