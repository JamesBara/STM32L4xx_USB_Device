#ifndef USBD_HW_H
#define USBD_HW_H

#include "stm32l4xx.h"

#define PMA_BASE STM32L4xx_USB_SRAM_BASE
#define PMA_SIZE (0x400U)
#define EP0 0
#define EP1 1
#define EP2 2
#define EP3 3
#define EP4 4
#define EP5 5
#define EP6 6
#define EP7 7

/*Masks for the whole endpoint register per bit type.*/
#define USBD_EP_RW (USB_EP_SETUP | USB_EP_TYPE | USB_EP_KIND | USB_EP_EA)
#define USBD_EP_RC_W0 (USB_EP_CTR_RX | USB_EP_CTR_TX)
#define USBD_EP_T (USB_EP_DTOG_RX | USB_EP_STAT_RX | USB_EP_DTOG_TX | USB_EP_STAT_TX)

/*Configuration of an endpoint. To set a toggle bit set the t_flags and the
*t_masks, to clear a toggle bit set the t_masks only, to preserve the cur_state
*value don't mask the bits.*/
#define USBD_EP_CONFIGURATION(reg, type, kind, address, t_flags, t_masks) ((((reg) ^ (t_flags)) & (t_masks)) | USBD_EP_RC_W0 | (type) | (kind) | (address))

/*Set or clear the rw bits while preserving all other bits.*/
#define USBD_EP_SET_RW(reg, rw_flags) ((((reg) & ~USBD_EP_T) | USBD_EP_RC_W0) | (rw_flags))
#define USBD_EP_CLEAR_RW(reg, rw_masks) ((((reg) & ~USBD_EP_T) | USBD_EP_RC_W0) & ~(rw_masks))

/*Clear rc_w0 bits while preserving all other bits.*/
#define USBD_EP_CLEAR_RC_W0(reg, rc_w0_masks) ((((reg) & ~USBD_EP_T) | USBD_EP_RC_W0) & ~(rc_w0_masks))

/*Modify the toggle bits while preserving all other bits. To set a toggle bit set the t_flags and the t_masks, to clear a toggle bit set the t_masks only, to preserve the cur_state value don't mask the bits.*/
#define USBD_EP_SET_TOGGLE(reg, t_flags, t_masks) ((((reg) ^ (t_flags)) & (USBD_EP_RW | (t_masks))) | USBD_EP_RC_W0)

#define USBD_EP_REG(ep) ((uint16_t*) (STM32L4xx_USB_EP_BASE + ((ep) << 0x2U))) /*!< Pointer to endpoint register. */

#define USBD_PMA_BLSIZE_Pos 15U
#define USBD_PMA_BLSIZE_Msk (0x1UL << USBD_PMA_BLSIZE_Pos)
#define USBD_PMA_BLSIZE USBD_PMA_BLSIZE_Msk
#define USBD_PMA_NUM_BLOCK_Pos 10U
#define USBD_PMA_NUM_BLOCK_Msk (0x1FUL << USBD_PMA_NUM_BLOCK_Pos)
#define USBD_PMA_NUM_BLOCK USBD_PMA_NUM_BLOCK_Msk
#define USBD_PMA_COUNT (0x3FFU)

/*If x is larger than 62 divide by 32 and remove 1, then move the calculated 
 *value to USBD_PMA_NUM_BLOCK_Pos, finally OR the result with USBD_PMA_BLSIZE.
 * If x is smaller than 62 divide x by 2 and move it to USBD_PMA_NUM_BLOCK_Pos
 */
#define USBD_PMA_RX_COUNT_ALLOC(x) (((x) > 62) \
    ? (uint16_t)(USBD_PMA_BLSIZE | ((((x) >> 0x5U) - 1) << USBD_PMA_NUM_BLOCK_Pos)) \
	: (uint16_t)(((x) >> 0x1U) << USBD_PMA_NUM_BLOCK_Pos))


#define USBD_PMA_SET_TX0_ADDR(ep, addr) *(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U)) = (addr)
#define USBD_PMA_SET_TX0_COUNT(ep, count) *(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U) + 0x2U) = ((count) & USBD_PMA_COUNT)
#define USBD_PMA_SET_TX1_ADDR(ep, addr) *(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U)  + 0x4U) = (addr)
#define USBD_PMA_SET_TX1_COUNT(ep, count) *(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U) + 0x6U) = ((count) & USBD_PMA_COUNT)
#define USBD_PMA_SET_TX_ADDR(ep, addr) USBD_PMA_SET_TX0_ADDR(ep, addr)
#define USBD_PMA_SET_TX_COUNT(ep, count) USBD_PMA_SET_TX0_COUNT(ep, count)

#define USBD_PMA_SET_RX0_ADDR(ep, addr) *(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U)) = (addr)
#define USBD_PMA_SET_RX0_COUNT(ep, count) *(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U) + 0x2U) = USBD_PMA_RX_COUNT_ALLOC(count)
#define USBD_PMA_GET_RX0_COUNT(ep) ((*(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U) + 0x2U)) & USBD_PMA_COUNT)
#define USBD_PMA_SET_RX1_ADDR(ep, addr) *(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U) + 0x4U) = (addr)
#define USBD_PMA_SET_RX1_COUNT(ep, count) *(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U) + 0x6U) = USBD_PMA_RX_COUNT_ALLOC(count)
#define USBD_PMA_GET_RX1_COUNT(ep) ((*(uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U) + 0x6U)) & USBD_PMA_COUNT)
#define USBD_PMA_SET_RX_ADDR(ep, addr) USBD_PMA_SET_RX1_ADDR(ep, addr)
#define USBD_PMA_SET_RX_COUNT(ep, count) USBD_PMA_SET_RX1_COUNT(ep, count)
#define USBD_PMA_GET_RX_COUNT(ep) USBD_PMA_GET_RX1_COUNT(ep)

#define USBD_EP_CLEAR_CONF(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	USBD_PMA_SET_TX0_ADDR(ep, 0); \
	USBD_PMA_SET_TX0_COUNT(ep, 0); \
	USBD_PMA_SET_TX1_ADDR(ep, 0); \
	USBD_PMA_SET_TX1_COUNT(ep, 0); \
	ep_val = USBD_EP_CONFIGURATION(ep_val, 0, 0, ep, 0, USBD_EP_T); \
	if(GET(ep_val, USB_EP_CTR_RX)) \
	{ \
		CLEAR(ep_val, USB_EP_CTR_RX); \
	} \
	else \
	{ \
		SET(ep_val, USB_EP_CTR_RX); \
	} \
	if(GET(ep_val, USB_EP_CTR_TX)) \
	{ \
		CLEAR(ep_val, USB_EP_CTR_TX); \
	} \
	else \
	{ \
		SET(ep_val, USB_EP_CTR_TX); \
	} \
	*USBD_EP_REG(ep) = ep_val; \
}while(0)

#define USBD_EP_SET_CONF(ep, type, tx_addr, rx_addr, rx_count) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	USBD_PMA_SET_TX_ADDR(ep, tx_addr); \
	USBD_PMA_SET_RX_ADDR(ep, rx_addr); \
	USBD_PMA_SET_RX_COUNT(ep, rx_count); \
	*USBD_EP_REG(ep) = USBD_EP_CONFIGURATION(ep_val, type, 0, ep, (USB_EP_STAT_RX_VALID | USB_EP_STAT_TX_NAK), USBD_EP_T); \
}while(0)

#define USBD_EP_SET_DBL_TX_CONF(ep, type, tx0_addr, tx1_addr) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	USBD_PMA_SET_TX0_ADDR(ep, tx0_addr); \
	USBD_PMA_SET_TX1_ADDR(ep, tx1_addr); \
	*USBD_EP_REG(ep) = USBD_EP_CONFIGURATION(ep_val, type, USB_EP_KIND, ep, (USB_EP_STAT_TX_DISABLED | USB_EP_STAT_RX_DISABLED), USBD_EP_T); \
}while(0)

#define USBD_EP_SET_DBL_RX_CONF(ep, type, rx0_addr, rx1_addr, rx_count) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	USBD_PMA_SET_RX0_ADDR(ep, rx0_addr); \
	USBD_PMA_SET_RX0_COUNT(ep, rx_count); \
	USBD_PMA_SET_RX1_ADDR(ep, rx1_addr); \
	USBD_PMA_SET_RX0_COUNT(ep, rx_count); \
	*USBD_EP_REG(ep) = USBD_EP_CONFIGURATION(ep_val, type, USB_EP_KIND, ep, (USB_EP_STAT_TX_DISABLED | USB_EP_STAT_RX_DISABLED), USBD_EP_T); \
}while(0)

#define USBD_EP_SET_TX_STALL(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	if (GET(ep, USB_EP_STAT_TX) != USB_EP_STAT_TX_DISABLED) \
	*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, USB_EP_STAT_TX_STALL, USB_EP_STAT_TX); \
}while(0)

#define USBD_EP_SET_RX_STALL(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	if (GET(ep, USB_EP_STAT_RX) != USB_EP_STAT_RX_DISABLED) \
	{ \
		*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, USB_EP_STAT_RX_STALL, USB_EP_STAT_RX); \
	}\
}while(0)

#define USBD_EP_CLEAR_TX_STALL(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	if (GET(ep, USB_EP_STAT_TX) == USB_EP_STAT_TX_STALL) \
	{ \
		*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, USB_EP_STAT_TX_NAK, (USB_EP_STAT_TX | USB_EP_DTOG_TX)); \
	} \
}while(0)

#define USBD_EP_CLEAR_RX_STALL(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	if (GET(ep, USB_EP_STAT_RX) == USB_EP_STAT_RX_STALL) \
	{ \
		*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, USB_EP_STAT_RX_VALID, (USB_EP_STAT_RX | USB_EP_DTOG_RX)); \
	} \
}while(0)

#define USBD_EP_GET_STALL(ep, dir) !(dir) \
	? (GET(*USBD_EP_REG(ep), USB_EP_STAT_RX) == USB_EP_STAT_RX_STALL) \
	: (GET(*USBD_EP_REG(ep), USB_EP_STAT_TX) == USB_EP_STAT_TX_STALL)

#define USBD_EP_SET_STAT_TX(ep, flag) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, flag, USB_EP_STAT_TX); \
}while(0)

#define USBD_EP_SET_STAT_RX(ep, flag) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, flag, USB_EP_STAT_RX); \
}while(0)


#define USBD_EP_SET_KIND(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(0) = USBD_EP_SET_RW(ep_val, USB_EP_KIND); \
}while(0)

#define USBD_EP_CLEAR_KIND(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(0) = USBD_EP_CLEAR_RW(ep_val, USB_EP_KIND); \
}while(0)

#define USBD_EP_GET_KIND(ep) GET(*USBD_EP_REG(ep), USB_EP_KIND)

#define USBD_EP_CLEAR_CTR_RX(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(0) = USBD_EP_CLEAR_RC_W0(ep_val, USB_EP_CTR_RX); \
}while(0)

#define USBD_EP_CLEAR_CTR_TX(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(0) = USBD_EP_CLEAR_RC_W0(ep_val, USB_EP_CTR_TX); \
}while(0)

#define USBD_EP_GET_SETUP(ep) (*USBD_EP_REG(ep) & USB_EP_SETUP)

#define USBD_EP0_SET_STALL() do \
{ \
	uint16_t ep_val = *USBD_EP_REG(0); \
	*USBD_EP_REG(0) = USBD_EP_SET_TOGGLE(ep_val, (USB_EP_STAT_RX_STALL | USB_EP_STAT_TX_STALL), (USB_EP_STAT_RX | USB_EP_STAT_TX)); \
}while(0)

__STATIC_INLINE void usbd_dev_error(const char* file, int line, const char* func, const char* val)
{
    UNUSED(file);
    UNUSED(line);
    UNUSED(func);
    UNUSED(val);
	__BKPT(0);
}

#if defined(DEBUG) 
	#define USBD_ERROR_LOG(param) usbd_dev_error(__FILE__, __LINE__, __func__, #param)
#else
	#define USBD_ERROR_LOG(param) (void)0
#endif

#endif /*USBD_HW_H*/