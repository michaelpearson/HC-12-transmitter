#ifndef __RADIO_H
#define __RADIO_H

#include <stm8s.h>
#include <util.h>
#include <spi.h>
#include <radio_config_Si4463.h>
#include <stdio.h>

#define READ_CMD_BUFF 0x44
#define WRITE_TX_FIFO 0x66
#define START_TX 0x31
#define START_RX 0x32
#define FIFO_INFO 0x15
#define READ_RX_FIFO 0x77
#define GET_INT_STATUS 0x20

#define FIELD_LENGTH 10

INTERRUPT_HANDLER(radio_interrupt_handler, 5);

void Radio_init();

int Write_command_read_response(u8 command, const u8 * parameters, u8 parameter_count, u8 * response, u8 response_length);

void transmit_data(u8 data[FIELD_LENGTH]);

void print_interrupt_status();

void start_tx();

void start_rx();

void read_rx_fifo(u8 * buffer, u8 length);

u8 is_data_ready();

#endif