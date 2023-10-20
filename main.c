#include <asf.h>

#include "uart.h"
#include "midi_parser.h"

int main(void)
{
	UART_init();
	
	
	cpu_irq_enable();
	
    while(1)
    {
		MIDI_ParserTask();
	}
}