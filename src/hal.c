#include "hal.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>

static bool tone_enabled = false;

void hal_init(void) {
  // Dit is pin 6, Dah is pin 7, speed set is pin 1
  // All configured as inputs with pullup enabled.
  PORTA.DIRCLR = PIN6_bm | PIN7_bm | PIN1_bm;
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN1CTRL = PORT_PULLUPEN_bm;

  // Tone is pin 3 as output. Pin 2 is NC, and set as output.
  PORTA.DIRSET = PIN3_bm | PIN2_bm;
  PORTA.OUTCLR = PIN3_bm;
  tone_enabled = false;

  // The processor is set to run at full speed.
  // The main loop puts the processor into power down sleep mode
  // once all initialization is complete, and a PIT timer enabled
  // here is the only thing that wakes it up.

  // The PIT timer is configured to wake the system via an interrupt
  // at 1.024 KHZ, or 1024 ticks per second.

  // Once woken up, the main clock is used at full speed, and all
  // processing is orchestrated by the main loop. It puts the device
  // back into power down sleep once all processing for that tick
  // is complete.

  // Disable the prescalar and run the device at full speed on the
  // main clock.
  _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);

  // Enable the internal ULP 32k oscillator. This will be used to feed
  // the RTC PIT.
  _PROTECTED_WRITE(CLKCTRL.OSC32KCTRLA, CLKCTRL_RUNSTDBY_bm);

  // Wait for all RTC registers to stabilize.
  while (RTC.STATUS > 0) {
  }
  // Configure the RTC to use the ULP oscillator at 32KHz
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;

  // Wait for all PIT registers to stabilize.
  while (RTC.PITSTATUS > 0) {
  }
  // Enable the PIT, running at 32 cycles. This will result in
  // interrupts occurring at a 1Khz rate (32k / 32) = 1024Hz
  RTC.PITCTRLA = RTC_PERIOD_CYC32_gc | RTC_PITEN_bm;

  // Enable interrupts from the PIT
  RTC.PITINTCTRL = RTC_PI_bm;
}

// The PIT handler just clears the flag, and all the work happens in
// the main loop which wakes up after this interrupt.
ISR(RTC_PIT_vect) {
  // clear the interrupt flag
  RTC.PITINTFLAGS = RTC_PI_bm;
}

bool hal_dit_pressed(void) { return !(PORTA.IN & PIN6_bm); }

bool hal_dah_pressed(void) { return !(PORTA.IN & PIN7_bm); }

void hal_tone(bool on) {
  if (on == tone_enabled) {
    return;
  }
  tone_enabled = on;
  if (on) {
    PORTA.OUTSET = PIN3_bm;
  } else {
    PORTA.OUTCLR = PIN3_bm;
  }
}
