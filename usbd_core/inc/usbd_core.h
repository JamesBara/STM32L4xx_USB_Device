#ifndef USBD_CORE_H
#define USBD_CORE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "usbd_hw.h"
#include "usbd_request.h"

#ifndef ADDR0_TX
    #define ADDR0_TX_OFFSET 64
	#define ADDR0_TX ((uint16_t*) (PMA_BASE + ADDR0_TX_OFFSET))
#endif
#ifndef ADDR0_RX
    #define ADDR0_RX_OFFSET 128
	#define ADDR0_RX ((uint16_t*) (PMA_BASE + ADDR0_RX_OFFSET))
#endif
#ifndef COUNT0_RX 
	#define COUNT0_RX USBD_FS_MAXPACKETSIZE
	#define EP0_COUNT COUNT0_RX 
#endif

typedef struct
{
	bool (*is_selfpowered)(void);
	void (*set_remote_wakeup)(bool en);
	bool (*get_remote_wakeup)(void);
	bool (*is_interface_valid)(uint8_t num);
	bool (*is_endpoint_valid)(uint8_t num, uint8_t dir);
    void (*clear_stall)(uint8_t num, uint8_t dir);
    uint8_t *(*device_descriptor)(void);
	uint8_t *(*configuration_descriptor)(uint8_t index);
	uint8_t *(*string_descriptor)(uint8_t index, uint16_t lang_id);
	uint8_t *(*bos_descriptor)(void);
	void (*set_descriptor)(usbd_setup_packet_type setup);
	uint8_t (*get_configuration)(void);
	bool (*is_configuration_valid)(uint8_t num);
	void (*set_configuration)(uint8_t num);
	uint8_t (*get_interface)(uint8_t num);
	void (*set_interface)(uint8_t num, uint8_t alt);
	void (*class_request)(usbd_setup_packet_type setup);
	void (*vendor_request)(usbd_setup_packet_type setup);
	void (*suspend)(void);
	void (*wakeup)(void);
}usbd_core_config;

void usbd_register_ep(uint8_t ep, uint32_t type, uint16_t tx_addr, uint16_t rx_addr, uint16_t rx_count, void (*ep_in)(void), void (*ep_out)(void));
void usbd_register_ep_tx(uint8_t ep, uint32_t type, uint32_t tx_addr, void (*ep_in)(void));
void usbd_register_ep_rx(uint8_t ep, uint32_t type, uint32_t rx_addr, uint32_t rx_count, void (*ep_out)(void));
void usbd_register_ep_dbl_tx(uint8_t ep, uint32_t type, uint32_t tx0_addr, uint32_t tx1_addr, void (*ep_in)(void));
void usbd_register_ep_dbl_rx(uint8_t ep, uint32_t type, uint32_t rx0_addr, uint32_t rx1_addr, uint32_t rx_count, void (*ep_out)(void));
void usbd_unregister_ep(uint8_t ep);

void usbd_pma_write(uint8_t* src, __IO uint16_t* dst, uint16_t cnt);
void usbd_pma_read(__IO uint16_t* src, uint8_t* dst, uint16_t cnt);
void usbd_prepare_data_in_stage(uint8_t* buf, uint32_t cnt);
void usbd_prepare_data_out_stage(uint8_t* buf, uint32_t cnt, void (*rx_cplt)(void));
void usbd_prepare_status_in_stage(void);

void usbd_core_init(usbd_core_config *conf);
//void usbd_core_run(void);

#endif /*USBD_CORE_H*/