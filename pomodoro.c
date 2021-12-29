#ifndef WDTASKS_LIGHT_SLEEP
  #define WDTASKS_LIGHT_SLEEP
#endif

#ifndef WDTASKS_CLOCK_16
  #define WDTASKS_CLOCK_16
#endif

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include "pomodoro.h"
#include "pt.h"

static struct pt g_ptBlink, g_ptButton, g_ptPomodoro ;


EMPTY_INTERRUPT(INT0_vect)  ;


uint8_t readButton(void) {
  uint8_t l_value ;

  l_value = getFlag(BUTTON_MASK) ;
  clearFlag(BUTTON_MASK) ;
  return l_value ;
}


void setLeds(uint8_t p_on) {
  static uint8_t l_red = 0 ;

  ledOff() ;
  if (p_on)
    switch (getFlag(STEP_MASK1)) {
      case STEP_READY:
        if (l_red) ledRed() ;
          else ledGreen() ;
        l_red = !l_red ;
        break ;
      case STEP_WORK:
        ledRed() ;
        break ;
      default:
        ledGreen() ;
    }
}


void powerDown(void) {
  cli() ;
    WDTCR |= _BV(WDCE) | _BV(WDE) ;
    WDTCR &= ~(_BV(WDIE) | _BV(WDE)) ; // Stop watchdog
    GIMSK |= _BV(INT0) ; // Start INT0
    set_sleep_mode(SLEEP_MODE_PWR_DOWN) ;
    ledOff() ;
  sei() ;

  sleep_mode() ;

  cli() ;
    set_sleep_mode(SLEEP_MODE_IDLE) ;
    GIMSK &= ~(_BV(INT0)) ; // Stop INT0
    WDTCR |= _BV(WDIE) | _BV(WDE) ; // Start watchdog
  sei() ;
}


PT_THREAD (ptBlink(struct pt *p_pt)) {
  static uint8_t l_step ;
  static uint8_t l_off ;
  static uint16_t l_clock ;

  PT_BEGIN(p_pt) ;

    l_step = getFlag(STEP_MASK1) ;
    l_off = (l_step == STEP_READY) ? OFF_READY : OFF_RUNNING ;

    for(;;) {
      if (getFlag(STEP_ENDING)) l_off = OFF_ENDING ;

      setLeds(1) ;
      l_clock = WdSched_Clock() ;
      while (WdSched_Clock() - l_clock < ON_DELAY) {
        if (getFlag(STEP_MASK1) != l_step) PT_RESTART(p_pt) ;
        PT_YIELD(p_pt) ;
      }
      setLeds(0) ;
      l_clock = WdSched_Clock() ;
      while (WdSched_Clock() - l_clock < l_off) {
        if (getFlag(STEP_MASK1) != l_step) PT_RESTART(p_pt) ;
        PT_YIELD(p_pt) ;
      }
    }

  PT_END(p_pt) ;
}


PT_THREAD (ptButton(struct pt *p_pt)) {
  static uint16_t l_clock ;

  PT_BEGIN(p_pt) ;

    for(;;) {
      PT_WAIT_WHILE(p_pt, PINB & _BV(PIN_BUTTON)) ; // Wait until button pressed
      l_clock = WdSched_Clock() ;
      PT_WAIT_UNTIL(p_pt, PINB & _BV(PIN_BUTTON)) ; // Wait until button released
      clearFlag(BUTTON_MASK) ;
      if (WdSched_Clock() - l_clock >= TIME_BTLONG) setFlag(BUTTON_LONG) ;
        else if (WdSched_Clock() - l_clock >= TIME_BTSHORT) setFlag(BUTTON_SHORT) ;
    }

  PT_END(p_pt) ;
}


PT_THREAD (ptPomodoro(struct pt *p_pt)) {
  static uint8_t l_level ;
  static uint8_t l_button ;
  static uint16_t l_clock ;

  PT_BEGIN(p_pt) ;

    readButton() ; // Empty button buffer

    for(;;) {

      clearFlag(STEP_MASK0) ;

      l_clock = WdSched_Clock() ;
      PT_WAIT_UNTIL (p_pt, (l_button = readButton()) || (WdSched_Clock() - l_clock >= TIME_START) ) ;
      if (l_button != BUTTON_SHORT) {
        l_clock = WdSched_Clock() ;
        PT_WAIT_UNTIL(p_pt, WdSched_Clock() - l_clock >= TIME_PWDOWN) ;
        powerDown() ;
        PT_RESTART(p_pt) ;
      }

      for (l_level=0 ; l_level<=3 ; l_level++) {

        clearFlag(STEP_MASK0) ; setFlag(STEP_WORK) ;
        l_clock = WdSched_Clock() ;
        while ( !( (l_button = readButton()) || (WdSched_Clock() - l_clock >= TIME_WORK0) ) ) {
          if ( (!getFlag(STEP_ENDING)) && (WdSched_Clock() - l_clock >= TIME_WORK1) ) setFlag(STEP_ENDING) ;
          PT_YIELD(p_pt) ;
        }
        if (l_button == BUTTON_LONG) PT_RESTART(p_pt) ;

        l_clock = WdSched_Clock() ;
        if (l_level == 3) break ;
        clearFlag(STEP_MASK0) ; setFlag(STEP_PAUSE) ;
        while ( !( (l_button = readButton()) || (WdSched_Clock() - l_clock >= TIME_PAUSE0) ) ) {
          if ( (!getFlag(STEP_ENDING)) && (WdSched_Clock() - l_clock >= TIME_PAUSE1) ) setFlag(STEP_ENDING) ;
          PT_YIELD(p_pt) ;
        }
        if (l_button == BUTTON_LONG) PT_RESTART(p_pt) ;
      }

      clearFlag(STEP_MASK0) ; setFlag(STEP_BREAK) ;
      while ( !( (l_button = readButton()) || (WdSched_Clock() - l_clock >= TIME_BREAK0) ) ) {
        if ( (!getFlag(STEP_ENDING)) && (WdSched_Clock() - l_clock >= TIME_BREAK1) ) setFlag(STEP_ENDING) ;
        PT_YIELD(p_pt) ;
      }

    }

  PT_END(p_pt) ;
}


void cbTkBlink(void) { ptBlink(&g_ptBlink) ; }

void cbTkButton(void) { ptButton(&g_ptButton) ; }

void cbTkPomodoro(void) { ptPomodoro(&g_ptPomodoro) ; }


void Init_Registers(void) {
  DDRB = _BV(PIN_GREEN) | _BV(PIN_RED) ; // Output
  PORTB = _BV(PIN_BUTTON) ; // Input pullup
  MCUCR &= ~(_BV(ISC01) | _BV(ISC00)) ; // Low level INT0
  power_all_disable() ;
}


void Init_Tasks(void) {
  WdSched_Init(g_tasks, TICK_UNIT) ;
  PT_INIT(&g_ptBlink) ;
  WdTask_Enable(WdTask_Init(MS_250, 0, &cbTkBlink)) ;
  PT_INIT(&g_ptButton) ;
  WdTask_Enable(WdTask_Init(MS_250, 0, &cbTkButton)) ;
  PT_INIT(&g_ptPomodoro) ;
  WdTask_Enable(WdTask_Init(MS_250*2, 0, &cbTkPomodoro)) ;
}


int main(void) {
  Init_Registers() ;
  Init_Tasks() ;
  for (;;) {
    WdSched_Run() ;
  }
  return 1 ;
}
