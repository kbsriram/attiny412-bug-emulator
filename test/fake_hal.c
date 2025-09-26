#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "hal.h"

bool fake_dit_pressed = false;

void hal_init(void) { fake_dit_pressed = false; }

bool hal_dit_pressed(void) { return fake_dit_pressed; }
