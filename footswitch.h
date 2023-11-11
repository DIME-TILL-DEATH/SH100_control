#ifndef FOOTSWITCH_H_
#define FOOTSWITCH_H_

#include <stdint.h>
#include <stdbool.h>

#include "sh100_hardware.h"

typedef enum
{
	FSW_RING=0,
	FSW_ZIGZAG,
	FSW_RELAY
}FSW_SwitchMode_t;

void FSW_Init();
void FSW_MainTask(const SH100HW_Controls_t* pressedBtns);

void FSW_SetMode(FSW_SwitchMode_t newSwitchMode);

bool FSW_BlockFrontControls();

#endif /* FOOTSWITCH_H_ */