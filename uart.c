#include "uart.h"

void UART_init()
{
	// Rx INT: On
	// Tx INT: On
	// Receiver: On
	// Transmitter: On
	
	UCSR0B = (1<<RXCIE0) | (1<<TXCIE0) | (1<<RXEN0) | (1<<TXEN0); //UCSR1B=0x98;
	
	// 8 Data, 1 Stop, No Parity
	UCSR0C= (1<<UCSZ01) | (1<<UCSZ00); 
	
	// USART0 Mode: Asynchronous
	// USART Baud Rate: Fosc/320 = Fosc/16(UBRR+1)
	UBRR0H=0;
	UBRR0L=19;
}


// USART services===========================================================
#define FRAMING_ERROR (1<<FE0)
#define PARITY_ERROR (1<<UPE0)
#define DATA_OVERRUN (1<<DOR0)
#define DATA_REGISTER_EMPTY (1<<UDRE0)
#define RX_COMPLETE (1<<RXC0)

// USART Receiver buffer
#define RX_BUFFER_SIZE 16
uint8_t rx_buffer0[RX_BUFFER_SIZE];

uint8_t rx_wr_index, rx_rd_index, rx_counter;

ISR(USART_RX_vect)
{
	uint8_t status, data;
	status = UCSR0A;
	data = UDR0;
	
	if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
	{
		rx_buffer0[rx_wr_index++] = data;
		
		if (rx_wr_index == RX_BUFFER_SIZE) rx_wr_index=0;
		
		if (++rx_counter == RX_BUFFER_SIZE)
		{
			rx_counter=0;
		}
	}
}

uint8_t UART_PopWord()
{
	uint8_t data;
	
	while (rx_counter == 0); // blocking
	
	data = rx_buffer0[rx_rd_index++];
	
	#if RX_BUFFER_SIZE != 256
		if (rx_rd_index == RX_BUFFER_SIZE) rx_rd_index = 0;
	#endif
	
	//cpu_irq_disable();
	--rx_counter;
	//cpu_irq_enable();
	return data;
}

bool UART_RxBufferNotEmpty()
{
	return (bool)rx_counter;
}

// USART Transmitter buffer
#define TX_BUFFER_SIZE 8
uint8_t tx_buffer[TX_BUFFER_SIZE];
uint8_t tx_wr_index, tx_rd_index, tx_counter;

// USART Transmitter interrupt service routine
ISR(USART_TX_vect)
{
	if (tx_counter)
	{
		--tx_counter;
		UDR0=tx_buffer[tx_rd_index++];
		
		if (tx_rd_index == TX_BUFFER_SIZE) tx_rd_index = 0;
	}
}

// Write a character to the USART Transmitter buffer
void UART_PushWord(uint8_t c)
{	
	while (tx_counter == TX_BUFFER_SIZE);
	
	//cpu_irq_disable();
	
	if (tx_counter || ((UCSR0A & DATA_REGISTER_EMPTY)==0))
	{
		tx_buffer[tx_wr_index++] = c;
		
		#if TX_BUFFER_SIZE0 != 256
			if (tx_wr_index == TX_BUFFER_SIZE) tx_wr_index=0;
		#endif
		
		++tx_counter;
	}
	else UDR0=c;
	
	//cpu_irq_enable();
}