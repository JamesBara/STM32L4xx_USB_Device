#ifndef USBD_CDC_H 
#define USBD_CDC_H

#include <stdint.h>
#include "usbd_cdc_request.h"

#ifndef ADDR1_TX
#define ADDR1_TX_OFFSET 192
#define ADDR1_TX ((uint16_t*) (PMA_BASE + ADDR1_TX_OFFSET))
#endif
#ifndef ADDR2_TX
#define ADDR2_TX_OFFSET 200
#define ADDR2_TX ((uint16_t*) (PMA_BASE + ADDR2_TX_OFFSET))
#endif
#ifndef ADDR2_RX
#define ADDR2_RX_OFFSET 264
#define ADDR2_RX ((uint16_t*) (PMA_BASE + ADDR2_RX_OFFSET))
#endif
#ifndef COUNT2_RX 
#define COUNT2_RX USBD_FS_MAX_PACKET_SIZE
#define EP2_COUNT COUNT2_RX 
#endif





usbd_core_config* usbd_cdc_init(void);
bool usbd_cdc_is_configured(void);
usbd_cdc_line_coding usbd_cdc_get_line_coding(void);
bool usbd_cdc_get_rts(void);
bool usbd_cdc_get_dtr(void);
void usbd_cdc_transmit(uint8_t* buf, uint32_t cnt);
bool usbd_is_cdc_transmit_cplt(void);

#endif /*USBD_CDC_H*/