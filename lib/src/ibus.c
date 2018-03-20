#include <ibus.h>

u8 number_of_channels = 0;
u8 * channel_data = NULL;

void init_ibus(u8 _number_of_channels) {
  number_of_channels = _number_of_channels;
  channel_data = malloc(number_of_channels);
  
  // Set Baud to 115200 with F_CPU = 16MHz
  UART1->BRR2 = 0x0B;
  UART1->BRR1 = 0x08;
  
  // Enable UART1
  UART1->CR2 |= UART1_CR2_TEN;
}

void uart_write(u8 data) {
  // Write the byte to the tx buffer
  UART1->DR = data;
  
  // Wait for the buffer to be ready
  while ((UART1->SR & UART1_SR_TXE) == 0);
}

_STD_BEGIN

#pragma module_name = "?__write"

size_t __write(int handle, const unsigned char * buffer, size_t size) {
  size_t nChars = 0;

  if (buffer == 0) {
    return 0;
  }

  /* This template only writes to "standard out" and "standard err",
   * for all other file handles it returns failure. */
  if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR) {
    return _LLIO_ERROR;
  }

  for (/* Empty */; size != 0; --size) {
    uart_write(*buffer++);
    ++nChars;
  }

  return nChars;
}

_STD_END