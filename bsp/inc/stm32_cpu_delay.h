#ifndef STM32_CPU_DELAY_H
#define STM32_CPU_DELAY_H

#include <stdint.h>
#include "stm32l4xx.h"


/**
 * @brief Do nothing for a specified amount of time.
 * @param max Amount of time to wait.
 * @note This function shouldn't be used under normal
 * circumstances since it locks the cpu and causes it
 * to spend cycles doing nothing.
 * @return 
 */
__STATIC_FORCEINLINE void __cpu_busy_wait(uint32_t max)
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