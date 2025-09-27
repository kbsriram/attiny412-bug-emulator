#pragma once

#include <stdbool.h>

void hal_init(void);
bool hal_dit_pressed(void);
bool hal_dah_pressed(void);
void hal_tone(bool on);
