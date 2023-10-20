#include <asf.h>

#include "uart.h"
#include "midi_parser.h"
#include "midi_controller.h"
#include "sh100_controller.h"

int main(void)
{
	UART_init();
	MIDICTRL_Init();
	SH100CTRL_Init();
	
	cpu_irq_enable();
	
    while(1)
    {
		MIDI_ParserTask();
	}
}