#ifndef USBD_CORE_H
#define USBD_CORE_H

#include <stdint.h>
#include "usbd_hw.h"
#include "usbd_request.h"

typedef enum
{
    USBD_ATTACHED_STATE = 0,
    USBD_POWERED_STATE,
    USBD_DEFAULT_STATE,
    USBD_ADDRESSED_STATE,
    USBD_CONFIGURED_STATE,
    USBD_SUSPENDED_STATE
}usbd_device_state;

#ifndef ADDR0_TX
    #define BTABLE_ADDR0_TX 64
	#define PMA_ADDR0_TX ((uint16_t*) PMA_BASE + BTABLE_ADDR0_TX)
#endif
#ifndef ADDR0_RX
    #define BTABLE_ADDR0_RX 128
	#define PMA_ADDR0_RX ((uint16_t*) PMA_BASE + BTABLE_ADDR0_RX)
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
}usbd_core_config;

void usbd_register_ep(uint8_t ep, uint32_t type, uint16_t tx_addr, uint16_t rx_addr, uint16_t rx_count, void (*ep_in)(void), void (*ep_out)(void));
#if 0
__STATIC_INLINE void usbd_register_ep_tx_only(usbd_ep_driver* me, uint8_t ep, uint32_t type, uint32_t tx_addr, void (*ep_in)(void))
{
	ASSERT(me != NULL);
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_CONTROL) || (type == USB_EP_TYPE_INTERRUPT));
	ASSERT(tx_addr < PMA_SIZE);
	
	me->tx_addr[ep] = ((uint16_t*) PMA_BASE + USB->BTABLE + tx_addr);
	me->ep_handler[ep][1] = ep_in;
	USBD_EP_SET_CONF(ep, type, tx_addr, 0, 0);
}

__STATIC_INLINE void usbd_register_ep_rx_only(usbd_ep_driver* me, uint8_t ep, uint32_t type, uint32_t rx_addr, uint32_t rx_count, void (*ep_out)(void))
{
	ASSERT(me != NULL);
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_CONTROL) || (type == USB_EP_TYPE_INTERRUPT));
	ASSERT((rx_addr < PMA_SIZE) && (rx_count < USBD_PMA_COUNT));

	me->rx_addr[ep] = ((uint16_t*) PMA_BASE + USB->BTABLE + rx_addr);
	me->ep_handler[ep][0] = ep_out;
	USBD_EP_SET_CONF(ep, type, 0, rx_addr, rx_count);
}

__STATIC_INLINE void usbd_register_ep_dbl_tx(usbd_ep_driver* me, uint8_t ep, uint32_t type, uint32_t tx0_addr, uint32_t tx1_addr, void (*ep_in)(void))
{
	ASSERT(me != NULL);
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_ISOCHRONOUS));
	ASSERT((tx0_addr < PMA_SIZE) && (tx1_addr < PMA_SIZE));

	me->tx0_addr[ep] = ((uint16_t*) PMA_BASE + USB->BTABLE + tx0_addr);
	me->tx1_addr[ep] = ((uint16_t*) PMA_BASE + USB->BTABLE + tx1_addr);
	me->ep_handler[ep][1] = ep_in;
	USBD_EP_SET_DBL_TX_CONF(ep, type, tx0_addr, tx1_addr);
}

__STATIC_INLINE void usbd_register_ep_dbl_rx(usbd_ep_driver* me, uint8_t ep, uint32_t type, uint32_t rx0_addr, uint32_t rx1_addr, uint32_t rx_count, void (*ep_out)(void))
{
	ASSERT(me != NULL);
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_ISOCHRONOUS));
	ASSERT((rx0_addr < PMA_SIZE) && (rx1_addr < PMA_SIZE) && (rx_count < USBD_PMA_COUNT));
		
	me->tx0_addr[ep] = ((uint16_t*) PMA_BASE + USB->BTABLE + rx0_addr);
	me->tx1_addr[ep] = ((uint16_t*) PMA_BASE + USB->BTABLE + rx1_addr);
	me->ep_handler[ep][0] = ep_out;
	USBD_EP_SET_DBL_RX_CONF(ep, type, rx0_addr, rx1_addr, rx_count);	
}

__STATIC_INLINE void usbd_unregister_ep(usbd_ep_driver* me, uint8_t ep)
{
	ASSERT(me != NULL);
	ASSERT(ep < 8);
	me->tx_addr[ep] = NULL;
	me->tx0_addr[ep] = NULL;
	me->tx1_addr[ep] = NULL;
	me->rx_addr[ep] = NULL;
	me->rx0_addr[ep] = NULL;
	me->rx1_addr[ep] = NULL;
	me->ep_handler[ep][1] = NULL;
	me->ep_handler[ep][0] = NULL;
	USBD_EP_CLEAR_CONF(ep);
}
#endif


void usbd_pma_write(uint8_t* src, __IO uint16_t* dst, uint16_t cnt);
void usbd_pma_read(__IO uint16_t* src, uint8_t* dst, uint16_t cnt);
void usbd_prepare_data_in_stage(uint8_t* buf, uint32_t cnt);
void usbd_prepare_data_out_stage(uint8_t* buf, uint32_t cnt, void (*rx_cplt)(void));
void usbd_prepare_status_in_stage(void);



//void usbd_device_event_handler(usbd_device_driver *me, USBD_EVENT e);


#endif /*USBD_CORE_H*/