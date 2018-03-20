#include <stm8s.h>
#include <spi.h>
#include <clock.h>
#include <radio.h>
#include <stdio.h>

#if !defined(TX) || defined(RX)
#error Invalid configuration
#endif

int main() {
  Clock_init();
  SPI_init();
  Radio_init();
  
  char buffer[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  u8 i = 0;
  while(1) {
    sprintf(buffer, "I: %d", i++);
    transmit_data((u8 *)buffer);
    while(!is_tx_finished());
  }
}
