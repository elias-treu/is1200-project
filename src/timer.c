#include "timer.h"

#define TIMER_STATUS (*(volatile unsigned int *)0x04000020)
#define TIMER_CONTROL (*(volatile unsigned int *)0x04000024)
#define TIMER_PERIOD_LOW (*(volatile unsigned int *)0x04000028)
#define TIMER_PERIOD_HIGH (*(volatile unsigned int *)0x0400002c)

#define TIMER_TICKS_PER_MS 30000u
#define MAX_INTERVAL_MS 100000

static void delay_interval(unsigned int ms) {
  unsigned int ticks = ms * TIMER_TICKS_PER_MS - 1u;

  // Clear a timeout left by an earlier timer operation.
  TIMER_STATUS = 0;

  // Load period into timer.
  TIMER_PERIOD_LOW = ticks & 0xffffu;
  TIMER_PERIOD_HIGH = ticks >> 16;

  // Set control bit 2 to 1 to start timer.
  TIMER_CONTROL = 0x4;

  // Wait for timeout flag.
  while ((TIMER_STATUS & 0x1u) == 0) {
  }

  // Acknowledge the timeout.s
  TIMER_STATUS = 0;
}

void delay(int ms) {
  if (ms <= 0) {
    return;
  }

  /*
   * Split long delays into smaller intervals so that
   * ms * 30,000 does not overflow a 32-bit unsigned integer.
   */
  while (ms > MAX_INTERVAL_MS) {
    delay_interval(MAX_INTERVAL_MS);
    ms -= MAX_INTERVAL_MS;
  }

  delay_interval((unsigned int)ms);
}
