#ifndef USBD_H
#define USBD_H






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