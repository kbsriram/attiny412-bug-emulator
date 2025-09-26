#include "hal.h"

#include <avr/io.h>
#include <stdbool.h>

void hal_init(void) {
  // Key is pin 6 configured as input with pullup enabled.
  PORTA.DIRCLR = PIN6_bm;
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
}

bool hal_dit_pressed(void) { return !(PORTA.IN & PIN6_bm); }
