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
// Q15 fixed point arithmetic is used for all calculations, as I felt
// was a reasonable balance between the simulation resolution and the
// ALU capabilities on the tinyAVR.
//
// A Q15 number x is an int16_t type that stands for an actual
// floating point value of x / 2^15
//
// So it can represent rational values from -32768/32768 ->
// 32767/32768 - roughly -1 -> 1
//
// Q15 numbers can be multiplied in integer arithmetic.
//
// If S = 2^15, a Q15 number x represents an actual value of x / S
//
// Given a pair of Q15 numbers x and y
//
// (x / S) * (y / S)
//     = (x * y / S) / S
//     = ((x * y) >> 15) / S
//
// which is just the Q17 number (x * y) >> 15 which we
// can do just in integer arithmetic.
//
// The only caveat is that x * y calculation needs to be performed in
// the int32_t domain before right shifting back to int16_t

#define Q15_MULT(x, y) ((int16_t)((((int32_t)(x)) * (y)) >> 15))

// The center of oscillation at the neutral position, set
// to -0.5
#define CENTER_NEUTRAL (-(1 << 14))

// The center of oscillation when the paddle is at the dit stop
// location.
#define CENTER_DIT 0

// How quickly the paddle is moved transitioning between the dit
// stops. We set it to move the full distance in about a quarter phase
// at top speed. This is roughly 16 ticks. Since the total distance
// is 2^14, the distance per tick is set to 2^10
#define PADDLE_DELTA_V (1 << 11)

// The spring constant, and is also the parameter varied to change dit
// speed.
int16_t spring_k = 44;

int16_t dit_position = CENTER_NEUTRAL;
int16_t dit_center = CENTER_NEUTRAL;
int16_t dit_velocity = 0;

void bug_init(void) {
  spring_k = 44;
  dit_position = CENTER_NEUTRAL;
  dit_center = CENTER_NEUTRAL;
  dit_velocity = 0;
}

bool bug_tick(void) {
  bool dit_pressed = hal_dit_pressed();
  if (dit_pressed) {
    if (dit_center < CENTER_DIT) {
      // move towards CENTER_DIT
      dit_center += PADDLE_DELTA_V;
      if (dit_center > CENTER_DIT) {
        dit_center = CENTER_DIT;
      }
    }
  } else {
    if (dit_center > CENTER_NEUTRAL) {
      // move towards CENTER_NEUTRAL
      dit_center -= PADDLE_DELTA_V;
      if (dit_center < CENTER_NEUTRAL) {
        dit_center = CENTER_NEUTRAL;
      }
    }
  }

  //   = -spring_k * (position - center)
  int16_t f = Q15_MULT(-spring_k, (dit_position - dit_center));

  // F = m * a
  // We fix m = 1, so a = F

  // dv = a * dt
  //    = f * dt
  // we fix dt = 1, so
  // dv = f
  dit_velocity += f;

  // dx = v * dt
  // dx = v
  dit_position = (int16_t)(dit_velocity + dit_position);

  // damp beyond neutral position.
  if (dit_position < CENTER_NEUTRAL) {
    dit_position = CENTER_NEUTRAL;
  }

  return (dit_position >= CENTER_DIT);
}
