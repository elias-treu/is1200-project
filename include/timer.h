#ifndef TIMER_H
#define TIMER_H

/*
 * Block execution for approximately ms milliseconds.
 *
 * This function uses the DTEK-V interval timer and therefore takes ownership
 * of timer 1 while it is running.
 */
void delay(int ms);

#endif
