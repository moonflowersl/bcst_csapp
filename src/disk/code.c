#include<stdlib.h>
#include<stdint.h>

#include "memory/instruction.h"


#define INST_LEN 100

// inst_t program[INST_LEN] = {
    
//     // rip
//     {
//       mov,
//       { REG, 0, 0, (uint64_t *)&reg.rax, NULL},
//       { REG, 0, 0, (uint64_t *)&reg.rbx, NULL},
//       "mov rax rbx"  
//     },
// }