/*
 * Pin# | Function          | board pin/signal
 * 1    | PD4               | Si4463 p1 - SDN - Shutdown Pin
 * 12   | PB4               | Si4463 p9  - GPIO0
 * 13   | PC3               | Si4463 p10 - GPIO1 - CTS
 * 14   | PC4               | Si4463 p11 - nIRQ - Interrupt (active low)
 *
 * 15   | PC5/SPI_SCK       | Si4463 p12 - SPI - SCLK
 * 16   | PC6/SPI_MOSI      | Si4463 p14 - SPI - SDI
 * 17   | PC7/SPI_MISO      | Si4463 p13 - SPI - SDO
 * 19   | PD2               | Si4463 p15 - SPI - nSEL
 *
 *
 * 2    | PD5/UART1_TX      | header TXD pad via level shifter
 * 3    | PD6/UART1_RX      | header RXD pad via level shifter
 *
 * 4    | nRST              | reset test point nearest header RXD pad
 * 18   | PD1/SWIM          | SWIM test point nearest header TXD pad
 *
 * 11   | PB5               | header SET pad via level shifter
 */

#include <radio.h>

static inline void Enable_radio();
static inline void Disable_radio();
static inline void Radio_begin();
static inline void Radio_end();
static inline void Setup_shutdown_pin();
static inline void Setup_chip_select_pin();
static inline void Setup_interrupt_pin();
static inline void Download_configuration();
static inline void CTS_wait();


int Write_command_read_response(u8 command, const u8 * parameters, u8 parameter_count, u8 * response, u8 response_length);

void read_and_clear_interrupts(u8 interrupt_status[9]);

u8 radio_configuration_data[] = RADIO_CONFIGURATION_DATA_ARRAY;

u8 rx_data_ready = 0;

INTERRUPT_HANDLER(radio_interrupt_handler, 5) {
  u8 buffer[9];
  read_and_clear_interrupts(buffer);
  rx_data_ready = 1;
}

static inline void Enable_radio() {
  // set the SDN pin to low to enable radio.
  GPIOD->ODR &= ~(1 << 4);
}

static inline void Disable_radio() {
  // set the SDN pin to high to shutdown radio.
  GPIOD->ODR |= (1 << 4);
}

static inline void Radio_begin() {
  // Set CS to low to begin communication with radio
  GPIOD->ODR &= ~(1 << 2);
}

static inline void Radio_end() {
  while(SPI->SR & SPI_SR_BSY);
  // Set CS to high to end communication with the radio
  GPIOD->ODR |= (1 << 2);
}

static inline void Setup_shutdown_pin() {
  // Set PD4 to push-pull fast mode
  GPIOD->DDR |= (1 << 4);
  GPIOD->CR1 |= (1 << 4);
  GPIOD->CR2 |= (1 << 4);
}

static inline void Setup_chip_select_pin() {
  // Set PD2 to output push pull fast
  GPIOD->DDR |= (1 << 2);
  GPIOD->CR1 |= (1 << 2);
  GPIOD->CR2 |= (1 << 2);
  Radio_end();
}

static inline void Setup_chip_gpio_pins() {
  // Setup GPIO1 (CTS) which is connected to PORTD PIN3
  // POERTD PIN3 - Floating without interrupt - pullup off
  GPIOC->DDR &= ~(1 << 3);
  GPIOC->CR1 &= ~(1 << 3);
  GPIOC->CR2 &= ~(1 << 3);
}

static inline void Setup_interrupt_pin() {
  // * 14   | PC4               | Si4463 p11 - nIRQ - Interrupt (active low)
  // Setup nIRQ - PC4 as floating input with interrupt.
  GPIOC->DDR &= ~(1 << 4);
  GPIOC->CR1 &= ~(1 << 4);
  GPIOC->CR2 |= (1 << 4);
  
  // Setup interrupt on falling edge only
  EXTI->CR1 = 0x20;
}

static inline void CTS_wait() {
  // Wait for GPIO 1 to go low
  while((GPIOC->IDR & (1 << 3)) == 0);
}

int Write_command_read_response(u8 command, const u8 * parameters, u8 parameter_count, u8 * response, u8 response_length) {
  // Wait for CTS
  CTS_wait();
  
  // Make sure we cannot be interrupted while communicating with the radio
  __disable_interrupt();
  
  // Begin communication & writing the command with parameters.
  Radio_begin();
  SPI_write(command);
  SPI_write_buffer(parameters, parameter_count);
  // End communication. The CTS will now go low. We need to wait for the CTS to go low in order to read the response.
  Radio_end();
  
  // If there is no response, do not attempt to read it.
  if (!response_length) {
    // Re-enable interrupts
    __enable_interrupt();
    return 0;
  }
  
  CTS_wait();
  
  // Start communication to retreive the response
  Radio_begin();
  
  // Write the command to read the response
  SPI_write(READ_CMD_BUFF);
  
  while(response_length--) {
    *response++ = SPI_read();
  }
  
  // End the communication
  Radio_end();
  
  // Re-enable interrupts
  __enable_interrupt();
  
  return 0;
}

static inline void Download_configuration() {
  const u8 * current_command = radio_configuration_data;
  while (*current_command > 0) {
    // Setup command parameters. Structure is { command_length, command, ...parameters, etc...}
    const u8 command_length = *current_command;
    const u8 command = *(current_command + 1);
    const u8 * parameters = current_command + 2;
    const u8 parameter_count = command_length - 1;
    Write_command_read_response(command, parameters, parameter_count, (u8 *)0, 0);
    
    // Move the current command pointer to the next command.
    current_command = parameters + parameter_count;
  }
}

void Radio_init() {
  __disable_interrupt();
  Setup_shutdown_pin();
  Setup_chip_select_pin();
  Setup_chip_gpio_pins();
  Setup_interrupt_pin();
  Disable_radio();
  delay(10);
  Enable_radio();
  delay(10);
  Download_configuration();
  __enable_interrupt();
}

void start_tx() {
  u8 start_tx_parameters[] = { 0, 0, 0, 0 };
  Write_command_read_response(START_TX, start_tx_parameters, 4, NULL, 0);
}

void start_rx() {
  u8 start_rx_parameters[] = { 
    0, // Channel
    0, // Start condition (immediate)
    0, // RX_LEN Lo
    0, // RX_LEN Hi
    0x08, // Next state rx timeout (Re-arm)
    0x08, // Next state rx valid (Re-arm)
    0x08  // Next state rx invalid (Re-arm)
  };
  Write_command_read_response(START_RX, start_rx_parameters, 7, NULL, 0);
}

void read_rx_fifo(u8 * buffer, u8 length) {
  Radio_begin();
  SPI_write(READ_RX_FIFO);
  while(length--) {
    *buffer++ = SPI_read();
  }
  Radio_end();
  rx_data_ready = 0;
}

void transmit_data(u8 data[FIELD_LENGTH]) {
  Write_command_read_response(WRITE_TX_FIFO, data, FIELD_LENGTH, NULL, 0);
  start_tx();
}

void read_and_clear_interrupts(u8 interrupt_status[9]) {
  u8 parameters[] = { 0, 0, 0 };
  Write_command_read_response(GET_INT_STATUS, parameters, 3, interrupt_status, 9);
}

void print_interrupt_status() {
  u8 buffer[9];
  read_and_clear_interrupts(buffer);
  for (int a = 0;a < 9; a++) {
    printf("%d - 0x%x\r\n", a, buffer[a]);
  }
}

u8 is_data_ready() {
  return rx_data_ready;
}




