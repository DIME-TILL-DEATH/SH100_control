#ifndef UART_H_
#define UART_H_

//#include <iom88.h>

#include <asf.h>

#include <stdbool.h>
#include <stdint.h>


void uart_init();

uint8_t usart_pop_next_word();
bool isRxBufferNotEmpty();


#endif /* UART_INTERFACE_H_ */