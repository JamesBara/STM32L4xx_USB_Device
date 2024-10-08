#include "assert_stm32l4xx.h"
#include "spinlock_stm32l4xx.h"
#include "usbd_core.h"

/************************************************
 * USB request callbacks.
 * These callbacks vary per state.
 ***********************************************/
struct usbd_core_state
{
	void (*get_status)(struct usbd_setup_packet_type setup);
	void (*clear_feature)(struct usbd_setup_packet_type setup);
	void (*set_feature)(struct usbd_setup_packet_type setup);
	void (*set_address)(struct usbd_setup_packet_type setup);
	void (*get_descriptor)(struct usbd_setup_packet_type setup);
	void (*set_descriptor)(struct usbd_setup_packet_type setup);
	void (*get_configuration)(struct usbd_setup_packet_type setup);
	void (*set_configuration)(struct usbd_setup_packet_type setup);
	void (*get_interface)(struct usbd_setup_packet_type setup);
	void (*set_interface)(struct usbd_setup_packet_type setup);
	void (*synch_frame)(struct usbd_setup_packet_type setup);
	void (*class_request)(struct usbd_setup_packet_type setup);
	void (*vendor_request)(struct usbd_setup_packet_type setup);
};

/************************************************
 * Static variables and callbacks 
 * used by the usbd core.
 ***********************************************/
static uint8_t *ep0_buf; /*!< Pointer to endpoint 0 buffer.*/
static __IO uint32_t ep0_cnt; /*!< endpoint 0 buffer data count.*/
static void (*__IO stage)(void); /*!< Pointer to current stage callback.*/
static void (*__IO reception_completed)(void); /*!< Stores a callback function, used to let the user know that a data reception in endpoint 0 has been completed. (Useful for class and/or vendor requests)*/
static struct usbd_core_state const __IO *cur_state; /*!< Pointer to current state of the device.*/
static struct usbd_core_state const __IO *prev_state; /*!< Pointer to previous state of the device.(Used to store the state when the device gets suspended)*/
static uint16_t device_address; /*!< Stores the device address.*/
static struct usbd_core_driver* __IO drv; /*!< Pointer to the configuration provided by the user during initialization.*/
static void (*__IO ep_handler[8][2])(void); /*!< Pointer to stored endpoint callback functions.*/

/************************************************
 * Function prototypes.
 ***********************************************/
static void usbd_ep0_handler(void);
static void usbd_setup_stage(void);
static void usbd_data_out_stage(void);
static void usbd_data_in_stage(void);
static void usbd_status_in_stage(void);
static void usbd_status_out_stage(void);

static void usbd_parse_setup_packet(struct usbd_setup_packet_type setup);

static void usbd_get_status(struct usbd_setup_packet_type setup);
static void usbd_clear_feature(struct usbd_setup_packet_type setup);
static void usbd_set_feature(struct usbd_setup_packet_type setup);
static void usbd_set_address(struct usbd_setup_packet_type setup);
static void usbd_get_descriptor(struct usbd_setup_packet_type setup);
static void usbd_set_descriptor(struct usbd_setup_packet_type setup);
static void usbd_get_configuration(struct usbd_setup_packet_type setup);
static void usbd_set_configuration(struct usbd_setup_packet_type setup);
static void usbd_get_interface(struct usbd_setup_packet_type setup);
static void usbd_set_interface(struct usbd_setup_packet_type setup);
static void usbd_synch_frame(struct usbd_setup_packet_type setup);
static void usbd_class_request(struct usbd_setup_packet_type setup);
static void usbd_vendor_request(struct usbd_setup_packet_type setup);

static void usbd_reset(void);
static void usbd_irq_handler(void);

/************************************************
 * Request callbacks for default state.
 ***********************************************/
static const struct usbd_core_state default_state =
{
	NULL,
	NULL,
	NULL,
	usbd_set_address,
	usbd_get_descriptor,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

/************************************************
 * Request callbacks for address state.
 ***********************************************/
static const struct usbd_core_state addressed_state =
{
	usbd_get_status,
	usbd_clear_feature,
	usbd_set_feature,
	usbd_set_address,
	usbd_get_descriptor,
	NULL,
	usbd_get_configuration,
	usbd_set_configuration,
	NULL,
	NULL,
	NULL,
	NULL,
	usbd_vendor_request
};

/************************************************
 * Request callbacks for configured state.
 ***********************************************/
static const struct usbd_core_state configured_state =
{
	usbd_get_status,
	usbd_clear_feature,
	usbd_set_feature,
	NULL,
	usbd_get_descriptor,
	usbd_set_descriptor,
	usbd_get_configuration,
	usbd_set_configuration,
	usbd_get_interface,
	usbd_set_interface,
	usbd_synch_frame,
	usbd_class_request,
	usbd_vendor_request
};

/************************************************
 * Request callbacks for suspended state.
 ***********************************************/
static const struct usbd_core_state suspended_state;

/**
 * @brief Endpoint 0 callback function.
 * @param  
 */
static void usbd_ep0_handler(void)
{
	ASSERT(stage != NULL);
	stage();
}

/**
 * @brief Setup stage callback function.
 * @param  
 */
static void usbd_setup_stage(void)
{
	struct usbd_setup_packet_type setup;
	uint16_t count = USBD_PMA_GET_RX_COUNT(EP0);
	
	if(count != USBD_SETUP_PACKET_SIZE)
	{
		USBD_EP0_SET_STALL();
	}

	usbd_pma_read(ADDR0_RX, (uint8_t*)&setup, count);
	usbd_parse_setup_packet(setup);
}

/**
 * @brief Data stage callback function, for IN direction.
 * @param  
 */
static void usbd_data_in_stage(void)
{
	uint32_t cnt = MIN(EP0_COUNT, ep0_cnt);
	/*Decrement the leftover bytes.*/
	ep0_cnt -= cnt;

	/*If there is no leftover data, Data In stage is completed.*/
	if (!ep0_cnt)
	{
		USBD_EP_SET_KIND(EP0);
		stage = usbd_status_out_stage;
		USBD_EP_SET_STAT_RX(EP0, USB_EP_STAT_RX_VALID);
		return;
	}
	/*If it's a short packet the opposite direction is set to NAK.*/
	if (ep0_cnt < EP0_COUNT)
	{
		USBD_EP_SET_STAT_RX(EP0, USB_EP_STAT_RX_NAK);
	}
	/*Increment the buffer.*/
	ep0_buf += cnt;
	/*Copy a packet to usb sram*/
	USBD_PMA_SET_TX_COUNT(EP0, MIN(EP0_COUNT, ep0_cnt));
	usbd_pma_write(ADDR0_TX, ep0_buf, MIN(EP0_COUNT, ep0_cnt));
	USBD_EP_SET_STAT_TX(EP0, USB_EP_STAT_TX_VALID);
}

/**
 * @brief Data stage callback function for OUT direction.
 * @param  
 */
static void usbd_data_out_stage(void)
{
	/*Get the rx count and protect from underflow.*/
	uint32_t cnt = MIN(USBD_PMA_GET_RX_COUNT(EP0), ep0_cnt);
	usbd_pma_read(ADDR0_RX, ep0_buf, cnt);
	/*Decrement the leftover bytes.*/
	ep0_cnt -= cnt;

	if (ep0_cnt < EP0_COUNT)
	{
		USBD_EP_SET_STAT_TX(EP0, USB_EP_STAT_TX_NAK);
	}
	/*If there is leftover data, increment the buffer pointer.*/
	if (ep0_cnt)
	{
		ep0_buf += cnt;
		USBD_EP_SET_STAT_RX(EP0, USB_EP_STAT_RX_VALID);
	}
	/*Otherwise the stage is completed.*/
	else
	{
		stage = usbd_status_in_stage;
		USBD_EP_SET_STAT_TX(EP0, USB_EP_STAT_TX_VALID);
	}
}

/**
 * @brief Status stage callback function for IN direction.
 * @param  
 */
static void usbd_status_in_stage(void)
{
	if (device_address && cur_state == &default_state)
	{
		cur_state = &addressed_state;
		/*Set the device address.*/
		SET(USB->DADDR, device_address);
	}

	if (!device_address && cur_state == &addressed_state)
	{
		cur_state = &default_state;
		/*Clear the device address*/
		CLEAR(USB->DADDR, USB_DADDR_ADD);
	}

	if(reception_completed != NULL)
	{
		reception_completed();
	}
	/*Clear the ep0 transfer.*/
	ep0_buf = NULL;
	ep0_cnt = 0;
	stage = NULL;
	USBD_EP_SET_STAT_RX(EP0, USB_EP_STAT_RX_VALID);
}

/**
 * @brief Status stage callback function for OUT direction.
 * @param  
 */
static void usbd_status_out_stage(void)
{
	/*Clear hardware status out*/
	USBD_EP_CLEAR_KIND(EP0);
	/*Clear the ep0 transfer.*/
	ep0_buf = NULL;
	ep0_cnt = 0;
	stage = NULL;
	USBD_EP_SET_STAT_RX(EP0, USB_EP_STAT_RX_VALID);
}

/**
 * @brief Parses the received setup packet and calls 
 * the appropriate callback function depending on 
 * the current device state.
 * @param setup USB setup packet.
 */
static void usbd_parse_setup_packet(struct usbd_setup_packet_type setup)
{
    switch(setup.bmRequestType & USBD_TYPE)
    {
        case USBD_TYPE_STANDARD:
        {
            switch(setup.bRequest)
            {
                case USBD_GET_STATUS:
                {
					if(cur_state->get_status == NULL)
					{						
						USBD_EP0_SET_STALL();
						return;
					}
                    cur_state->get_status(setup);
                    break;
                }
                case USBD_CLEAR_FEATURE:
                {
					if(cur_state->clear_feature == NULL)
					{					
						USBD_EP0_SET_STALL();
						return;
					}
					cur_state->clear_feature(setup);
                    break;
                }
                case USBD_SET_FEATURE:
                {
					if(cur_state->set_feature == NULL)
					{
						USBD_EP0_SET_STALL();
						return;
					}
					cur_state->set_feature(setup);
                    break;            
                }
                case USBD_SET_ADDRESS:
                {
					if(cur_state->set_address == NULL)
					{
						USBD_EP0_SET_STALL();
						return;
					}
					cur_state->set_address(setup);
                    break;            
                }
                case USBD_GET_DESCRIPTOR:
                {
					if(cur_state->get_descriptor == NULL)
					{
						USBD_EP0_SET_STALL();
						return;
					}
					cur_state->get_descriptor(setup);
                    break;            
                }
                case USBD_SET_DESCRIPTOR:
                {
					if(cur_state->set_descriptor == NULL)
					{
						USBD_EP0_SET_STALL();
						return;
					}
                    cur_state->set_descriptor(setup);
                    break;            
                }
                case USBD_GET_CONFIGURATION:
                {
					if(cur_state->get_configuration == NULL)
					{
						USBD_EP0_SET_STALL();
						return;
					}
                    cur_state->get_configuration(setup);
                    break;
                }
                case USBD_SET_CONFIGURATION:
                {
					if(cur_state->set_configuration == NULL)
					{
						USBD_EP0_SET_STALL();
						return;
					}
					cur_state->set_configuration(setup);
                    break;            
                }
                case USBD_GET_INTERFACE:
                {
					if(cur_state->get_interface == NULL)
					{
						USBD_EP0_SET_STALL();
						return;
					}
                    cur_state->get_interface(setup);
                    break;
                }
                case USBD_SET_INTERFACE:
                {
					if(cur_state->set_interface == NULL)
					{
						USBD_EP0_SET_STALL();
						return;
					}
                    cur_state->set_interface(setup);
                    break;
                }
                case USBD_SYNCH_FRAME:
                {
					if(cur_state->synch_frame == NULL)
					{
						USBD_EP0_SET_STALL();
						return;
					}
                    cur_state->synch_frame(setup);
                    break;
                }
                default:
                {
					USBD_EP0_SET_STALL();
                    break;
                }
            }
            break;
        }
        case USBD_TYPE_CLASS:
        {
			if(cur_state->class_request == NULL)
			{
				USBD_EP0_SET_STALL();
				return;
			}
            cur_state->class_request(setup);
            break;
        }
        case USBD_TYPE_VENDOR:
        {
			if(cur_state->vendor_request == NULL)
			{
				USBD_EP0_SET_STALL();
				return;
			}
			cur_state->vendor_request(setup);
            break;
        }
        default:
        {
			USBD_EP0_SET_STALL();
        }
    }
}

/**
 * @brief USB get status callback function.
 * @param setup USB setup packet.
 */
static void usbd_get_status(struct usbd_setup_packet_type setup)
{
	uint8_t buf[2] = { 0x0U, 0x0U };

	switch (setup.bmRequestType & USBD_RECIPIENT)
	{
		case USBD_RECIPIENT_DEVICE:
		{
			ASSERT(drv != NULL);
			ASSERT(drv->get_remote_wakeup != NULL);
			ASSERT(drv->is_selfpowered != NULL);
			buf[0] = drv->get_remote_wakeup() ? 1 << 1 : 0 << 1;
			buf[0] |= drv->is_selfpowered() ? 1 : 0;
			break;
		}
		case USBD_RECIPIENT_INTERFACE:
		{
			ASSERT(drv != NULL);
			ASSERT(drv->is_interface_valid != NULL);
			if(!drv->is_interface_valid(setup.wIndex & 0x7FU))
			{
				USBD_EP0_SET_STALL();
				return;
			}
			break;
		}
		case USBD_RECIPIENT_ENDPOINT:
		{
			uint8_t ep = (setup.wIndex & USBD_EP_ADDRESS_EP_NUMBER);
			uint8_t dir =  (setup.wIndex & USBD_EP_ADDRESS_EP_DIRECTION) ? 1 : 0;
			ASSERT(drv != NULL);
			ASSERT(drv->is_endpoint_valid != NULL);
			if(!drv->is_endpoint_valid(ep, dir))
			{
				USBD_EP0_SET_STALL();
				return;
			}
			buf[0] = (uint8_t) USBD_EP_GET_STALL(ep, dir) ? 1: 0;
			break;

		}
		default:
		{
			USBD_EP0_SET_STALL();
			return;
			break;			
		}
	}
	usbd_prepare_data_in_stage(buf, USBD_GET_STATUS_LENGTH);
}

/**
 * @brief USB clear feature callback function.
 * @param setup USB setup packet.
 */
static void usbd_clear_feature(struct usbd_setup_packet_type setup)
{
	switch (setup.bmRequestType & USBD_RECIPIENT)
	{
		case USBD_RECIPIENT_DEVICE:
		{
			ASSERT(drv != NULL);
			ASSERT(drv->set_remote_wakeup != NULL);
			drv->set_remote_wakeup(false);
			break;	
		}
		case USBD_RECIPIENT_ENDPOINT:
		{
			uint8_t ep = (setup.wIndex & USBD_EP_ADDRESS_EP_NUMBER);
			uint8_t dir =  (setup.wIndex & USBD_EP_ADDRESS_EP_DIRECTION) ? 1 : 0;
			ASSERT(drv != NULL);
			ASSERT(drv->is_endpoint_valid != NULL);
			if(!drv->is_endpoint_valid(ep, dir))
			{
				USBD_EP0_SET_STALL();
				return;
			}
			ASSERT(drv->clear_stall != NULL);
			drv->clear_stall(ep, dir);
			break;
		}
		default:
		{
			USBD_EP0_SET_STALL();
			return;
			break;			
		}
	}
	usbd_prepare_status_in_stage();
}

/**
 * @brief USB set feature callback function.
 * @param setup USB setup packet.
 */
static void usbd_set_feature(struct usbd_setup_packet_type setup)
{
	switch (setup.bmRequestType & USBD_RECIPIENT)
	{
		case USBD_RECIPIENT_DEVICE:
		{
			ASSERT(drv != NULL);
			ASSERT(drv->set_remote_wakeup != NULL);
			drv->set_remote_wakeup(true);
			break;	
		}
		case USBD_RECIPIENT_ENDPOINT:
		{
			uint8_t ep = (setup.wIndex & USBD_EP_ADDRESS_EP_NUMBER);
			uint8_t dir =  (setup.wIndex & USBD_EP_ADDRESS_EP_DIRECTION) ? 1 : 0;
			ASSERT(drv != NULL);
			ASSERT(drv->is_endpoint_valid != NULL);
			if(!drv->is_endpoint_valid(ep, dir))
			{
				USBD_EP0_SET_STALL();
				return;
			}
			if (dir)
			{
				USBD_EP_SET_TX_STALL(ep);
			}
			else
			{
				USBD_EP_SET_RX_STALL(ep);
			}
			break;
		}
		default:
		{
			USBD_EP0_SET_STALL();
			return;
			break;			
		}
	}
	usbd_prepare_status_in_stage();
}

/**
 * @brief USB set address callback function.
 * @param setup USB setup packet.
 */
static void usbd_set_address(struct usbd_setup_packet_type setup)
{
	device_address = setup.wValue;
	usbd_prepare_status_in_stage();
}

/**
 * @brief USB get descriptor callback function.
 * @param setup USB setup packet.
 */
static void usbd_get_descriptor(struct usbd_setup_packet_type setup)
{
	uint8_t *buf = NULL;
	uint32_t cnt = 0;

	switch ((((setup.wValue) >> 0x8U) & 0xFFU))
	{
		case USBD_DESC_TYPE_DEVICE:
		{
			ASSERT(drv != NULL);
			ASSERT(drv->device_descriptor != NULL);
			buf = drv->device_descriptor();
			cnt = MIN(setup.wLength, buf[0]);
			break;
		}
		case USBD_DESC_TYPE_CONFIGURATION:
		{
			ASSERT(drv != NULL);
			ASSERT(drv->configuration_descriptor != NULL);
			buf = drv->configuration_descriptor(setup.wValue & 0xFFU);
			cnt = MIN(setup.wLength, (buf[2] | buf[3] << 8));
			break;
		}
		case USBD_DESC_TYPE_STRING:
		{
			ASSERT(drv != NULL);
			ASSERT(drv->string_descriptor != NULL);
			buf = drv->string_descriptor((setup.wValue & 0xFFU), setup.wIndex);
			cnt = MIN(setup.wLength, buf[0]);
			break;
		}		
		case USBD_DESC_TYPE_BOS:
		{
			ASSERT(drv != NULL);
			ASSERT(drv->bos_descriptor != NULL);
			buf = drv->bos_descriptor();
			cnt = setup.wLength;
			break;
		}
		default:
		{
			USBD_EP0_SET_STALL();
			return;
			break;
		}
	}
	usbd_prepare_data_in_stage(buf, cnt);
}

/**
 * @brief USB set descriptor callback function.
 * @param setup USB setup packet.
 */
static void usbd_set_descriptor(struct usbd_setup_packet_type setup)
{
	ASSERT(drv != NULL);
	ASSERT(drv->set_descriptor != NULL);
	drv->set_descriptor(setup);
}

/**
 * @brief USB get descriptor callback function.
 * @param setup USB setup packet.
 */
static void usbd_get_configuration(struct usbd_setup_packet_type setup)
{
	UNUSED(setup);
	uint8_t buf = 0;
	ASSERT(drv != NULL);
	ASSERT(drv->get_configuration != NULL);
	buf = drv->get_configuration();
	usbd_prepare_data_in_stage(&buf, USBD_GET_CONFIGURATION_LENGTH);
}

/**
 * @brief USB set configuration callback function.
 * @param setup USB setup packet.
 */
static void usbd_set_configuration(struct usbd_setup_packet_type setup)
{
	uint8_t num = (setup.wValue & 0xFFU);
	ASSERT(drv != NULL);
	ASSERT(drv->is_configuration_valid != NULL);
	if(!drv->is_configuration_valid(num))
	{
		USBD_EP0_SET_STALL();
		return;
	}
	ASSERT(drv->set_configuration != NULL);
	drv->set_configuration(num);
	cur_state = num ? &configured_state : &addressed_state;
	usbd_prepare_status_in_stage();
}

/**
 * @brief USB get interface callback function.
 * @param setup USB setup packet.
 */
static void usbd_get_interface(struct usbd_setup_packet_type setup)
{
	uint8_t num = (setup.wIndex & 0x7FU);
	uint8_t buf = 0;
	ASSERT(drv != NULL);
	ASSERT(drv->is_interface_valid != NULL);
	if(!drv->is_interface_valid(num))
	{
		USBD_EP0_SET_STALL();
		return;
	}
	ASSERT(drv->get_interface != NULL);
	buf = drv->get_interface(num);
	usbd_prepare_data_in_stage(&buf, USBD_GET_INTERFACE_LENGTH);
}

/**
 * @brief USB set interface callback function.
 * @param setup USB setup packet.
 */
static void usbd_set_interface(struct usbd_setup_packet_type setup)
{
	uint8_t num = (setup.wIndex & 0x7FU);
	uint8_t alt = (setup.wValue & 0xFFU);
	ASSERT(drv != NULL);
	ASSERT(drv->is_interface_valid != NULL);
	if (!drv->is_interface_valid(num))
	{
		USBD_EP0_SET_STALL();		
	}
	ASSERT(drv->set_interface != NULL);
	drv->set_interface(num, alt);
	usbd_prepare_status_in_stage();	
}

/**
 * @brief USB synch frame callback function.
 * @todo Unsure how to implement.
 * @param setup USB setup packet.
 */
static void usbd_synch_frame(struct usbd_setup_packet_type setup)
{
	UNUSED(setup);
}

/**
 * @brief USB class specific request callback function.
 * @param setup USB setup packet.
 */
static void usbd_class_request(struct usbd_setup_packet_type setup)
{
	ASSERT(drv != NULL);
	ASSERT(drv->class_request != NULL);
	drv->class_request(setup);
}

/**
 * @brief USB vendor specific request callback function.
 * @param setup USB setup packet.
 */
static void usbd_vendor_request(struct usbd_setup_packet_type setup)
{
	ASSERT(drv != NULL);
	ASSERT(drv->vendor_request != NULL);
	drv->vendor_request(setup);
}

/**
 * @brief Resets the usb device.
 * @param  
 */
static void usbd_reset(void)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		usbd_unregister_ep(i);
	}
	usbd_register_ep(EP0, USB_EP_TYPE_CONTROL, ADDR0_TX, ADDR0_RX, EP0_COUNT, usbd_ep0_handler, usbd_ep0_handler);
	ep0_buf = NULL;
	ep0_cnt = 0;
	cur_state = &default_state;
	USB->DADDR = USB_DADDR_EF;
}

/**
 * @brief Handle the usb interrupts.
 * @note This function should be called by USB_IRQHandler interrupt callback.
 * @param  
 */
static void usbd_irq_handler(void)
{
	uint32_t istr = USB->ISTR;

	if (GET(istr, USB_ISTR_CTR))
	{
		uint8_t ep = GET(istr, USB_EP_EA);
		uint8_t dir = GET(*USBD_EP_REG(ep), USB_EP_CTR_TX) ? 1 : 0;
		if (dir)
		{
			USBD_EP_CLEAR_CTR_TX(ep);
		}
		else
		{
			USBD_EP_CLEAR_CTR_RX(ep);
		}

		if (USBD_EP_GET_SETUP(ep))
		{
			stage = usbd_setup_stage;
		}
		ASSERT(ep_handler[ep][dir] != NULL);
		ep_handler[ep][dir]();
	}

	if (GET(istr, USB_ISTR_RESET))
	{
		CLEAR(istr, USB_ISTR_RESET);
		usbd_reset();
	}

	if (GET(istr, USB_ISTR_WKUP))
	{
		CLEAR(istr, USB_ISTR_WKUP);
		cur_state = prev_state;
		prev_state = NULL;
		if (drv->wakeup != NULL)
		{
			drv->wakeup();
		}
	}

	if (GET(istr, USB_ISTR_SUSP))
	{
		CLEAR(istr, USB_ISTR_SUSP);
		prev_state = cur_state;
		cur_state = &suspended_state;
		if (drv->suspend != NULL)
		{
			drv->suspend();
		}	

	}	

	if (GET(istr, USB_ISTR_SOF))
	{
		CLEAR(istr, USB_ISTR_SOF);
		if (drv->sof != NULL)
		{
			drv->sof();
		}
	}

	USB->ISTR = istr;
}

/**
 * @brief Initialize a single buffer bidirectional endpoint.
 * @param ep Endpoint number.
 * @param type Endpoint type.
 * @param tx_addr The address offset of the endpoint's IN buffer inside the Packet Memory Area.
 * @param rx_addr The address offset of the endpoint's OUT buffer inside the Packet Memory Area.
 * @param rx_count The size of the endpoint's OUT buffer inside the Packet Memory Area.
 * @param ep_in Pointer to callback function that handles the IN transaction of the endpoint.
 * @param ep_out Pointer to callback function that handles the OUT transaction of the endpoint.
*/
void usbd_register_ep(uint8_t ep, uint32_t type, uint16_t tx_addr, uint16_t rx_addr, uint16_t rx_count, void (*ep_in)(void), void (*ep_out)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_CONTROL) || (type == USB_EP_TYPE_INTERRUPT));
	ASSERT((tx_addr < PMA_SIZE) && (rx_addr < PMA_SIZE) && (rx_count < USBD_PMA_COUNT));
	ASSERT(ep_in != NULL);
	ASSERT(ep_out != NULL);
	ep_handler[ep][1] = ep_in;
	ep_handler[ep][0] = ep_out;
	USBD_EP_SET_CONF(ep, type, tx_addr, rx_addr, rx_count);
}

/**
 * @brief Initialize a single buffer unidirectional IN endpoint.
 * @param ep Endpoint number.
 * @param type Endpoint type.
 * @param tx_addr The address offset of the endpoint's IN buffer inside the Packet Memory Area.
 * @param ep_in Pointer to callback function that handles the IN transaction of the endpoint.
 */
void usbd_register_ep_tx(uint8_t ep, uint32_t type, uint32_t tx_addr, void (*ep_in)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_CONTROL) || (type == USB_EP_TYPE_INTERRUPT));
	ASSERT(tx_addr < PMA_SIZE);
	ASSERT(ep_in != NULL);
	ep_handler[ep][1] = ep_in;
	USBD_EP_SET_CONF(ep, type, tx_addr, 0, 0);
}

/**
 * @brief Initialize a single buffer unidirectional OUT endpoint.
 * @param ep Endpoint number.
 * @param type Endpoint type.
 * @param rx_addr The address offset of the endpoint's OUT buffer inside the Packet Memory Area.
 * @param rx_count The size of the endpoint's OUT buffer inside the Packet Memory Area.
 * @param ep_out Pointer to callback function that handles the OUT transaction of the endpoint.
 */
void usbd_register_ep_rx(uint8_t ep, uint32_t type, uint32_t rx_addr, uint32_t rx_count, void (*ep_out)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_CONTROL) || (type == USB_EP_TYPE_INTERRUPT));
	ASSERT((rx_addr < PMA_SIZE) && (rx_count < USBD_PMA_COUNT));
	ASSERT(ep_out != NULL);
	ep_handler[ep][0] = ep_out;
	USBD_EP_SET_CONF(ep, type, 0, rx_addr, rx_count);
}

/**
 * @brief Initialize a double buffer unidirectional IN endpoint.
 * @param ep Endpoint number.
 * @param type Endpoint type.
 * @param tx0_addr The address offset of the endpoint's IN 0 buffer inside the Packet Memory Area.
 * @param tx1_addr The address offset of the endpoint's IN 1 buffer inside the Packet Memory Area.
 * @param ep_in Pointer to callback function that handles the IN transaction of the endpoint.
 */
void usbd_register_ep_dbl_tx(uint8_t ep, uint32_t type, uint32_t tx0_addr, uint32_t tx1_addr, void (*ep_in)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_ISOCHRONOUS));
	ASSERT((tx0_addr < PMA_SIZE) && (tx1_addr < PMA_SIZE));
	ASSERT(ep_in != NULL);
	ep_handler[ep][1] = ep_in;
	USBD_EP_SET_DBL_TX_CONF(ep, type, tx0_addr, tx1_addr);
}

/**
 * @brief Initialize a double buffer unidirectional OUT endpoint.
 * @param ep Endpoint number.
 * @param type Endpoint type.
 * @param rx0_addr The address offset of the endpoint's OUT 0 buffer inside the Packet Memory Area.
 * @param rx1_addr The address offset of the endpoint's OUT 1 buffer inside the Packet Memory Area.
 * @param rx_count The size of the endpoint's OUT buffer inside the Packet Memory Area.
 * @param ep_out Pointer to callback function that handles the OUT transaction of the endpoint.
 */
void usbd_register_ep_dbl_rx(uint8_t ep, uint32_t type, uint32_t rx0_addr, uint32_t rx1_addr, uint32_t rx_count, void (*ep_out)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_ISOCHRONOUS));
	ASSERT((rx0_addr < PMA_SIZE) && (rx1_addr < PMA_SIZE) && (rx_count < USBD_PMA_COUNT));
	ASSERT(ep_out != NULL);
	ep_handler[ep][0] = ep_out;
	USBD_EP_SET_DBL_RX_CONF(ep, type, rx0_addr, rx1_addr, rx_count);	
}

/**
 * @brief Uninitialize an endpoint.
 * @param ep Endpoint number.
 */
void usbd_unregister_ep(uint8_t ep)
{
	ASSERT(ep < 8);
	ep_handler[ep][1] = NULL;
	ep_handler[ep][0] = NULL;
	USBD_EP_CLEAR_CONF(ep);
}

/**
 * @brief Copy data from a buffer, to a usb sram memory buffer.
 * @param tx_addr Offset of the pma address of the enpoint to write to.
 * @param buf Pointer to uint8_t buffer, this is the buffer to copy the data to.
 * @param cnt Amount of data to copy from buffer to usb sram buffer.
*/
void usbd_pma_write(uint16_t tx_addr, uint8_t* buf, uint16_t cnt)
{
	uint8_t* src = buf;
	uint16_t half_cnt = (cnt >> 0x1U), tmp_val;
	__IO uint16_t* dst = (__IO uint16_t*) (PMA_BASE + tx_addr);

	/*Copy to packet buffer area.*/
	while (half_cnt--)
	{
		tmp_val = (uint16_t)(*src++);
		tmp_val |= (((uint16_t)(*src++)) << 0x8U);
		*dst++ = tmp_val;
	}

	/*If the size is odd copy the leftover bytes.*/
	if (cnt & 0x1U)
	{
		tmp_val = (uint16_t)(*src);
		*dst = tmp_val;
	}
}

/**
 * @brief Copy data from a usb sram memory buffer, to a buffer.
 * @param rx_addr Offset of the pma address of the enpoint to read from.
 * @param buf Pointer to uint8_t buffer, this is the buffer to copy the data to.
 * @param cnt Amount of data to copy from usb sram buffer to buffer. There is no error checking in this function.
*/
void usbd_pma_read(uint16_t rx_addr, uint8_t* buf, uint16_t cnt)
{
	uint8_t* dst = buf;
	uint16_t half_cnt = (cnt >> 0x1U), tmp_val;
	__IO uint16_t* src = (__IO uint16_t*) (PMA_BASE + rx_addr);

	/*Copy from packet buffer area.*/
	while (half_cnt--)
	{
		tmp_val = *src++;
		*dst++ = (uint8_t)(tmp_val & 0xFFU);
		*dst++ = (uint8_t)((tmp_val & 0xFF00U) >> 0x8U);
	}

	/*If the size is odd copy the leftover bytes.*/
	if (cnt & 0x1U)
	{
		tmp_val = *src;
		*dst = (uint8_t)(tmp_val & 0xFFU);
	}
}

/**
 * @brief After parsing a setup packet use this function to transmit data over endpoint 0. 
 * @param buf Pointer to uint8_t buffer, that will be used to transmit data.
 * @param cnt Size of buffer.
*/
void usbd_prepare_data_in_stage(uint8_t* buf, uint32_t cnt)
{
	ASSERT(buf != NULL);
	ep0_buf = buf;
	ep0_cnt = cnt;
	stage = usbd_data_in_stage;

	if (ep0_cnt >= EP0_COUNT)
	{

		USBD_EP_SET_STAT_RX(EP0, USB_EP_STAT_RX_STALL);
	}
	else
	{
		USBD_EP_SET_STAT_RX(EP0, USB_EP_STAT_RX_NAK);
	}
	USBD_PMA_SET_TX_COUNT(EP0, MIN(EP0_COUNT, ep0_cnt));
	usbd_pma_write(ADDR0_TX, ep0_buf, MIN(EP0_COUNT, ep0_cnt));
	USBD_EP_SET_STAT_TX(EP0, USB_EP_STAT_TX_VALID);
}

/**
 * @brief After parsing a setup packet use this function to receive data over endpoint 0.
 * @param buf Pointer to uint8_t buffer, that will be used to store the data.
 * @param cnt Size of buffer.
 * @param rx_cplt Pointer to function that will be called once the data reception has been completed.
*/
void usbd_prepare_data_out_stage(uint8_t* buf, uint32_t cnt, void (*rx_cplt)(void))
{
	ASSERT(buf != NULL);
	ASSERT(cnt);
	ep0_buf = buf;
	ep0_cnt = cnt;
	stage = usbd_data_out_stage;
	/*Store the pointer to the callback*/
	if (rx_cplt != NULL)
	{
		reception_completed = rx_cplt;
	}
	/*Prepare the other direction*/
	if (ep0_cnt > EP0_COUNT)
	{
		USBD_EP_SET_STAT_TX(EP0, USB_EP_STAT_TX_STALL);
	}
	else
	{
		USBD_EP_SET_STAT_TX(EP0, USB_EP_STAT_TX_NAK);
	}

	USBD_EP_SET_STAT_RX(EP0, USB_EP_STAT_RX_VALID);
}

/**
 * @brief After parsing a setup packet use this function to acknowledge that the transaction is complete,
 * without need for further data transmission or reception. In simple english send a zlp.
*/
void usbd_prepare_status_in_stage(void)
{
	stage = usbd_status_in_stage;
	USBD_PMA_SET_TX_COUNT(EP0, 0);
	USBD_EP_SET_STAT_TX(EP0, USB_EP_STAT_TX_VALID);
}

/**
 * @brief Initializes the usbd_core.
 * @param core_driver Pointer to usbd_core_driver struct that provides callback implementations.
 */
void usbd_core_init(struct usbd_core_driver* core_driver)
{
	ASSERT(core_driver != NULL);
	drv = core_driver;
	cur_state = &default_state;

	/*Prepare the hardware.*/
	__IO uint16_t *reg = (uint16_t*)PMA_BASE;
	CLEAR(USB->CNTR, USB_CNTR_PDWN);
	/*1 microsecond delay is needed for stm32l412 according to the datasheet.*/
	__spinlock(1);
	USB->BTABLE = 0;

	/*Clear the SRAM.*/
	for (uint16_t i = 0; i < (PMA_SIZE >> 0x1U); i++)
	{
		*reg++ = 0x0U;
	}
	/*Remove force reset.*/
	CLEAR(USB->CNTR, USB_CNTR_FRES);
	/*Enable interrupts.*/
	SET(USB->CNTR, (USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_SUSPM | USB_CNTR_WAKEUPM | USB_CNTR_SOFM));
	/*Clear pending interrupts*/
	USB->ISTR = 0x0U;

	/*Enable the usb DP pullup to connect to host.*/
	SET(USB->BCDR, USB_BCDR_DPPU);
}

/**
 * @brief Implementation of the weak function USB_IRQHandler.
 * @param  
 */
void USB_IRQHandler(void)
{
	usbd_irq_handler();
}