#include <asf.h>

#include "midi_parser.h"

int main(void)
{
	cpu_irq_enable();
	
    while(1)
    {
		MIDI_ParserTask();
	}
}