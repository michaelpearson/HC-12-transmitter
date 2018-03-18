#include <stm8s.h>
#include <spi.h>
#include <clock.h>
#include <radio.h>
#include <stdio.h>
#include <uart.h>

void _print_interrupt_status() {
  printf("Interrupt status:\r\n");
  print_interrupt_status();
}

void print_fifo_info() {
  u8 parameters[] = { 0x00 };
  u8 response[3];
  Write_command_read_response(FIFO_INFO, parameters, 1, response, 3);
 
  u8 rx_fifo[64];
  read_rx_fifo(rx_fifo, response[1]);
  
  rx_fifo[63] = '\0';
  printf("%s\r\n", rx_fifo);
  //printf("Content\r\n");
  //for (u8 a = 0;a < response[1]; a++) {
  //  printf("%d - %x (%c)\r\n", a, rx_fifo[a], rx_fifo[a]);
  //}
}

int main() {
  Clock_init();
  init_uart();
  SPI_init();
  Radio_init();
  
  start_rx();
  while (1) {
    if (is_data_ready()) {
      print_fifo_info();
    }
  }
}
