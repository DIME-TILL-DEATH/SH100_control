#ifndef FRONT_BUTTONS_H_
#define FRONT_BUTTONS_H_

#include <stdint.h>
#include <stdbool.h>

#include "sh100_hardware.h"

void FBTNS_MainTask(const SH100HW_Controls_t* pressedBtns);

#endif /* FRONT_BUTTONS_H_ */