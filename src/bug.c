#include "bug.h"

#include <stdbool.h>
#include <stdint.h>

#include "hal.h"

// The simulation here is straightforward - the dit contact is modeled
// as a point mass oscillating about a center. The center is moved
// depending on whether the dit paddle is open or closed.
//
// The simulation keeps track of the center, position and velocity of
// the dit contact. At each tick, the spring force on the contact is
// computed using the current distance of the dit contact from the
// current center. This is used to update the velocity and position.
//
// The clock ticks every 1/1024 seconds, so these values are updated
// with a delta t of 1/1024 seconds.
//
// We would also like to accomodate speeds from about 5wpm to 60 wpm.
// The dit duration for a given wpm is 1.2/wpm seconds, so the
// corresponding period for the underlying oscillation is twice that
// or 2.4/wpm.
//
// The desired frequency of the dit paddle for a given wpm then would just
// be its inverse, wpm/2.4
//
// But this is also 1/(2*pi)sqrt(k/m) where k is the spring constant
// and m is the mass. To simplify further calculations, we will assume
// m = 1
//
// re-arranging the terms, we get the desired spring constant for a
// given wpm as
// k = (2*pi*wpm)^2/2.4^2
//
// for a range of 5 to 60 wpm, k will range from about 171 to 24,674 which
// fits conveniently in an int16_t
//
// We select other parameters to use as much of the int16_t range as
// feasible, with some buffer to avoid overflow from rounding errors.
//
// The maximum velocity is amplitude * sqrt(k), and choosing an amplitude
// of 100, the maximum velocity across our desired wpm will range from
// about 1300 to 15700
//
// If we limit our position values from +/- 256, we can use Q8.8
// fixed-point values for positions (i.e., a given position value p
// will represent an actual value of p / 2^8)
//
// So at each tick, the following calculations will update the current
// position and velocity values.
//
// Force = -spring_k * displacement from center
// f = k * displacement / 2^8 = (-k * displacement) >> 8
// force = mass * acceleration, mass == 1 so
// a = f
// change in velocity dv = acceleration * change in time
// dv = a * 1 / 1024 = a >> 10
// velocity += (a >> 10)
// change in position dx = velocity * change in time.
// keeping in mind that we're scaling up position values by 2^8
// dx = v * dt * 2^8
//    = v * 1 / 1024 * 2^8
//    = v * 1 / 4 = v >> 2
// position += (v >> 2)

#define INT_TO_Q8_8(x) ((x) << 8)

// The center of oscillation at the neutral position, set
// to -100
#define CENTER_NEUTRAL INT_TO_Q8_8(-100)

// The center of oscillation when the paddle is at the dit stop
// location. This allows it to oscillate from -100 to 100
#define CENTER_DIT 0

// The spring constant, and is also the parameter varied to change dit
// speed.
// k = (2*pi*wpm)^2/2.4^2 ~= 6.85 * wpm^2

// 5wpm    60wpm
// 171     24674
int16_t spring_k = 2740;

int16_t dit_position = CENTER_NEUTRAL;
int16_t dit_center = CENTER_NEUTRAL;
int16_t dit_velocity = 0;

void bug_init(void) {
  spring_k = 2740;
  dit_position = CENTER_NEUTRAL;
  dit_center = CENTER_NEUTRAL;
  dit_velocity = 0;
}

bool bug_tick(void) {
  bool dit_pressed = hal_dit_pressed();
  if (dit_pressed) {
    if (dit_center != CENTER_DIT) {
      dit_center = CENTER_DIT;
    }
  } else {
    if (dit_center != CENTER_NEUTRAL) {
      dit_center = CENTER_NEUTRAL;
    }
  }

  int32_t displacement = (int32_t)dit_position - (int32_t)dit_center;
  int32_t f = ((int32_t)spring_k * displacement) >> 8;
  int32_t a = f;
  dit_velocity -= (int16_t)(a >> 10);
  dit_position += (dit_velocity >> 2);

  // damp beyond neutral position.
  if (dit_position < CENTER_NEUTRAL) {
    dit_position = CENTER_NEUTRAL;
    dit_velocity = 0;
  }

  return (dit_position >= CENTER_DIT);
}
