#ifndef USBD_CDC_H 
#define USBD_CDC_H

#ifndef ADDR1_TX
#define ADDR1_TX_OFFSET 192
#define ADDR1_TX ((uint16_t*) PMA_BASE + ADDR1_TX_OFFSET)
#endif
#ifndef ADDR2_TX
#define ADDR2_TX_OFFSET 200
#define ADDR2_TX ((uint16_t*) PMA_BASE + ADDR2_TX_OFFSET)
#endif
#ifndef ADDR2_RX
#define ADDR2_RX_OFFSET 264
#define ADDR2_RX ((uint16_t*) PMA_BASE + ADDR2_RX_OFFSET)
#endif
#ifndef COUNT2_RX 
#define COUNT2_RX USBD_FS_MAX_PACKET_SIZE
#define EP2_COUNT COUNT2_RX 
#endif

#define USBD_CDC_SET_LINE_CODING (0x20U)
#define USBD_CDC_GET_LINE_CODING (0x21U)
#define USBD_CDC_SET_CONTROL_LINE_STATE (0x22U)



usbd_core_config* cdc_init(void);


#endif /*USBD_CDC_H*/