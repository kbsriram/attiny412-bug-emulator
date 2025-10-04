#include "bug.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hal.h"

extern int16_t spring_k;
extern int16_t dit_position;
extern int16_t dit_velocity;
extern int16_t dit_center;
extern bool fake_dit_pressed;

void test_bug_oscillation(int16_t test_spring_k) {
  printf("Test: bug_oscillation at k=%d\n", test_spring_k);
  hal_init();
  bug_init();

  // initial conditions:
  // x = 100 (in Q8.8)
  // v = 0
  // dit fully pressed position (dit_center = 0)
  spring_k = test_spring_k;
  dit_position = 100 << 8;
  dit_velocity = 0;
  dit_center = 0;
  fake_dit_pressed = true;

  // Verify we're in the ballpark of the analytical solution
  // f = (1 / 2 * pi) * sqrt(spring_k / m),
  // period = 1 / f
  double f = (1.0 / (2 * M_PI)) * sqrt(spring_k);
  double expected_period = 1 / f;

  // We measure how long it takes to the next peak, which should
  // be one period.
  double actual_period = 0;
  int16_t prev_2 = 100 << 8;
  int16_t prev_1 = 100 << 8;
  for (int i = 0; i < 1000; i++) {
    bug_tick();
    if ((prev_1 > prev_2) && (prev_1 >= dit_position)) {
      break;
    }
    // Clock ticks at this rate.
    actual_period += 1.0 / 1024.0;
    prev_2 = prev_1;
    prev_1 = dit_position;
  }
  printf("Expected wpm=%.2f, actual wpm=%.2f\n", 2.4 / expected_period,
         2.4 / actual_period);

  // We're ok being about 5% off target.
  double error_fraction = (expected_period - actual_period) / expected_period;
  assert((error_fraction >= -0.05) && (error_fraction <= 0.05));
}

int main(void) {
  test_bug_oscillation(171);
  test_bug_oscillation(685);
  test_bug_oscillation(2740);
  test_bug_oscillation(10966);
  test_bug_oscillation(24674);

  return 0;
}
