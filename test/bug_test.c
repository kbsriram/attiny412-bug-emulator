#include "bug.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hal.h"

extern int16_t spring_k;
extern int16_t dit_position;
extern int16_t dit_center;
extern bool fake_dit_pressed;

void test_bug_oscillation(void) {
  printf("Test: bug_oscillation\n");
  hal_init();
  bug_init();

  // initial conditions:
  // spring_k = 50/32768
  // x0 = 16384/32768
  // dit fully pressed position (dit_center = 0)
  spring_k = 50;
  dit_position = 16384;
  dit_center = 0;
  fake_dit_pressed = true;

  // Verify we're roughly in the ballpark of the analytical solution
  // f = (1 / 2 * pi) * sqrt(spring_k / m),
  // m = 1
  // period = 1 / f
  double f = (1.0 / (2 * M_PI)) * sqrt(spring_k / 32768.0);
  double expected_period = 1 / f;

  int quarter_period = 0;
  for (int i = 0; i < 100; i++) {
    if (dit_position < 0) {
      break;
    }
    bug_tick();
    quarter_period++;
  }

  // We're ok being about 1% off target.
  double error_fraction =
      (expected_period - 4 * quarter_period) / expected_period;
  assert((error_fraction >= -0.01) && (error_fraction <= 0.01));
}

int main(void) {
  test_bug_oscillation();

  return 0;
}
