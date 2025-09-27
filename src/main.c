#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>

#include "bug.h"
#include "hal.h"

// (1) Vdd
// (2) PA6 - DIT_KEY
// (3) PA7 - DAH_KEY
// (4) PA1 - <SPEED_SET>
// (5) PA2 - NC
// (6) PA0 - UPDI
// (7) PA3 - TONE
// (8) GND

void setup(void) {
  hal_init();
  bug_init();
}

int main(void) {
  setup();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sei();

  while (1) {
    sleep_mode();
    // Woken up by the PIT in hal.c
    bool dit_tone = bug_tick();
    bool dah_tone = hal_dah_pressed();
    hal_tone(dit_tone | dah_tone);
  }
}
