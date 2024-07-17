#include <stdint.h>
#include <stdbool.h>
#include "usbd_core.h"
#include "usbd_cdc_request.h"
#include "usbd_cdc.h"
#include "bsp.h"

static uint8_t *cdc_in_buffer;
static uint32_t cdc_in_cnt;
static uint8_t cdc_out_buffer[EP2_COUNT];
static uint32_t cdc_out_cnt;
static bool is_configured;

struct __PACKED usbd_cdc_configuration_descriptor
{
    usbd_std_configuration_descriptor_type std_conf_desc;
    usbd_std_interface_descriptor_type std_interface_com_desc;
    usbd_cdc_header_descriptor_type header_desc;
    usbd_cdc_call_management_descriptor_type call_management_desc;
    usbd_cdc_acm_descriptor_type acm_desc;
    usbd_cdc_union_descriptor_type union_desc;
    usbd_std_endpoint_descriptor_type std_ep_com_desc;
    usbd_std_interface_descriptor_type std_interface_data_desc;
    usbd_std_endpoint_descriptor_type std_ep_data_desc[2];
};

static const usbd_std_device_descriptor_type device_desc =
{
    USBD_LENGTH_DEVICE_DESC,
    USBD_DESC_TYPE_DEVICE,
    USBD_BCD_USB,
    USBD_CLASS_CDC,
    0x0U,
    0x0U,
    EP0_COUNT,
    USBD_VID,
    USBD_PID,
    USBD_BCD_DEVICE,
    0x0U,
    0x0U,
    0x0U,
    1
};

static const struct __PACKED usbd_cdc_configuration_descriptor conf_desc =
{
    .std_conf_desc =
    {
        USBD_LENGTH_CONFIGURATION_DESC,
        USBD_DESC_TYPE_CONFIGURATION,
        67,
        2,
        1,
        0,
        USBD_ATTRIBUTES(0,0),
        USBD_MAX_POWER(500)
    },
    .std_interface_com_desc =
    {
        USBD_LENGTH_INTERFACE_DESC,
        USBD_DESC_TYPE_INTERFACE,
        0,
        0,
        1,
        USBD_CLASS_CDC,
        USBD_CDC_ABSTRACT_CONTROL_MODEL,
        0x0U,
        0
    },
    .header_desc =
    {
        USBD_CDC_HEADER_FUNCTIONAL_DESCRIPTOR_LENGTH,
        USBD_CDC_CS_INTERFACE,
        USBD_CDC_HEADER_FUNCTIONAL_DESCRIPTOR,
        USBD_CDC_BCD
    },
    .call_management_desc = 
    {
        USBD_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR_LENGTH,
        USBD_CDC_CS_INTERFACE,
        USBD_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR,
        USBD_CDC_CALL_MANAGEMENT_CAPABILITIES(0,0),
        1
    },
    .acm_desc = 
    {
        USBD_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR_LENGTH,
        USBD_CDC_CS_INTERFACE,
        USBD_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR,
        USBD_CDC_ACM_CAPABILITIES(0,0,1,0)
    },
    .union_desc =
    {
        USBD_CDC_UNION_FUNCTIONAL_DESCRIPTOR_LENGTH,
        USBD_CDC_CS_INTERFACE,
        USBD_CDC_UNION_FUNCTIONAL_DESCRIPTOR,
        0,
        1
    },
    .std_ep_com_desc =
    {
        USBD_LENGTH_ENDPOINT_DESC,
        USBD_DESC_TYPE_ENDPOINT,
        0x81U,
        USBD_ATTRIBUTES_TRANSFER_TYPE_INTERRUPT,
        USBD_LS_MAX_PACKET_SIZE,
        0xFFU
    },
    .std_interface_data_desc =
    {
        USBD_LENGTH_INTERFACE_DESC,
        USBD_DESC_TYPE_INTERFACE,
        1,
        0,
        2,
        USBD_CLASS_CDC_DATA,
        0x0U,
        0x0U,
        0
    },
    .std_ep_data_desc =
    {
        {
            USBD_LENGTH_ENDPOINT_DESC,
            USBD_DESC_TYPE_ENDPOINT,
            0x82U,
            USBD_ATTRIBUTES_TRANSFER_TYPE_BULK,
            USBD_FS_MAX_PACKET_SIZE,
            0x0U
        },
        {
            USBD_LENGTH_ENDPOINT_DESC,
            USBD_DESC_TYPE_ENDPOINT,
            0x2U,
            USBD_ATTRIBUTES_TRANSFER_TYPE_BULK,
            USBD_FS_MAX_PACKET_SIZE,
            0x0U
        }
    }
};

static usbd_cdc_line_coding line_coding =
{
    115200,
    0,
    0,
    8
};

static uint8_t dtr;
static uint8_t rts;

static void usbd_ep1_in_handler(void);
static void usbd_ep2_in_handler(void);
static void usbd_ep2_out_handler(void);

static bool is_selfpowered(void);
static bool is_interface_valid(uint8_t num);
static bool is_endpoint_valid(uint8_t num, uint8_t dir);
static void clear_stall(uint8_t num, uint8_t dir);
static uint8_t* get_device_descriptor(void);
static uint8_t* get_configuration_descriptor(uint8_t index);
static uint8_t get_configuration(void);
static bool is_configuration_valid(uint8_t num);
static void set_configuration(uint8_t num);
static void class_request(usbd_setup_packet_type setup);
void suspend(void);
void wakeup(void);


static usbd_core_config usbd_cdc_conf =
{
    is_selfpowered,
    NULL,
    NULL,
    is_interface_valid,
    is_endpoint_valid,
    clear_stall,
    get_device_descriptor,
    get_configuration_descriptor,
    NULL,
    NULL,
    NULL,
    get_configuration,
    is_configuration_valid,
    set_configuration,
    NULL,
    NULL,
    class_request,
    NULL,
    suspend,
    wakeup
};

static bool is_selfpowered(void)
{
    return false;
}

static bool is_interface_valid(uint8_t num)
{
    switch(num)
    {
        case 1:
        {
            return true;
            break;
        }
        case 2:
        {
            return true;
            break;
        }
        default:
        {
            return false;
            break;
        }
    }
}

static bool is_endpoint_valid(uint8_t num, uint8_t dir)
{
    switch (num)
    {
        case 1:
        {
            if (dir)
            {
                return true;
            }
            else
            {
                return false;
            }
            break;
        }
        case 2:
        {
            return true;
            break;
        }
        default:
        {
            return false;
            break;
        }
    }
}

static void clear_stall(uint8_t num, uint8_t dir)
{
    if (dir)
    {
        USBD_EP_CLEAR_TX_STALL(num);
        if (num == 1)
        {
            usbd_ep1_in_handler();
        }
        if (num == 2)
        {
            usbd_ep2_in_handler();
        }
    }
    else
    {
        USBD_EP_CLEAR_RX_STALL(num);
        usbd_ep2_out_handler();
    }
}

static uint8_t* get_device_descriptor(void)
{
    return (uint8_t*)&device_desc;
}

static uint8_t* get_configuration_descriptor(uint8_t index)
{
    UNUSED(index);
    return (uint8_t*)&conf_desc;
}

static uint8_t get_configuration(void)
{
    return (is_configured ? 1 : 0);
}

static bool is_configuration_valid(uint8_t num)
{
    switch (num)
    {
        case 1:
        {
            return true;
            break;
        }
        default:
        {
            return false;
            break;
        }
    }
}

static void set_configuration(uint8_t num)
{
    switch (num)
    {
        case 1:
        {
            usbd_register_ep_tx(EP1, USB_EP_TYPE_INTERRUPT, ADDR1_TX_OFFSET, usbd_ep1_in_handler);
            usbd_register_ep(EP2, USB_EP_TYPE_BULK, ADDR2_TX_OFFSET, ADDR2_RX_OFFSET, EP2_COUNT, usbd_ep2_in_handler, usbd_ep2_out_handler);
            is_configured = true;
            break;
        }
        default:
        {
            usbd_unregister_ep(EP1);
            usbd_unregister_ep(EP2);
            is_configured = false;
            break;
        }
    }
}

static void class_request(usbd_setup_packet_type setup)
{
    switch (setup.bRequest)
    {
        case USBD_CDC_SET_LINE_CODING:
        {
            usbd_prepare_data_out_stage((uint8_t*)&line_coding, sizeof(line_coding), NULL);
            break;
        }
        case USBD_CDC_GET_LINE_CODING:
        {
            usbd_prepare_data_in_stage((uint8_t*)&line_coding, sizeof(line_coding));
            break;
        }
        case USBD_CDC_SET_CONTROL_LINE_STATE:
        {
            dtr = (setup.wValue & 0x1U);
            rts = (!(setup.wValue & 0x2U)) ? 0x0U : 0x1U;
            usbd_prepare_status_in_stage();
            break;
        }
        default:
        {
            USBD_ERROR_LOG(Invalid bRequest.);
            USBD_EP0_SET_STALL();
            return;
            break;
        }
    }
}

void suspend(void)
{
    __WFI();
}

void wakeup(void)
{
    set_cpu_max_freq();
    set_usbd_clk_src_hsi48();
}

static void usbd_ep1_in_handler(void)
{

}

static void usbd_ep2_in_handler(void)
{
    uint32_t cnt = MIN(cdc_in_cnt, USBD_FS_MAX_PACKET_SIZE);
    cdc_in_cnt -= cnt;

    if (!cdc_in_cnt)
    {
        cdc_in_buffer = NULL;
        return;
    }
    cdc_in_buffer += cnt;

    USBD_PMA_SET_TX_COUNT(EP2, MIN(cdc_in_cnt, USBD_FS_MAX_PACKET_SIZE));
    usbd_pma_write(cdc_in_buffer, ADDR2_TX, MIN(cdc_in_cnt, USBD_FS_MAX_PACKET_SIZE));
    USBD_EP_SET_STAT_TX(EP2, USB_EP_STAT_TX_VALID);
}

static void usbd_ep2_out_handler(void)
{
    cdc_out_cnt = USBD_PMA_GET_RX_COUNT(EP2);
    usbd_pma_read(ADDR2_RX, cdc_out_buffer, cdc_out_cnt);
    USBD_EP_SET_STAT_RX(EP2, USB_EP_STAT_RX_VALID);
}

usbd_core_config* usbd_cdc_init(void)
{
    return &usbd_cdc_conf;
}

bool usbd_cdc_is_configured(void)
{
    return is_configured;
}

usbd_cdc_line_coding usbd_cdc_get_line_coding(void)
{
    return line_coding;
}

bool usbd_cdc_get_rts(void)
{
    return (bool)(rts & 0x1U);
}

bool usbd_cdc_get_dtr(void)
{
    return (bool)(dtr & 0x1U);
}

void usbd_cdc_transmit(uint8_t* buf, uint32_t cnt)
{
    if (!dtr)
    {
        return;
    }
    cdc_in_buffer = buf;
    cdc_in_cnt = cnt;
    USBD_PMA_SET_TX_COUNT(EP2, MIN(cdc_in_cnt, USBD_FS_MAX_PACKET_SIZE));
    usbd_pma_write(cdc_in_buffer, ADDR2_TX, MIN(cdc_in_cnt, USBD_FS_MAX_PACKET_SIZE));
    USBD_EP_SET_STAT_TX(EP2, USB_EP_STAT_TX_VALID);
}