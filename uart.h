#ifndef UART_H_
#define UART_H_

//#include <iom88.h>

#include <asf.h>

#include <stdbool.h>
#include <stdint.h>


void UART_init();

uint8_t UART_PopWord();
void UART_PushWord(uint8_t c);

bool UART_RxBufferNotEmpty();


#endif /* UART_INTERFACE_H_ */