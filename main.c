#include <asf.h>

#include "uart.h"
#include "midi_parser.h"
#include "midi_controller.h"
#include "sh100_hardware.h"
#include "sh100_controller.h"

int main(void)
{
	UART_init();
	SH100HW_Init();
	SH100CTRL_Init();
	MIDICTRL_Init();
	
	cpu_irq_enable();
	
    while(1)
    {
		MIDI_ParserTask();
	}
}