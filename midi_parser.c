#include "midi_parser.h"
#include "midi_controller.h"

#include "uart.h"

MIDI_Command_t currentCommand;
MIDI_StatusType_t currentStatusType;
bool isSecondDataWord;

bool retranslate = true;

enum 
{
	RESET=0,
	WAIT_WORD,
	DISPATCH_STATUS,
	REC_ONE_BYTE,
	REC_TWO_BYTE,
	REC_SYS_EX,
	HADLE_SYS_EX
}state = RESET;

void handleRealTimeStatus(MIDI_Status_t status)
{
	UART_PushWord(status);
};

uint8_t recievedWord;
void MIDI_ParserTask()
{
	switch(state)
	{
		case RESET:
		{
			currentCommand.status = MIDI_COMM_UNDEFINED;
			currentCommand.data1 = 0;
			currentCommand.data2 = 0;
			currentStatusType = MIDI_TYPE_UNDEFINED;
			isSecondDataWord = false;
			state = WAIT_WORD;
			break;
		}
		
		case WAIT_WORD:
		{
			if(UART_RxBufferNotEmpty())
			{
				recievedWord = UART_PopWord();
				if(MIDI_IsSysRealTime(recievedWord))
				{ 
					handleRealTimeStatus((MIDI_Status_t)recievedWord);
					return;
				}
				
				if(MIDI_IsStatusWord(recievedWord))
				{					
					state = DISPATCH_STATUS;
				}
				else
				{
					switch(currentStatusType)
					{
						case MIDI_TYPE_TWO_BYTE: state = REC_TWO_BYTE; break;
						case MIDI_TYPE_ONE_BYTE: state = REC_ONE_BYTE; break;
						case MIDI_TYPE_START_SYS_EX: state = REC_SYS_EX; break;
						case MIDI_TYPE_STOP_SYS_EX: state = HADLE_SYS_EX; break;
						default: state = WAIT_WORD;
					}
				}
				
				if(retranslate)
				{
					UART_PushWord(recievedWord);
				}
			}
			break;
		}
		
		case DISPATCH_STATUS:
		{
			currentCommand.status = (MIDI_Status_t)((recievedWord & 0xF0) >> 4);
			currentCommand.channel = recievedWord & 0x0F;
			currentCommand.data1 = 0;
			currentCommand.data2 = 0;
			currentStatusType = MIDI_GetStatusType(currentCommand.status);
			isSecondDataWord = false;
			state = WAIT_WORD;
			break;
		}				
		
		case REC_TWO_BYTE:
		{
			if(isSecondDataWord)
			{
				currentCommand.data2 = recievedWord;
				isSecondDataWord = false;			
				MIDICTRL_HandleCommand(currentCommand);
			}
			else
			{
				currentCommand.data1 = recievedWord;
				isSecondDataWord = true;
			}
			state = WAIT_WORD;
			break;
		}
		
		case REC_ONE_BYTE:
		{
			currentCommand.data1 = recievedWord;
			MIDICTRL_HandleCommand(currentCommand);
			state = WAIT_WORD;
			break;
		}
		
		case REC_SYS_EX:
		{
			state = WAIT_WORD;
			break;
		}
		
		case HADLE_SYS_EX:
		{
			currentCommand.status = MIDI_COMM_UNDEFINED;
			currentCommand.data1 = 0;
			currentCommand.data2 = 0;
			state = WAIT_WORD;
			break;	
		}
	}
}

void MIDI_SetRetranslateState(bool enabled)
{
	retranslate = enabled;
}

void MIDI_SendCommand(MIDI_Command_t command)
{
	switch(MIDI_GetStatusType(command.status))
	{
		case MIDI_TYPE_REAL_TIME:
		{
			UART_PushWord(command.status);
			break;
		}
		case MIDI_TYPE_ONE_BYTE:
		{
			UART_PushWord(command.status);
			UART_PushWord(command.data1);
			break;
		}
		case MIDI_TYPE_TWO_BYTE:
		{
			UART_PushWord(command.status);
			UART_PushWord(command.data1);
			UART_PushWord(command.data2);
			break;
		}
		case MIDI_TYPE_START_SYS_EX:
		{
			UART_PushWord(command.status);
			break;
		}
		case MIDI_TYPE_STOP_SYS_EX:
		{
			UART_PushWord(command.status);
			break;
		}
		case MIDI_TYPE_UNDEFINED: break;
	}
}