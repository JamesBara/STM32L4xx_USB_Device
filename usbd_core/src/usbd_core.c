#include "stm32_assert.h"
#include "stm32_cpu_delay.h"
#include "usbd_core.h"
#include "usbd_request.h"
#include "usbd_hw.h"

#if 0
enum usbd_event
{
	NONE = 0,
	RESET,
	CTR,
	SETUP,
	SUSPEND,
	WAKEUP,
	PMAOVR,
	SOF,
	ESOF,
	LPM_L1,
	ERROR
};
#define USBD_MAX_EVENT 11
#endif

struct usbd_requests
{
	void (*get_status)(usbd_setup_packet_type setup);
	void (*clear_feature)(usbd_setup_packet_type setup);
	void (*set_feature)(usbd_setup_packet_type setup);
	void (*set_address)(usbd_setup_packet_type setup);
	void (*get_descriptor)(usbd_setup_packet_type setup);
	void (*set_descriptor)(usbd_setup_packet_type setup);
	void (*get_configuration)(usbd_setup_packet_type setup);
	void (*set_configuration)(usbd_setup_packet_type setup);
	void (*get_interface)(usbd_setup_packet_type setup);
	void (*set_interface)(usbd_setup_packet_type setup);
	void (*synch_frame)(usbd_setup_packet_type setup);
	void (*class_request)(usbd_setup_packet_type setup);
	void (*vendor_request)(usbd_setup_packet_type setup);
};

static uint8_t *ep0_buf;
static uint32_t ep0_cnt;
static void (*stage)(void);
static void (*reception_completed)(void);
static struct usbd_requests const *cur_state;
static struct usbd_requests const *prev_state;
static uint16_t device_address;
static usbd_core_config *config;
static void (*ep_handler[8][2])(void);
#if 0
static void (*cur_ep_handler)(void);


static struct usbd_queue
{
	enum usbd_event buffer[USBD_MAX_EVENT + 1];
	uint32_t head;
	uint32_t tail;
}queue;

#define USBD_QUEUE_NEXT_HEAD() ((queue.head + 1 == USBD_MAX_EVENT) ? 0 : (queue.head + 1))
#define USBD_QUEUE_NEXT_TAIL() ((queue.tail + 1 == USBD_MAX_EVENT) ? 0 : (queue.tail + 1))
#define USBD_QUEUE_IS_EMPTY() ((queue.head == queue.tail) ? 1 : 0)
#define USBD_QUEUE_IS_FULL() ((USBD_QUEUE_NEXT_HEAD() == queue.tail) ? 1 : 0)
#endif
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
static void usbd_set_descriptor(usbd_setup_packet_type setup);
static void usbd_get_configuration(usbd_setup_packet_type setup);
static void usbd_set_configuration(usbd_setup_packet_type setup);
static void usbd_get_interface(usbd_setup_packet_type setup);
static void usbd_set_interface(usbd_setup_packet_type setup);
static void usbd_synch_frame(usbd_setup_packet_type setup);
static void usbd_class_request(usbd_setup_packet_type setup);
static void usbd_vendor_request(usbd_setup_packet_type setup);

#if 0
static void usbd_event_enqueue(enum usbd_event e);
static enum usbd_event usbd_event_dequeue(void);
#endif
static void usbd_event_generator(void);
static void usbd_reset(void);
#if 0
static void usbd_device_event_handler(enum usbd_event e);
#endif

static const struct usbd_requests default_state =
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

static const struct usbd_requests addressed_state =
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

static const struct usbd_requests configured_state =
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

static const struct usbd_requests suspended_state;

static void usbd_ep0_handler(void)
{
	ASSERT(stage != NULL);
	stage();
}


volatile usbd_setup_packet_type packet_dbg[25];
volatile uint32_t packet_dbg_cnt = 0;

static void usbd_setup_stage(void)
{
	usbd_setup_packet_type setup = {0};
	uint16_t count = USBD_PMA_GET_RX_COUNT(EP0);
	ASSERT(count == sizeof(usbd_setup_packet_type));
	usbd_pma_read(ADDR0_RX, (uint8_t*)&setup, count);

	packet_dbg[packet_dbg_cnt] = setup;

	if (packet_dbg_cnt >= 8)
	{
		__BKPT(0);
	}
	packet_dbg_cnt++;
	usbd_parse_setup_packet(setup);
}

static void usbd_data_in_stage(void)
{
	uint32_t cnt = MIN(USBD_FS_MAXPACKETSIZE, ep0_cnt);
	/*Decrement the leftover bytes.*/
	ep0_cnt -= cnt;
	
	/*If there is no leftover data, Data In stage is completed.*/
	if (!ep0_cnt)
	{
		USBD_EP_SET_KIND(EP0);
		stage = usbd_status_out_stage;
		USBD_EP_SET_RX_VALID(EP0);
		return;
	}
	/*Increment the buffer.*/
	ep0_buf += cnt;
	/*Copy a packet to usb sram*/
	USBD_PMA_SET_TX_COUNT(EP0, MIN(EP0_COUNT, ep0_cnt));
	usbd_pma_write(ep0_buf, ADDR0_TX, MIN(EP0_COUNT, ep0_cnt));
	USBD_EP_SET_TX_VALID(EP0);
}

static void usbd_data_out_stage(void)
{
	/*Get the rx count and protect from underflow.*/
	uint32_t cnt = MIN(USBD_PMA_GET_RX_COUNT(EP0), ep0_cnt);
	
	usbd_pma_read(ADDR0_TX, ep0_buf, cnt);
	USBD_EP_SET_RX_VALID(EP0);
	/*Decrement the leftover bytes.*/
	ep0_cnt -= cnt;

	/*If there is leftover data, increment the buffer pointer.*/
	if (ep0_cnt)
	{
		ep0_buf += cnt;
	}
	/*Otherwise the stage is completed.*/
	else
	{
		stage = usbd_status_in_stage;
	}
}

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

	USBD_PMA_SET_TX_COUNT(EP0, 0);
	
	/*Clear the ep0 transfer.*/
	ep0_buf = NULL;
	ep0_cnt = 0;
	stage = usbd_setup_stage;
	USBD_EP_SET_TX_VALID(EP0);
}

static void usbd_status_out_stage(void)
{
	/*Clear hardware status out*/
	USBD_EP_CLEAR_KIND(EP0);
	/*Clear the ep0 transfer.*/
	ep0_buf = NULL;
	ep0_cnt = 0;
	stage = usbd_setup_stage;
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
					ASSERT(cur_state->get_status != NULL);
                    cur_state->get_status(setup);
                    break;
                }
                case USBD_REQUEST_CLEAR_FEATURE:
                {
					ASSERT(cur_state->clear_feature != NULL);
					cur_state->clear_feature(setup);
                    break;
                }
                case USBD_REQUEST_SET_FEATURE:
                {
					ASSERT(cur_state->set_feature != NULL);
					cur_state->set_feature(setup);
                    break;            
                }
                case USBD_REQUEST_SET_ADDRESS:
                {
					ASSERT(cur_state->set_address != NULL);
					cur_state->set_address(setup);
                    break;            
                }
                case USBD_REQUEST_GET_DESCRIPTOR:
                {
					ASSERT(cur_state->get_descriptor != NULL);
					cur_state->get_descriptor(setup);
                    break;            
                }
                case USBD_REQUEST_SET_DESCRIPTOR:
                {
					ASSERT(cur_state->set_descriptor != NULL);
                    cur_state->set_descriptor(setup);
                    break;            
                }
                case USBD_REQUEST_GET_CONFIGURATION:
                {
					ASSERT(cur_state->get_configuration != NULL);
                    cur_state->get_configuration(setup);
                    break;
                }
                case USBD_REQUEST_SET_CONFIGURATION:
                {
					ASSERT(cur_state->set_configuration != NULL);
                    cur_state->set_configuration(setup);
                    break;            
                }
                case USBD_REQUEST_GET_INTERFACE:
                {
					ASSERT(cur_state->get_interface != NULL);
                    cur_state->get_interface(setup);
                    break;
                }
                case USBD_REQUEST_SET_INTERFACE:
                {
					ASSERT(cur_state->set_interface != NULL);
                    cur_state->set_interface(setup);
                    break;
                }
                case USBD_REQUEST_SYNCH_FRAME:
                {
					ASSERT(cur_state->synch_frame != NULL);
                    cur_state->synch_frame(setup);
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
			ASSERT(cur_state->class_request != NULL);
            cur_state->class_request(setup);
            break;
        }
        case USBD_REQUEST_TYPE_VENDOR:
        {
			ASSERT(cur_state->vendor_request != NULL);
			cur_state->vendor_request(setup);
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
			ASSERT(config != NULL);
			ASSERT(config->get_remote_wakeup != NULL);
			ASSERT(config->is_selfpowered != NULL);
			buf[0] = config->get_remote_wakeup() ? 1 << 1 : 0 << 1;
			buf[0] |= config->is_selfpowered() ? 1 : 0;
			break;	
		}
		case USBD_REQUEST_RECIPIENT_INTERFACE:
		{
			ASSERT(config != NULL);
			ASSERT(config->is_interface_valid != NULL);
			if(!config->is_interface_valid(setup.wIndex & 0x7FU))
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
			ASSERT(config != NULL);
			ASSERT(config->is_endpoint_valid != NULL);
			if(!config->is_endpoint_valid(ep, dir))
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
			ASSERT(config != NULL);
			ASSERT(config->set_remote_wakeup != NULL);
			config->set_remote_wakeup(false);
			break;	
		}
		case USBD_REQUEST_RECIPIENT_ENDPOINT:
		{
			uint8_t ep = (setup.wIndex & 0xF);
			uint8_t dir =  (setup.wIndex & 0x80) ? 1 : 0;
			ASSERT(config != NULL);
			ASSERT(config->is_endpoint_valid != NULL);
			if(!config->is_endpoint_valid(ep, dir))
			{
				USBD_DEV_ERR(Invalid endpoint.);
				return;
			}
			ASSERT(config->clear_stall != NULL);
			config->clear_stall(ep, dir);
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
			ASSERT(config != NULL);
			ASSERT(config->set_remote_wakeup != NULL);
			config->set_remote_wakeup(true);
			break;	
		}
		case USBD_REQUEST_RECIPIENT_ENDPOINT:
		{
			uint8_t ep = (setup.wIndex & 0xF);
			uint8_t dir =  (setup.wIndex & 0x80);
			ASSERT(config != NULL);
			ASSERT(config->is_endpoint_valid != NULL);
			if(!config->is_endpoint_valid(ep, dir))
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
	device_address = setup.wValue;
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
			ASSERT(config != NULL);
			ASSERT(config->device_descriptor != NULL);
			buf = config->device_descriptor();
			cnt = setup.wLength;
			break;
		}
		case USBD_REQUEST_DESC_CONFIGURATION:
		{
			ASSERT(config != NULL);
			ASSERT(config->configuration_descriptor != NULL);
			buf = config->configuration_descriptor(setup.wValue & 0xFFU);
			cnt = MIN(setup.wLength, (buf[2] | buf[3] << 8));
			break;
		}
		case USBD_REQUEST_DESC_STRING:
		{
			ASSERT(config != NULL);
			ASSERT(config->string_descriptor != NULL);
			buf = config->string_descriptor((setup.wValue & 0xFFU), setup.wIndex);
			cnt = MIN(setup.wLength, buf[0]);
			break;
		}		
		case USBD_REQUEST_DESC_BOS:
		{
			ASSERT(config != NULL);
			ASSERT(config->bos_descriptor != NULL);
			buf = config->bos_descriptor();
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

static void usbd_set_descriptor(usbd_setup_packet_type setup)
{
	ASSERT(config != NULL);
	ASSERT(config->set_descriptor != NULL);
	config->set_descriptor(setup);
}

static void usbd_get_configuration(usbd_setup_packet_type setup)
{
	UNUSED(setup);
	uint8_t buf = 0;
	ASSERT(config != NULL);
	ASSERT(config->get_configuration != NULL);
	buf = config->get_configuration();
	usbd_prepare_data_in_stage(&buf, 1);
}

static void usbd_set_configuration(usbd_setup_packet_type setup)
{
	__BKPT(0);
	uint8_t num = (setup.wValue & 0xFFU);
	ASSERT(config != NULL);
	ASSERT(config->is_configuration_valid != NULL);
	if(!config->is_configuration_valid(num))
	{
		USBD_DEV_ERR(Invalid configuration.);
		return;
	}
	ASSERT(config->set_configuration != NULL);
	config->set_configuration(num);
	cur_state = num ? &configured_state : &addressed_state;
	usbd_prepare_status_in_stage();
}

static void usbd_get_interface(usbd_setup_packet_type setup)
{
	uint8_t num = (setup.wIndex & 0x7FU);
	uint8_t buf = 0;
	ASSERT(config != NULL);
	ASSERT(config->is_interface_valid != NULL);
	if(!config->is_interface_valid(num))
	{
		USBD_DEV_ERR(Invalid interface);
		return;
	}
	ASSERT(config->get_interface != NULL);
	buf = config->get_interface(num);
	usbd_prepare_data_in_stage(&buf, 1);
}

static void usbd_set_interface(usbd_setup_packet_type setup)
{
	uint8_t num = (setup.wIndex & 0x7FU);
	uint8_t alt = (setup.wValue & 0xFFU);
	ASSERT(config != NULL);
	ASSERT(config->is_interface_valid != NULL);
	if (!config->is_interface_valid(num))
	{
		USBD_DEV_ERR(Invalid interface.);
	}
	ASSERT(config->set_interface != NULL);
	config->set_interface(num, alt);
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

static void usbd_class_request(usbd_setup_packet_type setup)
{
	ASSERT(config != NULL);
	ASSERT(config->class_request != NULL);
	config->class_request(setup);
}

static void usbd_vendor_request(usbd_setup_packet_type setup)
{
	ASSERT(config != NULL);
	ASSERT(config->vendor_request != NULL);
	config->vendor_request(setup);
}
#if 0
static void usbd_event_enqueue(enum usbd_event e)
{
	ASSERT(!USBD_QUEUE_IS_FULL());
	queue.buffer[queue.head] = e;
	queue.head = USBD_QUEUE_NEXT_HEAD();
}

static enum usbd_event usbd_event_dequeue(void)
{
	enum usbd_event e = NONE;
	if (!USBD_QUEUE_IS_EMPTY())
	{
		e = queue.buffer[queue.tail];
		queue.tail = USBD_QUEUE_NEXT_TAIL();
	}
	return e;
}
#endif
static void usbd_event_generator(void)
{
	//enum usbd_event e = NONE;
	uint32_t istr = USB->ISTR;

	if (GET(istr, USB_ISTR_CTR))
	{
		uint8_t ep = GET(istr, USB_EP_EA);
		uint8_t dir = GET(*USBD_EP_REG(ep), USB_EP_CTR_TX) ? 1 : 0;
		ASSERT(ep_handler[ep][dir] != NULL);

		if (dir)
		{
			USBD_EP_CLEAR_CTR_TX(ep);
		}
		else
		{
			USBD_EP_CLEAR_CTR_RX(ep);
		}
		//cur_ep_handler = ep_handler[ep][dir];
		//e = CTR;
		ep_handler[ep][dir]();
		//if (USBD_EP_GET_SETUP(ep))
		//{
			//stage = usbd_setup_stage;
		//}
	}

	if (GET(istr, USB_ISTR_RESET))
	{
		CLEAR(istr, USB_ISTR_RESET);
		usbd_reset();
		//e = RESET;
	}
	//if (packet_dbg_cnt >= 8)
	//{
		//__BKPT(0);
	//}


#if 0
	else if (GET(istr, USB_ISTR_WKUP))
	{
		CLEAR(istr, USB_ISTR_WKUP);		
		/*Restore the cur_state state.*/
		cur_state = prev_state;
		prev_state = NULL;
		ASSERT(config->wakeup != NULL);
		config->wakeup();
	}
	else if (GET(istr, USB_ISTR_SUSP))
	{
		CLEAR(istr, USB_ISTR_SUSP);
		prev_state = cur_state;
		cur_state = &suspended_state;
		ASSERT(config->suspend != NULL);
		config->suspend();
	}
	else if (GET(istr, USB_ISTR_ESOF))
	{
		CLEAR(istr, USB_ISTR_ESOF);
	}
	else if (GET(istr, USB_ISTR_SOF))
	{
		CLEAR(istr, USB_ISTR_SOF);
	}
	else if (GET(istr, USB_ISTR_PMAOVR))
	{
		CLEAR(istr, USB_ISTR_PMAOVR);
	}
	else if (GET(istr, USB_ISTR_ERR))
	{
		CLEAR(istr, USB_ISTR_ERR);		
	}
#endif
	//usbd_event_enqueue(e);
	USB->ISTR = istr;
}

static void usbd_reset(void)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		usbd_unregister_ep(i);
	}
	usbd_register_ep(EP0, USB_EP_TYPE_CONTROL, ADDR0_TX_OFFSET, ADDR0_RX_OFFSET, EP0_COUNT, usbd_ep0_handler, usbd_ep0_handler);
	ep0_buf = NULL;
	ep0_cnt = 0;
	stage = &usbd_setup_stage;
	cur_state = &default_state;
	USB->DADDR = USB_DADDR_EF;
}

#if 0
static void usbd_device_event_handler(enum usbd_event e)
{
	switch (e)
	{
		case RESET:
		{
			usbd_reset();
			break;
		}
		case SETUP:
		{
			/*Set setup stage.*/
			stage = usbd_setup_stage;
			/*fallthrough*/
			__attribute__((fallthrough));
		}
		case CTR:
		{
			ASSERT(cur_ep_handler != NULL);
			cur_ep_handler();
			break;
		}
		case SUSPEND:
		{
			/*Store the cur_state state.*/
			prev_state = cur_state;
			cur_state = &suspended_state;
			ASSERT(config->suspend != NULL);
			config->suspend();	
			break;
		}
		case WAKEUP:
		{
			/*Restore the cur_state state.*/
			cur_state = prev_state;
			prev_state = NULL;
			ASSERT(config->wakeup != NULL);
			config->wakeup();	
			break;
		}
		case NONE:
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
	ep_handler[ep][1] = ep_in;
	ep_handler[ep][0] = ep_out;
	USBD_EP_SET_CONF(ep, type, tx_addr, rx_addr, rx_count);
}

void usbd_register_ep_tx(uint8_t ep, uint32_t type, uint32_t tx_addr, void (*ep_in)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_CONTROL) || (type == USB_EP_TYPE_INTERRUPT));
	ASSERT(tx_addr < PMA_SIZE);
	ASSERT(ep_in != NULL);
	ep_handler[ep][1] = ep_in;
	USBD_EP_SET_CONF(ep, type, tx_addr, 0, 0);
}

void usbd_register_ep_rx(uint8_t ep, uint32_t type, uint32_t rx_addr, uint32_t rx_count, void (*ep_out)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_CONTROL) || (type == USB_EP_TYPE_INTERRUPT));
	ASSERT((rx_addr < PMA_SIZE) && (rx_count < USBD_PMA_COUNT));
	ASSERT(ep_out != NULL);
	ep_handler[ep][0] = ep_out;
	USBD_EP_SET_CONF(ep, type, 0, rx_addr, rx_count);
}

void usbd_register_ep_dbl_tx(uint8_t ep, uint32_t type, uint32_t tx0_addr, uint32_t tx1_addr, void (*ep_in)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_ISOCHRONOUS));
	ASSERT((tx0_addr < PMA_SIZE) && (tx1_addr < PMA_SIZE));
	ASSERT(ep_in != NULL);
	ep_handler[ep][1] = ep_in;
	USBD_EP_SET_DBL_TX_CONF(ep, type, tx0_addr, tx1_addr);
}

void usbd_register_ep_dbl_rx(uint8_t ep, uint32_t type, uint32_t rx0_addr, uint32_t rx1_addr, uint32_t rx_count, void (*ep_out)(void))
{
	ASSERT(ep < 8);
	ASSERT((type == USB_EP_TYPE_BULK) || (type == USB_EP_TYPE_ISOCHRONOUS));
	ASSERT((rx0_addr < PMA_SIZE) && (rx1_addr < PMA_SIZE) && (rx_count < USBD_PMA_COUNT));
	ASSERT(ep_out != NULL);
	ep_handler[ep][0] = ep_out;
	USBD_EP_SET_DBL_RX_CONF(ep, type, rx0_addr, rx1_addr, rx_count);	
}

void usbd_unregister_ep(uint8_t ep)
{
	ASSERT(ep < 8);
	ep_handler[ep][1] = NULL;
	ep_handler[ep][0] = NULL;
	USBD_EP_CLEAR_CONF(ep);
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
	ep0_buf = buf;
	ep0_cnt = cnt;
	stage = usbd_data_in_stage;
	USBD_PMA_SET_TX_COUNT(EP0, MIN(EP0_COUNT, ep0_cnt));
	usbd_pma_write(ep0_buf, ADDR0_TX, MIN(EP0_COUNT, ep0_cnt));
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
	ep0_buf = buf;
	ep0_cnt = cnt;
	stage = usbd_data_out_stage;
	if (rx_cplt != NULL)
	{
		reception_completed = rx_cplt;
	}
	USBD_EP_SET_RX_VALID(EP0);
}

/**
 * @brief After parsing a setup packet use this function to acknowledge that the transaction is complete,
 * without need for further data transmission or reception.
*/
void usbd_prepare_status_in_stage(void)
{
	stage = usbd_status_in_stage;
	USBD_PMA_SET_TX_COUNT(EP0, 0);
	USBD_EP_SET_TX_VALID(EP0);
}


void usbd_core_init(usbd_core_config *conf)
{
	ASSERT(conf != NULL);
	config = conf;
	cur_state = &default_state;
	stage = usbd_setup_stage;

	/*Prepare the hardware.*/
	__IO uint16_t *reg = (uint16_t*)PMA_BASE;
	CLEAR(USB->CNTR, USB_CNTR_PDWN);

	/*1 microsecond delay is needed for stm32l412 according to the datasheet.*/
	cpu_busy_wait(1);

	USB->BTABLE = 0;

	/*Clear the SRAM.*/
	for (uint16_t i = 0; i < (PMA_SIZE >> 0x1U); i++)
	{
		*reg++ = 0x0U;
	}
	/*Remove force reset.*/
	CLEAR(USB->CNTR, USB_CNTR_FRES);
	/*Enable interrupts.*/
	SET(USB->CNTR, (USB_CNTR_CTRM | USB_CNTR_RESETM));//| USB_CNTR_SUSPM | USB_CNTR_WAKEUPM
	/*Clear pending interrupts*/
	USB->ISTR = 0x0U;

	/*Enable the usb DP pullup to connect to host.*/
	SET(USB->BCDR, USB_BCDR_DPPU);
}

#if 0
void usbd_core_run(void)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	__ISB();
	__DSB();
	enum usbd_event e = usbd_event_dequeue();
	__enable_irq();
	__set_PRIMASK(primask);
	usbd_device_event_handler(e);
}
#endif

void USB_IRQHandler(void)
{
	usbd_event_generator();
}