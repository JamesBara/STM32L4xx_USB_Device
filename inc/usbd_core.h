#ifndef USBD_CORE_H
#define USBD_CORE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "usbd_hw.h"
#include "usbd_desc.h"

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
    #define ADDR0_TX 64
#endif
#ifndef ADDR0_RX
    #define ADDR0_RX 128
#endif
#ifndef COUNT0_RX 
	#define EP0_COUNT USBD_FS_MAX_PACKET_SIZE
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
struct usbd_core_driver
{
	bool (*is_selfpowered)(void); /*!< Notifies the usbd_core if the device is selfpowered.*/
	void (*set_remote_wakeup)(bool en); /*!< Callback that sets or clears remote wakeup.*/
	bool (*get_remote_wakeup)(void); /*!< Notifies the usbd_core if the device remote wakeup is enabled.*/
	bool (*is_interface_valid)(uint8_t num); /*!< Notifies the usbd_core if the selected interface is valid.*/
	bool (*is_endpoint_valid)(uint8_t num, uint8_t dir); /*!< Notifies the usbd_core if the selected endpoint is valid.*/
    void (*clear_stall)(uint8_t num, uint8_t dir); /*!< CLEAR_STALL request callback. Use it to resume an endpoint.*/
    uint8_t *(*device_descriptor)(void); /*!< Notifies the usbd_core of the device descriptor.*/
	uint8_t *(*configuration_descriptor)(uint8_t index); /*!< Notifies the usbd_core of a configuration descriptor.*/
	uint8_t *(*string_descriptor)(uint8_t index, uint16_t lang_id); /*!< Notifies the usbd_core of a string descriptor.*/
	uint8_t *(*bos_descriptor)(void); /*!< Notifies the usbd_core of the bos descriptor.*/
	void (*set_descriptor)(struct usbd_setup_packet_type setup); /*!< SET_DESCRIPTOR request callback.*/
	uint8_t (*get_configuration)(void); /*!< Notifies the usbd_core of current configuration number.*/
	bool (*is_configuration_valid)(uint8_t num); /*!< Notifies the usbd_core if the selected configuration is valid.*/
	void (*set_configuration)(uint8_t num); /*!< Callback that sets a configuration.*/
	uint8_t (*get_interface)(uint8_t num); /*!< Notifies the usbd_core of the alternative interface number for a selected interface.*/
	void (*set_interface)(uint8_t num, uint8_t alt); /*!< Callback that sets an alternate interface for a selected interface.*/
	void (*class_request)(struct usbd_setup_packet_type setup); /*!< type CLASS request callback.*/
	void (*vendor_request)(struct usbd_setup_packet_type setup); /*!< type VENDOR request callback.*/
	void (*suspend)(void); /*!< Callback that suspends the device.*/
	void (*wakeup)(void); /*!< Callback that wakesup the device.*/
	void (*sof)(void); /*!< Callback for start of frame.*/
};

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
void usbd_pma_read(uint16_t rx_addr, uint8_t* buf, uint16_t cnt);
void usbd_pma_write(uint16_t tx_addr, uint8_t* buf, uint16_t cnt);

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

void usbd_core_init(struct usbd_core_driver* core_driver);

#endif /*USBD_CORE_H*/