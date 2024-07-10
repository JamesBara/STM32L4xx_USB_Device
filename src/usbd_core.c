#include "usbd_core.h"
#include "usbd_request.h"
#include "usbd_hw.h"

/*@todo maybe remove this and have them all as static globals.*/
static struct usbd_device_core
{
	uint8_t *buf;
	uint32_t cnt;
	void (*stage)(void);
	void (*reception_completed)(void);
	
	usbd_device_state cur_state;
    usbd_device_state prev_state;
	
    uint16_t device_address;

	usbd_core_config *config;
	void (*ep_handler[8][2])(void);
};

static struct usbd_device_core core;


static void usbd_ep0_handler(void);
static void usbd_setup_stage(void);
static void usbd_data_out_stage(void);
static void usbd_data_in_stage(void);
static void usbd_status_in_stage(void);
static void usbd_status_out_stage(void);
static void usbd_parse_setup_packet(usbd_setup_packet_type setup);

static void usbd_get_status(usbd_setup_packet_type setup);
static void usbd_clear_feature(usbd_setup_packet_type setup);
static void usbd_set_feature(usbd_setup_packet_type setup);
static void usbd_set_address(usbd_setup_packet_type setup);
static void usbd_get_descriptor(usbd_setup_packet_type setup);
static void usbd_get_configuration(usbd_setup_packet_type setup);
static void usbd_set_configuration(usbd_setup_packet_type setup);
static void usbd_get_interface(usbd_setup_packet_type setup);
static void usbd_set_interface(usbd_setup_packet_type setup);
static void usbd_synch_frame(usbd_setup_packet_type setup);

static void usbd_ep0_handler(void)
{
	core.stage();
}

static void usbd_setup_stage(void)
{
	usbd_setup_packet_type setup;
	USBD_EP_CLEAR_CTR_RX(EP0);
	uint16_t count = USBD_PMA_GET_RX_COUNT(EP0);
	ASSERT(count == sizeof(usbd_setup_packet_type));
	usbd_pma_read(PMA_ADDR0_RX, (uint8_t*)&setup, count);
	usbd_parse_setup_packet(setup);
	USBD_EP_SET_RX_VALID(EP0);
}

static void usbd_data_in_stage(void)
{
	uint32_t cnt = MIN(USBD_FS_MAXPACKETSIZE, core.cnt);
	/*Decrement the leftover bytes.*/
	core.cnt -= cnt;
	
	USBD_EP_CLEAR_CTR_TX(EP0);
	/*If there is no leftover data, Data In stage is completed.*/
	if (!core.cnt)
	{
		USBD_EP_SET_KIND(EP0);
		core.stage = usbd_status_out_stage;
		return;
	}
	/*Increment the buffer.*/
	core.buf += cnt;
	/*Copy a packet to usb sram*/
	USBD_PMA_SET_TX_COUNT(EP0, MIN(EP0_COUNT, core.cnt));
	usbd_pma_write(core.buf, PMA_ADDR0_TX, MIN(EP0_COUNT, core.cnt));
	USBD_EP_SET_TX_VALID(EP0);
}

static void usbd_data_out_stage(void)
{
	USBD_EP_CLEAR_CTR_RX(EP0);
	/*Get the rx count and protect from underflow.*/
	uint32_t cnt = MIN(USBD_PMA_GET_RX_COUNT(EP0), core.cnt);
	
	usbd_pma_read(PMA_ADDR0_TX, core.buf, cnt);

	/*Decrement the leftover bytes.*/
	core.cnt -= cnt;

	/*If there is leftover data, increment the buffer pointer.*/
	if (core.cnt)
	{
		core.buf += cnt;
	}
	/*Otherwise the stage is completed.*/
	else
	{
		core.stage = usbd_status_in_stage;
	}
	USBD_EP_SET_RX_VALID(EP0);
}

static void usbd_status_in_stage(void)
{
	USBD_EP_CLEAR_CTR_TX(EP0);
	if (core.device_address && core.cur_state == USBD_DEFAULT_STATE)
	{
		core.cur_state = USBD_ADDRESSED_STATE;
		/*Set the device address.*/
		SET(USB->DADDR, core.device_address);
	}

	if (!core.device_address && core.cur_state == USBD_ADDRESSED_STATE)
	{
		core.cur_state = USBD_DEFAULT_STATE;
		/*Clear the device address*/
		CLEAR(USB->DADDR, USB_DADDR_ADD);
	}

	if(core.reception_completed != NULL)
	{
		core.reception_completed();
	}

	USBD_PMA_SET_TX_COUNT(EP0, 0);
	
	/*Clear the ep0 transfer.*/
	core.buf = NULL;
	core.cnt = 0;
	core.stage = NULL;
	USBD_EP_SET_TX_VALID(EP0);
}

static void usbd_status_out_stage(void)
{
	USBD_EP_CLEAR_CTR_RX(EP0);
	/*Clear hardware status out*/
	USBD_EP_CLEAR_KIND(EP0);
	/*Clear the ep0 transfer.*/
	core.buf = NULL;
	core.cnt = 0;
	core.stage = NULL;
	USBD_EP_SET_RX_VALID(EP0);
}

static void usbd_parse_setup_packet(usbd_setup_packet_type setup)
{
    switch(setup.bmRequestType & USBD_REQUEST_TYPE)
    {
        case USBD_REQUEST_TYPE_STANDARD:
        {
            switch(setup.bRequest)
            {
                case USBD_REQUEST_GET_STATUS:
                {
                    usbd_get_status(setup);
                    break;
                }
                case USBD_REQUEST_CLEAR_FEATURE:
                {
					usbd_clear_feature(setup);
                    break;
                }
                case USBD_REQUEST_SET_FEATURE:
                {
					usbd_set_feature(setup);
                    break;            
                }
                case USBD_REQUEST_SET_ADDRESS:
                {
					usbd_set_address(setup);
                    break;            
                }
                case USBD_REQUEST_GET_DESCRIPTOR:
                {
					usbd_get_descriptor(setup);
                    break;            
                }
                case USBD_REQUEST_SET_DESCRIPTOR:
                {
					ASSERT(core.config->set_descriptor != NULL);            
                    core.config->set_descriptor(setup);
                    break;            
                }
                case USBD_REQUEST_GET_CONFIGURATION:
                {
					usbd_get_configuration(setup);
                    break;            
                }
                case USBD_REQUEST_SET_CONFIGURATION:
                {
					usbd_set_configuration(setup);
                    break;            
                }
                case USBD_REQUEST_GET_INTERFACE:
                {                                     
                    usbd_get_interface(setup);
                    break;
                }
                case USBD_REQUEST_SET_INTERFACE:
                {
					usbd_set_interface(setup);
                    break;
                }
                case USBD_REQUEST_SYNCH_FRAME:
                {
					usbd_synch_frame(setup);
                    break;
                }
                default:
                {
                    USBD_DEV_ERR(Invalid bRequest.);
                    break;
                }
            }
            break;
        }
        case USBD_REQUEST_TYPE_CLASS:
        {
			ASSERT(core.config->class_request != NULL);
            core.config->class_request(setup);
            break;
        }
        case USBD_REQUEST_TYPE_VENDOR:
        {
			ASSERT(core.config->vendor_request != NULL);
			core.config->vendor_request(setup);
            break;
        }
        default:
        {
            USBD_DEV_ERR(Invalid bmRequestType. Unknown request type.);
        }
    }
}

static void usbd_get_status(usbd_setup_packet_type setup)
{
	uint8_t buf[2] = { 0x0U, 0x0U };

	switch (setup.bmRequestType & USBD_REQUEST_RECIPIENT)
	{
		case USBD_REQUEST_RECIPIENT_DEVICE:
		{
			ASSERT(core.config->get_remote_wakeup != NULL);
			ASSERT(core.config->is_selfpowered != NULL);
			buf[0] = core.config->get_remote_wakeup() ? 1 << 1 : 0 << 1;
			buf[0] |= core.config->is_selfpowered() ? 1 : 0;
			break;	
		}
		case USBD_REQUEST_RECIPIENT_INTERFACE:
		{
			ASSERT(core.config->is_interface_valid != NULL);

			if(!core.config->is_interface_valid(setup.wIndex & 0x7FU))
			{
				USBD_DEV_ERR(Invalid interface.);
				return;
			}
			break;
		}
		case USBD_REQUEST_RECIPIENT_ENDPOINT:
		{
			uint8_t ep = (setup.wIndex & 0xF);
			uint8_t dir =  (setup.wIndex & 0x80) ? 1 : 0;
			ASSERT(core.config->is_endpoint_valid != NULL);
			if(!core.config->is_endpoint_valid(ep, dir))
			{
				USBD_DEV_ERR(Invalid endpoint.);
				return;
			}
			buf[0] = (uint8_t) USBD_EP_GET_STALL(ep, dir) ? 1: 0;
			break;

		}
		default:
		{
			USBD_DEV_ERR(Invalid bmRequestType. Unknown recipient);
			return;
			break;			
		}
	}
	usbd_prepare_data_in_stage(buf, 2);
}

static void usbd_clear_feature(usbd_setup_packet_type setup)
{
	switch (setup.bmRequestType & USBD_REQUEST_RECIPIENT)
	{
		case USBD_REQUEST_RECIPIENT_DEVICE:
		{
			ASSERT(core.config->set_remote_wakeup != NULL);
			core.config->set_remote_wakeup(false);
			break;	
		}
		case USBD_REQUEST_RECIPIENT_ENDPOINT:
		{
			uint8_t ep = (setup.wIndex & 0xF);
			uint8_t dir =  (setup.wIndex & 0x80) ? 1 : 0;
			ASSERT(core.config->is_endpoint_valid != NULL);
			if(!core.config->is_endpoint_valid(ep, dir))
			{
				USBD_DEV_ERR(Invalid endpoint.);
				return;
			}
			ASSERT(core.config->clear_stall != NULL);
			core.config->clear_stall(ep, dir);
			break;
		}
		default:
		{
			USBD_DEV_ERR(Invalid bmRequestType. Unknown recipient);
			return;
			break;			
		}
	}
	usbd_prepare_status_in_stage();
}

static void usbd_set_feature(usbd_setup_packet_type setup)
{
	switch (setup.bmRequestType & USBD_REQUEST_RECIPIENT)
	{
		case USBD_REQUEST_RECIPIENT_DEVICE:
		{
			ASSERT(core.config->set_remote_wakeup != NULL);
			core.config->set_remote_wakeup(true);
			break;	
		}
		case USBD_REQUEST_RECIPIENT_ENDPOINT:
		{
			uint8_t ep = (setup.wIndex & 0xF);
			uint8_t dir =  (setup.wIndex & 0x80);
			ASSERT(core.config->is_endpoint_valid != NULL);
			if(!core.config->is_endpoint_valid(ep, dir))
			{
				USBD_DEV_ERR(Invalid endpoint.);
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
			USBD_DEV_ERR(Invalid bmRequestType. Unknown recipient);
			return;
			break;			
		}
	}
	usbd_prepare_status_in_stage();
}

static void usbd_set_address(usbd_setup_packet_type setup)
{
	core.device_address = setup.wValue;
	usbd_prepare_status_in_stage();
}

static void usbd_get_descriptor(usbd_setup_packet_type setup)
{
	uint8_t *buf = NULL;
	uint32_t cnt = 0;

	switch ((((setup.wValue) >> 0x8U) & 0xFFU))
	{
		case USBD_REQUEST_DESC_DEVICE:
		{
			ASSERT(core.config->device_descriptor != NULL);
			buf = core.config->device_descriptor();
			cnt = setup.wLength;
			break;
		}
		case USBD_REQUEST_DESC_CONFIGURATION:
		{
			ASSERT(core.config->configuration_descriptor != NULL);
			buf = core.config->configuration_descriptor(setup.wValue & 0xFFU);
			cnt = MIN(setup.wLength, (buf[2] | buf[3] << 8));
			break;
		}
		case USBD_REQUEST_DESC_STRING:
		{
			ASSERT(core.config->string_descriptor != NULL);
			buf = core.config->string_descriptor((setup.wValue & 0xFFU), setup.wIndex);
			cnt = MIN(setup.wLength, buf[0]);
			break;
		}		
		case USBD_REQUEST_DESC_BOS:
		{
			ASSERT(core.config->bos_descriptor != NULL);
			buf = core.config->bos_descriptor();
			cnt = setup.wLength;
			break;
		}
		default:
		{
			USBD_DEV_ERR(Invalid descriptor request);
			return;
			break;
		}
	}
	usbd_prepare_data_in_stage(buf, cnt);
}

static void usbd_get_configuration(usbd_setup_packet_type setup)
{
	UNUSED(setup);
	uint8_t buf = 0;
	ASSERT(core.config->get_configuration != NULL);
	buf = core.config->get_configuration();
	usbd_prepare_data_in_stage(&buf, 1);
}

static void usbd_set_configuration(usbd_setup_packet_type setup)
{
	uint8_t num = (setup.wValue & 0xFFU);
	ASSERT(core.config->is_configuration_valid != NULL);
	if(!core.config->is_configuration_valid(num))
	{
		USBD_DEV_ERR(Invalid configuration.);
		return;
	}
	ASSERT(core.config->set_configuration != NULL);
	core.config->set_configuration(num);
	core.cur_state = num ? USBD_CONFIGURED_STATE : USBD_ADDRESSED_STATE;
	usbd_prepare_status_in_stage();
}

static void usbd_get_interface(usbd_setup_packet_type setup)
{
	uint8_t num = (setup.wIndex & 0x7FU);
	uint8_t buf = 0;
	ASSERT(core.config->is_interface_valid != NULL);
	if(!core.config->is_interface_valid(num))
	{
		USBD_DEV_ERR(Invalid interface);
		return;
	}
	ASSERT(core.config->get_interface != NULL);
	buf = core.config->get_interface(num);
	usbd_prepare_data_in_stage(buf, 1);
}

static void usbd_set_interface(usbd_setup_packet_type setup)
{
	uint8_t num = (setup.wIndex & 0x7FU);
	uint8_t alt = (setup.wValue & 0xFFU);
	ASSERT(core.config->is_interface_valid != NULL);
	if (!core.config->is_interface_valid(num))
	{
		USBD_DEV_ERR(Invalid interface.);
	}
	ASSERT(core.config->set_interface != NULL);
	core.config->set_interface(num, alt);
	usbd_prepare_status_in_stage();	
}

static void usbd_synch_frame(usbd_setup_packet_type setup)
{
	uint8_t ep = (setup.wIndex & 0xFF);
	uint8_t buf[2];
	/*@todo check if endpoint iso*/
	/*@todo get frame number*/
	usbd_prepare_data_in_stage(buf, 2);
}






































































#if 0
void usbd_device_event_handler(usbd_device_driver *me, USBD_EVENT e)
{
	switch (e)
	{
		case RESET_EVENT:
		{
			me->hw->reset();
			break;
		}
		case SETUP_EVENT:
		{
			/*Force to setup stage.*/
			me->stage = usbd_setup_stage;
			/*fallthrough*/
			__attribute__((fallthrough));
		}
		case CTR_EVENT:
		{
			/*Callback to hardware function that handles the ctr event, 
			 *this function should callback the registered endpoint function.
			 * In case of endpoint 0 that should be usbd_ep0_handler.
			 * By using void* as a parameter we can pass the usbd_device_driver
			 * pointer, which will be upcasted when the registered endpoint callback
			 * is called.
			*/
			me->hw->correct_transfer(me);
			break;
		}
		case SUSPEND_EVENT:
		{
			/*Store the cur_state state.*/
			me->prev_state = me->cur_state;
			me->cur_state = USBD_SUSPENDED_STATE;
			me->hw->suspend();	
			break;
		}
		case WAKEUP_EVENT:
		{
			/*Restore the cur_state state.*/
			me->cur_state = me->prev_state;
			me->hw->wakeup();	
			break;
		}
		case NO_EVENT:
		{
			/*Do nothing.*/
			break;
		}
		default:
		{
			USBD_DEV_ERR(Unknown Event.);
			break;
		}		
	}
}
#endif



/**
 * @brief Initialize a single buffer bidirectional endpoint.
 * @param ep Endpoint number.
 * @param type Endpoint type.
 * @param tx_addr The address offset of the endpooint's IN buffer inside the Packet Memory Area.
 * @param rx_addr The address offset of the endpoint's OUT buffer inside the Packet Memory Area.
 * @param rx_count The size of the endpoint's OUT buffer inside the Packet Memory Area.
 * @param ep_in Pointer to function that handles the IN transaction of the endpoint.
 * @param ep_out Pointer to function that handles the OUT transaction of the endpoint.
*/
void usbd_register_ep(uint8_t ep, uint32_t type, uint16_t tx_addr, uint16_t rx_addr, uint16_t rx_count, void (*ep_in)(void), void (*ep_out)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_CONTROL) || (type == USB_EP_TYPE_INTERRUPT));
	ASSERT((tx_addr < PMA_SIZE) && (rx_addr < PMA_SIZE) && (rx_count < USBD_PMA_COUNT));
	ASSERT(ep_in != NULL);
	ASSERT(ep_out != NULL);
	core.ep_handler[ep][1] = ep_in;
	core.ep_handler[ep][0] = ep_out;
	USBD_EP_SET_CONF(ep, type, tx_addr, rx_addr, rx_count);
}

/**
 * @brief Copy data from a buffer, to a usb sram memory buffer.
 * @param src Pointer to uint8_t buffer, this is the buffer given.
 * @param dst Pointer to volatile uint16_t usb sram memory buffer, the pma address of an enpoint should be used.
 * @param cnt Amount of data to copy from buffer to usb sram buffer.
*/
void usbd_pma_write(uint8_t* src, __IO uint16_t* dst, uint16_t cnt)
{
	uint16_t half_cnt, tmp_val;
	half_cnt = (cnt >> 0x1U);

	/*Copy to packet buffer area.*/
	while (half_cnt--)
	{
		tmp_val = (uint16_t)(*src);
		src++;
		tmp_val |= (((uint16_t)(*src)) << 0x8U);
		*dst = tmp_val;
		dst++;
		src++;
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
 * @param src Pointer to volatile uint16_t usb sram memory buffer, the pma address of an enpoint should be used.
 * @param dst Pointer to uint8_t buffer, this is the buffer to copy the data to.
 * @param cnt Amount of data to copy from usb sram buffer to buffer. There is no error checking in this function.
*/
void usbd_pma_read(__IO uint16_t* src, uint8_t* dst, uint16_t cnt)
{
	uint16_t half_cnt, tmp_val;
	half_cnt = (cnt >> 0x1U);

	/*Copy from packet buffer area.*/
	while (half_cnt--)
	{
		tmp_val = *src;
		*dst = (uint8_t)(tmp_val & 0xFFU);
		dst++;
		*dst = (uint8_t)((tmp_val & 0xFF00U) >> 0x8U);
		src++;
		dst++;
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
	core.buf = buf;
	core.cnt = cnt;
	core.stage = usbd_data_in_stage;
	USBD_PMA_SET_TX_COUNT(EP0, MIN(EP0_COUNT, core.cnt));
	usbd_pma_write(core.buf, PMA_ADDR0_TX, MIN(EP0_COUNT, core.cnt));
	USBD_EP_SET_TX_VALID(EP0);
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
	ASSERT(rx_cplt != NULL);
	core.buf = buf;
	core.cnt = cnt;
	core.stage = usbd_data_out_stage;
	if (rx_cplt != NULL)
	{
		core.reception_completed = rx_cplt;
	}
	USBD_EP_SET_RX_VALID(EP0);
}

/**
 * @brief After parsing a setup packet use this function to acknowledge that the transaction is complete,
 * without need for further data transmission or reception.
*/
void usbd_prepare_status_in_stage(void)
{
	core.stage = usbd_status_in_stage;
	USBD_PMA_SET_TX_COUNT(EP0, 0);
	USBD_EP_SET_TX_VALID(EP0);
}


void usbd_core_init(usbd_core_config *conf)
{
	core.config = conf;
}