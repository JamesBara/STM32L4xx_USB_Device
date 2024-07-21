#ifndef USBD_REQUEST_H
#define USBD_REQUEST_H

#include <stdint.h>
#include "stm32l4xx.h"

/*******************************************************************************
 * Setup Packet definitions
 ******************************************************************************/

#define USBD_SETUP_PACKET_SIZE (0x8U)

/************************************************
 *	bmRequestType
 ***********************************************/
#define USBD_DIRECTION_Pos 7
#define USBD_DIRECTION_Msk (0x1U << USBD_DIRECTION_Pos)
#define USBD_DIRECTION USBD_DIRECTION_Msk
#define USBD_DIRECTION_IN (0x1U << USBD_DIRECTION_Pos)
#define USBD_DIRECTION_OUT (0x0U << USBD_DIRECTION_Pos)

#define USBD_TYPE_Pos 5
#define USBD_TYPE_Msk (0x3U << USBD_TYPE_Pos)
#define USBD_TYPE USBD_TYPE_Msk
#define USBD_TYPE_STANDARD (0x0U << USBD_TYPE_Pos)
#define USBD_TYPE_CLASS (0x1U << USBD_TYPE_Pos)
#define USBD_TYPE_VENDOR (0x2U << USBD_TYPE_Pos)
#define USBD_TYPE_RESERVED (0x3U << USBD_TYPE_Pos)

#define USBD_RECIPIENT_Pos 0
#define USBD_RECIPIENT_Msk (0x1FU << USBD_RECIPIENT_Pos)
#define USBD_RECIPIENT USBD_RECIPIENT_Msk
#define USBD_RECIPIENT_DEVICE (0x0U << USBD_RECIPIENT_Pos)
#define USBD_RECIPIENT_INTERFACE (0x1U << USBD_RECIPIENT_Pos)
#define USBD_RECIPIENT_ENDPOINT (0x2U << USBD_RECIPIENT_Pos)
#define USBD_RECIPIENT_OTHER (0x3U << USBD_RECIPIENT_Pos)

/************************************************
 *	bRequest
 ***********************************************/
#define USBD_GET_STATUS 0  
#define USBD_CLEAR_FEATURE 1
#define USBD_SET_FEATURE 3
#define USBD_SET_ADDRESS 5
#define USBD_GET_DESCRIPTOR 6
#define USBD_SET_DESCRIPTOR 7
#define USBD_GET_CONFIGURATION 8
#define USBD_SET_CONFIGURATION 9
#define USBD_GET_INTERFACE 10
#define USBD_SET_INTERFACE 11
#define USBD_SYNCH_FRAME 12

/************************************************
 *	wLength
 ***********************************************/
#define USBD_GET_STATUS_LENGTH 2
#define USBD_SET_FEATURE_LENGTH 0
#define USBD_SET_ADDRESS_LENGTH 0
#define USBD_CLEAR_FEATURE_LENGTH 0
#define USBD_SET_INTERFACE_LENGTH 0
#define USBD_GET_INTERFACE_LENGTH 1
#define USBD_SET_CONFIGURATION_LENGTH 0
#define USBD_GET_CONFIGURATION_LENGTH 1
#define USBD_SYNCH_FRAME_LENGTH 2

#define USBD_TEST_MODE 2
#define USBD_DEVICE_REMOTE_WAKEUP 1
#define USBD_ENDPOINT_HALT 0

/*******************************************************************************
 *  USBD Device Descriptors definitions
 ******************************************************************************/

/************************************************
 *	bLength
 ***********************************************/
#define USBD_LENGTH_DEVICE_DESC 18
#define USBD_LENGTH_CONFIGURATION_DESC 9
#define USBD_LENGTH_INTERFACE_DESC 9
#define USBD_LENGTH_ENDPOINT_DESC 7

/************************************************
 *	bDescriptorType
 ***********************************************/
#define USBD_DESC_TYPE_DEVICE 1
#define USBD_DESC_TYPE_CONFIGURATION 2
#define USBD_DESC_TYPE_STRING 3
#define USBD_DESC_TYPE_INTERFACE 4
#define USBD_DESC_TYPE_ENDPOINT 5
#define USBD_DESC_TYPE_BOS 15
#define USBD_DESC_TYPE_DEVICE_CAPABILITY 16

/************************************************
 *  Device bDeviceClass
 ***********************************************/
#define USBD_CLASS_NONE 0x0U
#define USBD_CLASS_AUDIO 0x1U
#define USBD_CLASS_CDC 0x2U
#define USBD_CLASS_HID 0x3U
#define USBD_CLASS_MSC 0x8U
#define USBD_CLASS_CDC_DATA 0xAU
#define USBD_CLASS_MISC 0xEFU
#define USBD_CLASS_VENDOR 0xFFU

/************************************************
 *  bcd
 ***********************************************/
#define USBD_BCD_USB20 0x0200
#define USBD_BCD_USB21 0x0201
#define USBD_BCD_DEVICE 0x0100

/************************************************
 *	bmAttributes
 ***********************************************/
#define USBD_ATTRIBUTES(selfpowered, remotewakeup) (0x80U | (((selfpowered) & 0x1U) << 6) | (((remotewakeup) & 0x1U) << 5))

#define USBD_ATTRIBUTES_TRANSFER_TYPE_Pos 0
#define USBD_ATTRIBUTES_TRANSFER_TYPE_Msk (0x3U << USBD_ATTRIBUTES_TRANSFER_TYPE_Pos)
#define USBD_ATTRIBUTES_TRANSFER_TYPE USBD_ATTRIBUTES_TRANSFER_TYPE_Msk
#define USBD_ATTRIBUTES_TRANSFER_TYPE_CONTROL (0x0U << USBD_ATTRIBUTES_TRANSFER_TYPE_Pos)
#define USBD_ATTRIBUTES_TRANSFER_TYPE_ISOCHRONOUS (0x1U << USBD_ATTRIBUTES_TRANSFER_TYPE_Pos)
#define USBD_ATTRIBUTES_TRANSFER_TYPE_BULK (0x2U << USBD_ATTRIBUTES_TRANSFER_TYPE_Pos)
#define USBD_ATTRIBUTES_TRANSFER_TYPE_INTERRUPT (0x3U << USBD_ATTRIBUTES_TRANSFER_TYPE_Pos)

#define USBD_ATTRIBUTES_SYNC_TYPE_Pos 2
#define USBD_ATTRIBUTES_SYNC_TYPE_Msk (0x3U << USBD_ATTRIBUTES_SYNC_TYPE_Pos)
#define USBD_ATTRIBUTES_SYNC_TYPE USBD_ATTRIBUTES_SYNC_TYPE_Msk
#define USBD_ATTRIBUTES_SYNC_TYPE_NO_SYNCHRONISATION (0x0U << USBD_ATTRIBUTES_SYNC_TYPE_Pos)
#define USBD_ATTRIBUTES_SYNC_TYPE_ASYNCHRONOUS (0x1U << USBD_ATTRIBUTES_SYNC_TYPE_Pos)
#define USBD_ATTRIBUTES_SYNC_TYPE_ADAPTIVE (0x2U << USBD_ATTRIBUTES_SYNC_TYPE_Pos)
#define USBD_ATTRIBUTES_SYNC_TYPE_SYNCHRONOUS (0x3U << USBD_ATTRIBUTES_SYNC_TYPE_Pos)

#define USBD_ATTRIBUTES_USAGE_TYPE_Pos 4
#define USBD_ATTRIBUTES_USAGE_TYPE_Msk (0x3U << USBD_ATTRIBUTES_USAGE_TYPE_Pos)
#define USBD_ATTRIBUTES_USAGE_TYPE USBD_ATTRIBUTES_USAGE_TYPE_Msk
#define USBD_ATTRIBUTES_USAGE_TYPE_DATA (0x0U << USBD_ATTRIBUTES_USAGE_TYPE_Pos)
#define USBD_ATTRIBUTES_USAGE_TYPE_FEEDBACK (0x1U << USBD_ATTRIBUTES_USAGE_TYPE_Pos)
#define USBD_ATTRIBUTES_USAGE_TYPE_EXPICIT_FEEDBACK_DATA (0x2U << USBD_ATTRIBUTES_USAGE_TYPE_Pos)
#define USBD_ATTRIBUTES_USAGE_TYPE_RESERVED (0x3U << USBD_ATTRIBUTES_USAGE_TYPE_Pos)

/************************************************
 * bMaxPower
 ***********************************************/
#define USBD_MAX_POWER(x) ((x) >> 0x1U)

/************************************************
 *  bEndpointAddress
 ***********************************************/
#define USBD_EP_ADDRESS_EP_NUMBER_Pos 0
#define USBD_EP_ADDRESS_EP_NUMBER_Msk (0xFU << USBD_EP_ADDRESS_EP_NUMBER_Pos)
#define USBD_EP_ADDRESS_EP_NUMBER USBD_EP_ADDRESS_EP_NUMBER_Msk

#define USBD_EP_ADDRESS_EP_DIRECTION_Pos 7
#define USBD_EP_ADDRESS_EP_DIRECTION_Msk (0x1U << USBD_EP_ADDRESS_EP_DIRECTION_Pos)
#define USBD_EP_ADDRESS_EP_DIRECTION USBD_EP_ADDRESS_EP_DIRECTION_Msk

/************************************************
 *  wMaxPacketSize
 ***********************************************/
#define USBD_LS_MAX_PACKET_SIZE 0x8U /*!< USB Low Speed Max packet size. */
#define USBD_FS_MAX_PACKET_SIZE 0x40U /*!< USB Full Speed Max packet size. */
	
/************************************************
 *  String wLANGID
 ***********************************************/
#define USBD_LANGID_ENGLISH_US 0x409U

/************************************************
 *  BOS bDevCapabilityType
 ***********************************************/
#define USBD_DEV_CAPABILITY_TYPE_PLATFORM_CAPABILITY 0x5U

/*******************************************************************************
 * USBD Descriptors structs
 ******************************************************************************/

/************************************************
 *  Standard Device Descriptor
 ***********************************************/
typedef struct
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
}__PACKED usbd_std_device_descriptor_type;

/************************************************
 *  Standard Configuration Descriptor
 ***********************************************/
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
}__PACKED usbd_std_configuration_descriptor_type;

/************************************************
 *  Standard Interface Descriptor
 ***********************************************/
typedef struct
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
}__PACKED usbd_std_interface_descriptor_type;

/************************************************
 *  Standard Endpoint Descriptor
 ***********************************************/
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
}__PACKED usbd_std_endpoint_descriptor_type;

/************************************************
 *  Standard String Descriptor
 ***********************************************/
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
}__PACKED usbd_std_string_descriptor_type;

/************************************************
 *  Standard BOS Descriptor
 ***********************************************/
typedef struct
{
    uint8_t bLength;
    uint8_t	bDescriptorType;
    uint16_t wLength;
    uint8_t bNumDeviceCaps;
}__PACKED usbd_std_bos_descriptor_type;

/************************************************
 * Standard Platform Capability Descriptor
 ***********************************************/
typedef struct
{
    uint8_t bLength;
    uint8_t	bDescriptorType;
    uint8_t bDevCapabilityType;
    uint8_t bReserved;
    uint8_t PlatformCapabilityUUID[16];
    /*CapabilityData is not included in the struct.*/
}__PACKED usbd_std_platform_capability_descriptor_type;

/************************************************
 * Microsoft OS 2.0 Descriptor Set Information 
 ***********************************************/
/*CapabilityData of the MS OS 2.0 descriptor.*/
typedef struct
{
    uint32_t dwWindowsVersion;
    uint16_t wMSOSDescriptorSetTotalLength;
    uint8_t bMS_VendorCode;
    uint8_t bAltEnumCode;
}__PACKED usbd_ms_os20_set_information_descriptor_type;

/************************************************
 *  Setup packet
 ***********************************************/
typedef struct
{
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
}__PACKED usbd_setup_packet_type;

#endif /*USBD_REQUEST_H*/