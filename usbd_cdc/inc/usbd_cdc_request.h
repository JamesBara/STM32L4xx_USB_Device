#ifndef USBD_CDC_REQUEST_H
#define USBD_CDC_REQUEST_H

#include <stdint.h>
#include "stm32l4xx.h"

/*******************************************************************************
 * USBD CDC definitions
 ******************************************************************************/

 /************************************************
  *	bDeviceSubClass
  ***********************************************/
#define USBD_CDC_ABSTRACT_CONTROL_MODEL 0x2U

/************************************************
 *	bLength
 ***********************************************/
#define USBD_CDC_HEADER_FUNCTIONAL_DESCRIPTOR_LENGTH 5
#define USBD_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR_LENGTH 5
#define USBD_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR_LENGTH 4
#define USBD_CDC_UNION_FUNCTIONAL_DESCRIPTOR_LENGTH 5

/************************************************
 *	bDescriptorType
 ***********************************************/
#define USBD_CDC_CS_INTERFACE 0x24U
#define USBD_CDC_CS_ENDPOINT 0x25U

/************************************************
 *	bDescriptorSubtype
 ***********************************************/
#define USBD_CDC_HEADER_FUNCTIONAL_DESCRIPTOR 0
#define USBD_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR 1
#define USBD_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR 2
#define USBD_CDC_UNION_FUNCTIONAL_DESCRIPTOR 6



/************************************************
 *	bcdCDC
 ***********************************************/
#define USBD_CDC_BCD 0x0120U

/************************************************
 *	bmCapabilities
 ***********************************************/

#define USBD_CDC_CALL_MANAGEMENT_CAPABILITIES(cminfo_over_data_class, device_handled_cm) \
                                 ((((cminfo_over_data_class) & 0x1U) << 1) | (((device_handled_cm) & 0x1U) << 0))

#define USBD_CDC_ACM_CAPABILITIES(network_connection, send_break, line_coding_serial_state, comm_feature) \
                                 ((((network_connection) & 0x1U) << 3) | (((send_break) & 0x1U) << 2) | \
                                 (((line_coding_serial_state) & 0x1U) << 1) | (((comm_feature) & 0x1U) << 0))

/************************************************
 *	bRequest
 ***********************************************/
#define USBD_CDC_SET_LINE_CODING 0x20U
#define USBD_CDC_GET_LINE_CODING 0x21U
#define USBD_CDC_SET_CONTROL_LINE_STATE 0x22U



/*******************************************************************************
 * CDC Descriptors structs
 ******************************************************************************/

typedef struct 
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdCDC;
}__PACKED usbd_cdc_header_descriptor_type;

typedef struct 
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
}__PACKED usbd_cdc_call_management_descriptor_type;

typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
}__PACKED usbd_cdc_acm_descriptor_type;

typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bControlInterface;
    uint8_t bSubordinateInterface0;
}__PACKED usbd_cdc_union_descriptor_type;

/************************************************
 *  Line coding
 ***********************************************/

typedef struct
{
    uint32_t dwDTERate;
    uint8_t bCharFormat;
    uint8_t bParityType;
    uint8_t bDataBits;
}__PACKED usbd_cdc_line_coding;





#endif /*USBD_CDC_REQUEST_H*/