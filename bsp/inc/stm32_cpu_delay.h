#ifndef STM32_CPU_DELAY_H
#define STM32_CPU_DELAY_H

#include <stdint.h>
#include "stm32l4xx.h"

/*@todo There is a better way. Modify this later.*/
__STATIC_FORCEINLINE void cpu_busy_wait(uint32_t max)
{
  __ASM volatile (
  ".syntax unified\n"
  "0:\n\t"
    "subs %0,%0,#1\n\t"
    "bne  0b\n"
  : "+l" (max) : : "cc"
  );
}


#endif /*STM32_CPU_DELAY_H*/