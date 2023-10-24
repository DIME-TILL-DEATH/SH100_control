#ifndef SH100_MEMORY_H_
#define SH100_MEMORY_H_

#include "sh100_controller.h"

#define MEMORY_MAGIC_WORD 0xACAB

#define MEMORY_AMP_STATE_OFFSET 8
#define MEMORY_COMMAND_BLOCK_TYPE_OFFSET MEMORY_AMP_STATE_OFFSET + sizeof(SH100_State_t)
#define MEMORY_USER_COMMANDS_OFFSET MEMORY_COMMAND_BLOCK_TYPE_OFFSET+1



#endif /* SH100_MEMORY_H_ */