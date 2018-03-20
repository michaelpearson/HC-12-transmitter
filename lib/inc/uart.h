#ifndef __UART_H
#define __UART_H

#include <stm8s.h>
#include <stdio.h>
#include <yfuns.h>

void init_uart();

void uart_write(u8 data);

void put_char(char c);

#endif