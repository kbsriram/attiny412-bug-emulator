#pragma once

#include <stdbool.h>

// A reasonably accurate, if idealized physics simulation for a
// Vibroplex-style bug which models the position of its dit paddle
// contact.
//
// An explanation of the mechanics is here:
// https://kbsriram.github.io/vibroplex-bug-analysis

void bug_init(void);
bool bug_tick(void);
