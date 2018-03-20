#include <clock.h>

void Clock_init() {
  __disable_interrupt();
  
  CLK->CKDIVR = 0; //Ensure the clocks are running at full speed

  __enable_interrupt();
}