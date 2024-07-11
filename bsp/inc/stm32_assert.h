#ifndef STM32_ASSERT_H
#define STM32_ASSERT_H

#include "stm32l4xx.h"

static inline void assert_func(const char* file, int line, const char* func, const char* val)
{
    UNUSED(file);
    UNUSED(line);
    UNUSED(func);
    UNUSED(val);
    #ifndef DEBUG
        NVIC_SystemReset();
    #else
        __BKPT(0);
    #endif /*DEBUG*/
}

#define ASSERT(param) ((param) ? (void)0 : assert_func(__FILE__, __LINE__, __func__, #param))


#endif /*STM32_ASSERT_H*/