#ifndef USBD_H
#define USBD_H

#include "stm32l4xx.h"

typedef struct
{
	void (*state)(void);
	void (*stage)(void);

}usbd_device_driver;






typedef struct
{
	void (*get_status)(usbd_setup_packet setup);
	void (*clear_feature)(usbd_setup_packet setup);
	void (*set_feature)(usbd_setup_packet setup);
	void (*set_address)(usbd_setup_packet setup);
	void (*get_descriptor)(usbd_setup_packet setup);
	void (*set_descriptor)(usbd_setup_packet setup);
	void (*get_configuration)(usbd_setup_packet setup);
	void (*set_configuration)(usbd_setup_packet setup);
	void (*get_interface)(usbd_setup_packet setup);
	void (*set_interface)(usbd_setup_packet setup);
	void (*synch_frame)(usbd_setup_packet setup);
}__PACKED usbd_std_device_requests;

typedef struct
{
	uint8_t *(*device)(void);
	uint8_t *(*configuration)(void);
	uint8_t *(*string)(void);
	uint8_t *(*bos)(void);
}__PACKED usbd_descriptor_type;


typedef struct
{
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
}__PACKED usbd_setup_packet_type;






typedef struct
{
	void (*ep0_in)(void);
	void (*ep0_out)(void);
	void (*ep1_in)(void);
	void (*ep1_out)(void);
	void (*ep2_in)(void);
	void (*ep2_out)(void);
	void (*ep3_in)(void);
	void (*ep3_out)(void);
	void (*ep4_in)(void);
	void (*ep4_out)(void);
	void (*ep5_in)(void);
	void (*ep5_out)(void);
	void (*ep6_in)(void);
	void (*ep6_out)(void);
	void (*ep7_in)(void);
	void (*ep7_out)(void);
}usbd_endpoints;


typedef struct
{
	void (*correct_transfer)(uint8_t ep_id, uint8_t dir); /*Maybe void?*/
	void (*packet_memory_ovr)(void);
	void (*error)(void);
	void (*wakeup)(void);
	void (*suspend_mode_request)(void);
	void (*reset)(void);
	void (*start_of_frame)(void);
	void (*expected_start_of_frame)(void);
	void (*lpm_l1_state_request)(void);
}usbd_hw_driver;




#endif /*USBD_H*/