#include <stdint.h>
#include <stdbool.h>
#include "usbd_core.h"
#include "usbd_cdc.h"



static uint8_t cdc_out_buffer[EP2_COUNT];
static uint32_t cdc_out_cnt;
static bool is_configured;

struct __PACKED usbd_cdc_line_coding
{
    uint32_t dwDTERate;
    uint8_t bCharFormat;
    uint8_t bParityType;
    uint8_t bDataBits;
};



struct __PACKED usbd_cdc_device_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
};

struct __PACKED usbd_std_configuration_descriptor_type
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
};

struct __PACKED usbd_std_interface_descriptor_type
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
};

struct __PACKED usbd_std_endpoint_descriptor_type
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
};

struct __PACKED usbd_cdc_header_descriptor_type
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdCDC;
};

struct __PACKED usbd_cdc_call_management_descriptor_type
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
};

struct __PACKED usbd_cdc_acm_descriptor_type
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
};

struct __PACKED usbd_cdc_union_descriptor_type
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bControlInterface;
    uint8_t bSubordinateInterface0;
};


struct __PACKED usbd_cdc_configuration_descriptor
{
    struct usbd_std_configuration_descriptor_type std_conf_desc;
    struct usbd_std_interface_descriptor_type std_interface_com_desc;
    struct usbd_cdc_header_descriptor_type header_desc;
    struct usbd_cdc_call_management_descriptor_type call_management_desc;
    struct usbd_cdc_acm_descriptor_type acm_desc;
    struct usbd_cdc_union_descriptor_type union_desc;
    struct usbd_std_endpoint_descriptor_type std_ep_com_desc;
    struct usbd_std_interface_descriptor_type std_interface_data_desc;
    struct usbd_std_endpoint_descriptor_type std_ep_data_desc[2];
};

static const struct __PACKED usbd_cdc_device_descriptor device_desc =
{
    18,
    USBD_REQUEST_DESC_DEVICE,
    0x0200U,
    0x2U,
    0x0U,
    0x0U,
    EP0_COUNT,
    0x0483U,
    0x5740U,
    0x0100U,
    0x0U,
    0x0U,
    0x0U,
    1
};

static const struct __PACKED usbd_cdc_configuration_descriptor conf_desc =
{
    .std_conf_desc =
    {
        9,
        USBD_REQUEST_DESC_CONFIGURATION,
        0, /*@todo*/
        2,
        1,
        0,
        0x80U,
        0xFEU
    },
    .std_interface_com_desc =
    {
        9,
        USBD_REQUEST_DESC_INTERFACE,
        0,
        0,
        1,
        0x2U,
        0x2U,
        0x0U,
        0
    },
    .header_desc =
    {
        5,
        0x24U,
        0x0U,
        0x0120U
    },
    .call_management_desc = 
    {
        5,
        0x24U,
        0x1U,
        0x0U,
        0x1U
    },
    .acm_desc = 
    {
        4,
        0x24U,
        0x2U,
        0x2U
    },
    .union_desc =
    {
        5,
        0x24U,
        0x6U,
        0x0U,
        0x1U
    },
    .std_ep_com_desc =
    {
        7,
        USBD_REQUEST_DESC_ENDPOINT,
        0x81U,
        0x3U,
        8,
        0xFFU
    },
    .std_interface_data_desc =
    {
        9,
        USBD_REQUEST_DESC_INTERFACE,
        0,
        0,
        2,
        0xAU,
        0x0U,
        0x0U,
        0
    },
    .std_ep_data_desc =
    {
        {
            7,
            USBD_REQUEST_DESC_ENDPOINT,
            0x82U,
            0x2U,
            64,
            0xFFU
        },
        {
            7,
            USBD_REQUEST_DESC_ENDPOINT,
            0x2U,
            0x2U,
            64,
            0xFFU
        }
    }
};

static struct __PACKED usbd_cdc_line_coding line_coding =
{
    115200,
    0,
    0,
    8
};

static uint8_t dtr;
static uint8_t rts;

static void set_line_coding_cplt(void);
static void usbd_ep1_in_handler(void);
static void usbd_ep2_in_handler(void);
static void usbd_ep2_out_handler(void);



static bool is_selfpowered(void);
/*Remote wakeup function are not needed.*/
static bool is_interface_valid(uint8_t num);
static bool is_endpoint_valid(uint8_t num, uint8_t dir);
static void clear_stall(uint8_t num, uint8_t dir);
static uint8_t* get_device_descriptor(void);
static uint8_t* get_configuration_descriptor(uint8_t index);
/*String descriptor and bos descriptor are not needed.*/
/*Set descriptor is not needed.*/
static uint8_t get_configuration(void);
static bool is_configuration_valid(uint8_t num);
static void set_configuration(uint8_t num);
/*get and set interface are not needed.*/
static void class_request(usbd_setup_packet_type setup);
/*vendor request is not needed.*/
/*suspend and wakeup may be added later.*/

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
    NULL,
    NULL
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
            usbd_prepare_data_out_stage((uint8_t*)&line_coding, sizeof(line_coding), set_line_coding_cplt);
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
            USBD_DEV_ERR(Invalid bRequest.);
            break;
        }
    }

}

static void set_line_coding_cplt(void)
{

}

static void usbd_ep1_in_handler(void)
{

}

static void usbd_ep2_in_handler(void)
{

}

static void usbd_ep2_out_handler(void)
{
    USBD_EP_CLEAR_CTR_RX(EP2);
    cdc_out_cnt = USBD_PMA_GET_RX_COUNT(EP2);
    usbd_pma_read(ADDR2_RX, cdc_out_buffer, cdc_out_cnt);
    USBD_EP_SET_RX_VALID(EP2);
}