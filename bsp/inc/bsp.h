#ifndef BSP_H
#define BSP_H

#include <stdint.h>

uint32_t get_tick(void);
void delay(uint32_t timeout);

void bsp_init(void);

#endif /*BSP_H*/