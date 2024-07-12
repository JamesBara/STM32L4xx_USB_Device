#ifndef BSP_H
#define BSP_H

#include <stdint.h>

uint32_t get_tick(void);
void delay(uint32_t timeout);

void set_cpu_max_freq(void);
void set_usbd_clk_src_hsi48(void);

void bsp_init(void);

#endif /*BSP_H*/