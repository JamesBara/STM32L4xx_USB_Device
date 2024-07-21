#ifndef USBD_CORE_H
#define USBD_CORE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "usbd_hw.h"
#include "usbd_request.h"

/*******************************************************************************
 * USBD core logic.
 ******************************************************************************/

/************************************************
 * @brief Chosen addresses and for endpoint 0 in the
 * PMA.
 * 
 * @note The user can overide them and select 
 * their own.
 ***********************************************/
#ifndef ADDR0_TX
    #define ADDR0_TX_OFFSET 64
	#define ADDR0_TX ((uint16_t*) (PMA_BASE + ADDR0_TX_OFFSET))
#endif
#ifndef ADDR0_RX
    #define ADDR0_RX_OFFSET 128
	#define ADDR0_RX ((uint16_t*) (PMA_BASE + ADDR0_RX_OFFSET))
#endif
#ifndef COUNT0_RX 
	#define COUNT0_RX USBD_FS_MAX_PACKET_SIZE
	#define EP0_COUNT COUNT0_RX 
#endif

/************************************************
 * @brief This is a series of callbacks that
 * should be implemented from the user,
 * depending on the device configuration.
 * Some of them are mandatory for example,
 * the device_descriptor callback.
 * 
 * @todo Add other callbacks that might be missing.
 * 
 ***********************************************/
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
	void (*sof)(void);
}usbd_core_config_type;

/*******************************************************************************
 * Endpoint configuration functions.
 ******************************************************************************/
void usbd_register_ep(uint8_t ep, uint32_t type, uint16_t tx_addr, uint16_t rx_addr, uint16_t rx_count, void (*ep_in)(void), void (*ep_out)(void));
void usbd_register_ep_tx(uint8_t ep, uint32_t type, uint32_t tx_addr, void (*ep_in)(void));
void usbd_register_ep_rx(uint8_t ep, uint32_t type, uint32_t rx_addr, uint32_t rx_count, void (*ep_out)(void));
void usbd_register_ep_dbl_tx(uint8_t ep, uint32_t type, uint32_t tx0_addr, uint32_t tx1_addr, void (*ep_in)(void));
void usbd_register_ep_dbl_rx(uint8_t ep, uint32_t type, uint32_t rx0_addr, uint32_t rx1_addr, uint32_t rx_count, void (*ep_out)(void));
void usbd_unregister_ep(uint8_t ep);

/*******************************************************************************
 * Read/Write PMA functions.
 ******************************************************************************/
void usbd_pma_read(uint16_t* src, uint8_t* dst, uint16_t cnt);
void usbd_pma_write(uint8_t* src, uint16_t* dst, uint16_t cnt);

/*******************************************************************************
 * Endpoint 0 related functions. Used for class, or vendor request
 * handling.
 ******************************************************************************/
void usbd_prepare_data_in_stage(uint8_t* buf, uint32_t cnt);
void usbd_prepare_data_out_stage(uint8_t* buf, uint32_t cnt, void (*rx_cplt)(void));
void usbd_prepare_status_in_stage(void);

/*******************************************************************************
 * Core functions to be used by main.
 ******************************************************************************/
void usbd_core_init(usbd_core_config_type* conf);
#if USBD_CORE_EVENT_DRIVEN == 1
void usbd_core_run(void);
#endif

#endif /*USBD_CORE_H*/