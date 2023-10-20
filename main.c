#include <asf.h>

#include "midi_parser.h"

int main(void)
{
	uart_init();
	
	
	cpu_irq_enable();
	
    while(1)
    {
		MIDI_ParserTask();
	}
}