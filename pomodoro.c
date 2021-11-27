#include "pomodoro.h"
#define WDTASKS_LIGHT_SLEEP
#include "pt.h"

static struct pt g_ptBlink, g_ptButton, g_ptPomodoro ;


uint8_t readButton(void) {
  uint8_t l_value ;

  l_value = getFlag(BUTTON_MASK) ;
  clearFlag(BUTTON_MASK) ;
  return l_value ;
}


void setLeds(uint8_t p_on) {
  if (p_on) {
    switch (g_flags & STEP_MASK) {
      case STEP_READY:
        PORTB |= (_BV(PIN_GREEN) | _BV(PIN_RED)) ;
        break ;
      case STEP_WORK:
        PORTB |= _BV(PIN_RED) ;
        break ;
      case STEP_PAUSE:
        PORTB |= _BV(PIN_GREEN) ;
        break ;
    }
  } else
    PORTB &= ~(_BV(PIN_GREEN) | _BV(PIN_RED)) ;
}


uint8_t ptBlink(struct pt *p_pt) {
  static uint8_t l_step ;
  static uint8_t l_off ;
  static uint8_t l_clock ;

  PT_BEGIN(p_pt) ;

    setLeds(0) ;
    l_step = getFlag(STEP_MASK) ;
    if (l_step == STEP_READY)
      l_off = OFF_READY ;
    else if (l_step == STEP_ENDED)
      l_off = OFF_ENDED ;
    else
      l_off = OFF_RUNNING ;

    for(;;) {
      setLeds(1) ;
      l_clock = WdSched_Clock() ;
      while (WdSched_Clock() - l_clock < ON_DELAY) {
        if (getFlag(STEP_MASK) != l_step) PT_RESTART(p_pt) ;
        PT_YIELD(p_pt) ;
      }
      setLeds(0) ;
      l_clock = WdSched_Clock() ;
      while (WdSched_Clock() - l_clock < l_off) {
        if (getFlag(STEP_MASK) != l_step) PT_RESTART(p_pt) ;
        PT_YIELD(p_pt) ;
      }
    }

  PT_END(p_pt) ;
}


void cbTkBlink(void) {
  ptBlink(&g_ptBlink) ;
}


uint8_t ptButton(struct pt *p_pt) {
  static uint8_t l_count ;

  PT_BEGIN(p_pt) ;

    for(;;) {
      l_count = 0 ;
      while (PINB & _BV(PIN_BUTTON)) { // Wait until button pressed
        PT_YIELD(p_pt) ;
        if (l_count >= SECOND * 10) { // Forget previously pressed button after 10s
          clearFlag(BUTTON_MASK) ;
          l_count = 0 ;
        } else
          l_count++ ;
      }
      l_count = 0 ;
      while (!(PINB & _BV(PIN_BUTTON))) { // Wait until button released
        PT_YIELD(p_pt) ;
        l_count++ ;
      }
      clearFlag(BUTTON_MASK) ;
      if (l_count >= (SECOND * 2))
        setFlag(BUTTON_LONG) ;
      else if (l_count)
        setFlag(BUTTON_SHORT) ;
    }

  PT_END(p_pt) ;
}


void cbTkButton(void) {
  ptButton(&g_ptButton) ;
}


uint8_t ptPomodoro(struct pt *p_pt) {
  static uint8_t l_level ;
  static uint8_t l_button ;
  static uint8_t l_clock ;

  PT_BEGIN(p_pt) ;
    for(;;) {
      l_button = readButton() ;
      if (l_button == BUTTON_LONG) {} //  ETEINDRE
      if (l_button == BUTTON_SHORT) break ;
      PT_YIELD(p_pt) ;
    }
    for (l_level=0 ; l_level<4 ; l_level++) {
      l_clock = WdSched_Clock() ;
      while (WdSched_Clock()-l_clock < MN_WORK) {
        l_button = readButton() ;
        if (l_button == BUTTON_LONG) PT_RESTART(p_pt) ;
        if (l_button == BUTTON_SHORT) break ;
        PT_YIELD(p_pt) ;
      }
      l_clock = WdSched_Clock() ;
      while (WdSched_Clock()-l_clock < (l_level == 3) ? MN_BREAK : MN_PAUSE) {
        l_button = readButton() ;
        if (l_button == BUTTON_LONG) PT_RESTART(p_pt) ;
        if (l_button == BUTTON_SHORT) break ;
        PT_YIELD(p_pt) ;
      }
    }

  PT_END(p_pt) ;
}


void cbTkPomodoro(void) {
  ptPomodoro(&g_ptPomodoro) ;
}


void Init_Pins(void) {
  DDRB = _BV(PIN_GREEN) | _BV(PIN_RED) ;
  PORTB = ~(_BV(PIN_GREEN) | _BV(PIN_RED) | _BV(PIN_BUTTON)) ;
}


void Init_Tasks(void) {
  WdSched_Init(g_tasks,TICK_UNIT) ;
  PT_INIT(&g_ptBlink) ;
  WdTask_Enable(WdTask_Init(MS_250,0,&cbTkBlink)) ;
  WdTask_Enable(WdTask_Init(MS_250,0,&cbTkButton)) ;
  WdTask_Enable(WdTask_Init(SECOND,0,&cbTkPomodoro)) ;
}


int main(void) {
  Init_Pins() ;
  Init_Tasks() ;
  for (;;) {
    WdSched_Run() ;
  }
  return 1 ;
}
