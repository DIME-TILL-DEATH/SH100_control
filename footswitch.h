#ifndef FOOTSWITCH_H_
#define FOOTSWITCH_H_

#include <stdint.h>
#include <stdbool.h>

#include "sh100_hardware.h"

typedef enum
{
	FSW_DISCRET = 0,
	FSW_RELAY = 1,
	FSW_RING = 2,
	FSW_ZIGZAG = 3
}FSW_SwitchMode_t;

void FSW_Init();
void FSW_MainTask(const SH100HW_Controls_t* pressedBtns);

void FSW_SetMode(FSW_SwitchMode_t newSwitchMode);

bool FSW_BlockFrontControls();

#endif /* FOOTSWITCH_H_ */