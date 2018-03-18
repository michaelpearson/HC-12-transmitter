#ifndef __SPI_H
#define __SPI_H

#include <stm8s.h>
#include <util.h>

void SPI_init();
u8 SPI_read();
void SPI_write(uint8_t data);
void SPI_write_buffer(const u8 * buffer, u8 len);

#endif