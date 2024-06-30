#ifndef USBD_HW_H
#define USBD_HW_H

#include "stm32l4xx.h"
#include "usbd_hw_user_definitions.h"

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
}usbd_hw_endpoints;


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


#define PMA_BASE STM32L4xx_USB_SRAM_BASE

/*Masks for the whole endpoint register per bit type.*/
#define USBD_EP_RW (USB_EP_SETUP | USB_EP_TYPE | USB_EP_KIND | USB_EP_EA)
#define USBD_EP_RC_W0 (USB_EP_CTR_RX | USB_EP_CTR_TX)
#define USBD_EP_T (USB_EP_DTOG_RX | USB_EP_STAT_RX | USB_EP_DTOG_TX | USB_EP_STAT_TX)

/*Configuration of an endpoint. To set a toggle bit set the t_flags and the
*t_masks, to clear a toggle bit set the t_masks only, to preserve the current
*value don't mask the bits.*/
#define USBD_EP_CONFIGURATION(reg, type, kind, address, t_flags, t_masks) ((((reg) ^ (t_flags)) & (t_masks)) | USBD_EP_RC_W0 | (type) | (kind) | (address))

/*Set or clear the rw bits while preserving all other bits.*/
#define USBD_EP_SET_RW(reg, rw_flags) ((((reg) & ~USBD_EP_T) | USBD_EP_RC_W0) | (rw_flags))
#define USBD_EP_CLEAR_RW(reg, rw_masks) ((((reg) & ~USBD_EP_T) | USBD_EP_RC_W0) & ~(rw_masks))

/*Clear rc_w0 bits while preserving all other bits.*/
#define USBD_EP_CLEAR_RC_W0(reg, rc_w0_masks) ((((reg) & ~USBD_EP_T) | USBD_EP_RC_W0) & ~(rc_w0_masks))

/*Modify the toggle bits while preserving all other bits. To set a toggle bit set the t_flags and the t_masks, to clear a toggle bit set the t_masks only, to preserve the current value don't mask the bits.*/
#define USBD_EP_SET_TOGGLE(reg, t_flags, t_masks) ((((reg) ^ (t_flags)) & (USBD_EP_RW | (t_masks))) | USBD_EP_RC_W0)

#define USBD_EP_REG(ep) ((uint16_t*) (STM32L4xx_USB_EP_BASE + ((ep) << 0x2U))) /*!< Pointer to endpoint register. */

#define USBD_PMA_BLSIZE (0x8000U)
#define USBD_PMA_NUMBLOCK (0x7C00U)
#define USBD_PMA_RX_COUNT_ALLOC(x) (((x) > 0x3EU) \
    ? (uint16_t)(USBD_PMA_BLSIZE | ((((x) >> 0x5U) - 0x1U) << USBD_PMA_NUMBLOCK)) \
	: (uint16_t)(((x) >> 0x1U) << USBD_PMA_NUMBLOCK))
#define USBD_PMA_COUNT (0x3FFU)

#define USBD_PMA_SET_EP_TX0_ADDR(ep, addr) *(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U)) = (addr)
#define USBD_PMA_SET_EP_TX0_COUNT(ep, count) *(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U) + 0x2U) = ((count) & USBD_PMA_COUNT)
#define USBD_PMA_SET_EP_TX1_ADDR(ep, addr) *(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U)  + 0x4U) = (addr)
#define USBD_PMA_SET_EP_TX1_COUNT(ep, count) *(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U) + 0x6U) = ((count) & USBD_PMA_COUNT)
#define USBD_PMA_SET_EP_TX_ADDR(ep, addr) USBD_PMA_SET_EP_TX0_ADDR(ep, addr)
#define USBD_PMA_SET_EP_TX_COUNT(ep, count) USBD_PMA_SET_EP_TX0_COUNT(ep, count)

#define USBD_PMA_SET_EP_RX0_ADDR(ep, addr) *(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U)) = (addr)
#define USBD_PMA_SET_EP_RX0_COUNT(ep, count) *(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U) + 0x2U) = USBD_PMA_RX_COUNT_ALLOC(count)
#define USBD_PMA_GET_EP_RX0_COUNT(ep) ((*(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U) + 0x2U)) & USBD_PMA_COUNT)
#define USBD_PMA_SET_EP_RX1_ADDR(ep, addr) *(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U) + 0x4U) = (addr)
#define USBD_PMA_SET_EP_RX1_COUNT(ep, count) *(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U) + 0x6U) = USBD_PMA_RX_COUNT_ALLOC(count)
#define USBD_PMA_GET_EP_RX1_COUNT(ep) ((*(uint16_t*)(PMA_BASE + (USBD->btable) + ((ep) << 0x3U) + 0x6U)) & USBD_PMA_COUNT)
#define USBD_PMA_SET_EP_RX_ADDR(ep, addr) USBD_PMA_SET_EP_RX1_ADDR(ep, addr)
#define USBD_PMA_SET_EP_RX_COUNT(ep, count) USBD_PMA_SET_EP_RX1_COUNT(ep, count)
#define USBD_PMA_GET_EP_RX_COUNT(ep) USBD_PMA_GET_EP_RX1_COUNT(ep)








#endif /*USBD_HW_H*/