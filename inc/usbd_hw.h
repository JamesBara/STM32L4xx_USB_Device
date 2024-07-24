#ifndef USBD_HW_H
#define USBD_HW_H

#include "stm32l4xx.h"

/*******************************************************************************
 * USBD Hardware Endpoint definitions and Macros
 ******************************************************************************/

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

#define USBD_EP_RW (USB_EP_SETUP | USB_EP_TYPE | USB_EP_KIND | USB_EP_EA) /*!< Mask all read/write bits of an endpoint.*/
#define USBD_EP_RC_W0 (USB_EP_CTR_RX | USB_EP_CTR_TX) /*!< Mask all read/clear 0 bits of an endpoint.*/
#define USBD_EP_T (USB_EP_DTOG_RX | USB_EP_STAT_RX | USB_EP_DTOG_TX | USB_EP_STAT_TX) /*!< Mask all toggle bits of an endpoint.*/

/************************************************
 * @brief Hardware configuration of an endpoint.
 *
 * @note In order to set toggle bits, set both
 * the t_flags and t_masks for the necessary bits.
 * To clear toggle bits, set t_masks only.
 * To preserve the current state of the toggle bits,
 * set both t_flags and t_masks to 0, for the
 * necessary bits.
 ***********************************************/
#define USBD_EP_CONFIGURATION(reg, type, kind, address, t_flags, t_masks) ((((reg) ^ (t_flags)) & (t_masks)) | USBD_EP_RC_W0 | (type) | (kind) | (address))

/************************************************
 * @brief Set or clear read write bits of an
 * endpoint while preserving both toggle and 
 * read/clear 0 bits current values.
 ***********************************************/
#define USBD_EP_SET_RW(reg, rw_flags) ((((reg) & ~USBD_EP_T) | USBD_EP_RC_W0) | (rw_flags))
#define USBD_EP_CLEAR_RW(reg, rw_masks) ((((reg) & ~USBD_EP_T) | USBD_EP_RC_W0) & ~(rw_masks))

 /************************************************
  * @brief Clear read/clear 0 bits of an
  * endpoint while preserving both toggle and
  * read/write bits current values.
  ***********************************************/
#define USBD_EP_CLEAR_RC_W0(reg, rc_w0_masks) ((((reg) & ~USBD_EP_T) | USBD_EP_RC_W0) & ~(rc_w0_masks))

/************************************************
 * @brief Set or clear the toggle bits of an
 * endpoint while preserving both read/write bits and 
 * read/clear 0 bits current values.
 *
 * @note In order to set toggle bits, set both
 * the t_flags and t_masks for the necessary bits.
 * To clear toggle bits, set t_masks only.
 * To preserve the current state of the toggle bits,
 * set both t_flags and t_masks to 0, for the
 * necessary bits.
 ***********************************************/
#define USBD_EP_SET_TOGGLE(reg, t_flags, t_masks) ((((reg) ^ (t_flags)) & (USBD_EP_RW | (t_masks))) | USBD_EP_RC_W0)

#define USBD_EP_REG(ep) ((__IO uint16_t*) (STM32L4xx_USB_EP_BASE + ((ep) << 0x2U))) /*!< Pointer to endpoint register. */

/*******************************************************************************
 * USBD Hardware Buffer Descriptor Table and Packet Memory Area 
 ******************************************************************************/

/************************************************
 *  PMA definitions. 
 ***********************************************/
#define USBD_PMA_BLSIZE_Pos 15U
#define USBD_PMA_BLSIZE_Msk (0x1UL << USBD_PMA_BLSIZE_Pos)
#define USBD_PMA_BLSIZE USBD_PMA_BLSIZE_Msk
#define USBD_PMA_NUM_BLOCK_Pos 10U
#define USBD_PMA_NUM_BLOCK_Msk (0x1FUL << USBD_PMA_NUM_BLOCK_Pos)
#define USBD_PMA_NUM_BLOCK USBD_PMA_NUM_BLOCK_Msk
#define USBD_PMA_COUNT (0x3FFU)

/************************************************
 * @brief Calculate the the buffer overflow
 * detection and the packet length size.
 * If x is larger than 62, divide it by 32 and 
 * remove 1, move the resulting value to 
 * USBD_PMA_NUM_BLOCK_Pos, and OR the result with
 * USBD_PMA_BLSIZE. If x is smaller than 62,
 * divide it by 2 and move it to 
 * USBD_PMA_NUM_BLOCK_Pos.
 * 
 * @note x has to be multiple
 * of 2 or 32.
 ***********************************************/
#define USBD_PMA_RX_COUNT_ALLOC(x) (((uint16_t)(x) > 62) \
    ? (uint16_t)(USBD_PMA_BLSIZE | ((((uint16_t)(x) >> 0x5U) - 1) << USBD_PMA_NUM_BLOCK_Pos)) \
	: (uint16_t)(((uint16_t)(x) >> 0x1U) << USBD_PMA_NUM_BLOCK_Pos))


#define USBD_PMA_REG_HELPER(ep, offset) (__IO uint16_t*)(PMA_BASE + (USB->BTABLE) + ((ep) << 0x3U) + (offset))

/************************************************
 * @brief Create the Buffer Descriptor Table by
 * setting the IN address of each endpoint.
 * USBD_PMA_SET_TX_ADDR can be used for single 
 * buffer IN endpoints, USBD_PMA_SET_TX0_ADDR 
 * and USBD_PMA_SET_TX1_ADDR for double buffer
 * IN endpoints. USBD_PMA_SET_TX_COUNT, 
 * USBD_PMA_SET_TX0_COUNT and USBD_PMA_SET_TX1_COUNT
 * can be used at any time to set data size that
 * needs to be transmitted during the next transaction.
 * 
 * @note The address of each endpoint has to be
 * set statically, it has fit the PMA
 * and not colide with other endpoints.
 ***********************************************/
#define USBD_PMA_SET_TX0_ADDR(ep, addr) *USBD_PMA_REG_HELPER(ep, 0) = ((uint16_t)(addr))
#define USBD_PMA_SET_TX0_COUNT(ep, count) *USBD_PMA_REG_HELPER(ep, 2) = ((uint16_t)(count) & USBD_PMA_COUNT)
#define USBD_PMA_SET_TX1_ADDR(ep, addr) *USBD_PMA_REG_HELPER(ep, 4) = ((uint16_t)(addr))
#define USBD_PMA_SET_TX1_COUNT(ep, count) *USBD_PMA_REG_HELPER(ep, 6) = ((uint16_t)(count) & USBD_PMA_COUNT)
#define USBD_PMA_SET_TX_ADDR(ep, addr) USBD_PMA_SET_TX0_ADDR(ep, addr)
#define USBD_PMA_SET_TX_COUNT(ep, count) USBD_PMA_SET_TX0_COUNT(ep, count)

/************************************************
 * @brief Create the Buffer Descriptor Table by
 * setting the OUT address of each endpoint.
 * USBD_PMA_SET_RX_ADDR can be used for single 
 * buffer OUT endpoints, USBD_PMA_SET_RX0_ADDR 
 * and USBD_PMA_SET_RX1_ADDR for double buffer
 * OUT endpoints. USBD_PMA_SET_RX_COUNT, 
 * USBD_PMA_SET_RX0_COUNT and USBD_PMA_SET_RX1_COUNT
 * need to be used to set the amount of data that
 * an endpoint can receive. USBD_PMA_GET_RX_COUNT
 * USBD_PMA_GET_RX0_COUNT and USBD_PMA_GET_RX1_COUNT
 * can be used to check how many data where received
 * during the last transaction.
 * 
 * @note The address of each endpoint has to be
 * set statically, it has fit the PMA
 * and not colide with other endpoints.
 ***********************************************/
#define USBD_PMA_SET_RX0_ADDR(ep, addr) *USBD_PMA_REG_HELPER(ep, 0) = ((uint16_t)(addr))
#define USBD_PMA_SET_RX0_COUNT(ep, count) *USBD_PMA_REG_HELPER(ep, 2) = USBD_PMA_RX_COUNT_ALLOC(count)
#define USBD_PMA_GET_RX0_COUNT(ep) ((*USBD_PMA_REG_HELPER(ep, 2)) & USBD_PMA_COUNT)
#define USBD_PMA_SET_RX1_ADDR(ep, addr) *USBD_PMA_REG_HELPER(ep, 4) = ((uint16_t)(addr))
#define USBD_PMA_SET_RX1_COUNT(ep, count) *USBD_PMA_REG_HELPER(ep, 6) = USBD_PMA_RX_COUNT_ALLOC(count)
#define USBD_PMA_GET_RX1_COUNT(ep) ((*USBD_PMA_REG_HELPER(ep, 6)) & USBD_PMA_COUNT)
#define USBD_PMA_SET_RX_ADDR(ep, addr) USBD_PMA_SET_RX1_ADDR(ep, addr)
#define USBD_PMA_SET_RX_COUNT(ep, count) USBD_PMA_SET_RX1_COUNT(ep, count)
#define USBD_PMA_GET_RX_COUNT(ep) USBD_PMA_GET_RX1_COUNT(ep)

/*******************************************************************************
 * USBD Hardware Macros
 ******************************************************************************/

/************************************************
 * @brief Clear the current configuration of an 
 * endpoint. Also clears the PMA address,
 * and count.
 ***********************************************/
#define USBD_EP_CLEAR_CONF(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	USBD_PMA_SET_TX0_ADDR(ep, 0); \
	USBD_PMA_SET_TX0_COUNT(ep, 0); \
	USBD_PMA_SET_TX1_ADDR(ep, 0); \
	USBD_PMA_SET_TX1_COUNT(ep, 0); \
	ep_val = USBD_EP_CONFIGURATION(ep_val, 0, 0, ep, 0, USBD_EP_T); \
	GET(ep_val, USB_EP_CTR_RX) ? CLEAR(ep_val, USB_EP_CTR_RX) : SET(ep_val, USB_EP_CTR_RX); \
	GET(ep_val, USB_EP_CTR_TX) ? CLEAR(ep_val, USB_EP_CTR_TX) : SET(ep_val, USB_EP_CTR_TX); \
	*USBD_EP_REG(ep) = ep_val; \
}while(0)

 /************************************************
  * @brief Set a new configuration of a single
  * buffer bidirectional or unidirectional
  * endpoint.
  *
  * @note For tx_addr and rx_addr only their offset
  * inside the PMA is needed. For example if tx_addr
  * is set as 64, then the address 0x40006C40
  * will be used.
  ***********************************************/
#define USBD_EP_SET_CONF(ep, type, tx_addr, rx_addr, rx_count) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	USBD_PMA_SET_TX_ADDR(ep, tx_addr); \
	USBD_PMA_SET_RX_ADDR(ep, rx_addr); \
	USBD_PMA_SET_RX_COUNT(ep, rx_count); \
	*USBD_EP_REG(ep) = USBD_EP_CONFIGURATION(ep_val, type, 0, ep, (USB_EP_STAT_RX_VALID | USB_EP_STAT_TX_NAK), USBD_EP_T); \
}while(0)

/************************************************
* @brief Set a new configuration of an IN double
* buffer endpoint. (Bulk or Isochronous).
***********************************************/
#define USBD_EP_SET_DBL_TX_CONF(ep, type, tx0_addr, tx1_addr) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	USBD_PMA_SET_TX0_ADDR(ep, tx0_addr); \
	USBD_PMA_SET_TX1_ADDR(ep, tx1_addr); \
	*USBD_EP_REG(ep) = USBD_EP_CONFIGURATION(ep_val, type, USB_EP_KIND, ep, (USB_EP_STAT_TX_DISABLED | USB_EP_STAT_RX_DISABLED), USBD_EP_T); \
}while(0)

/************************************************
* @brief Set a new configuration of an OUT double
* buffer endpoint. (Bulk or Isochronous).
***********************************************/
#define USBD_EP_SET_DBL_RX_CONF(ep, type, rx0_addr, rx1_addr, rx_count) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	USBD_PMA_SET_RX0_ADDR(ep, rx0_addr); \
	USBD_PMA_SET_RX0_COUNT(ep, rx_count); \
	USBD_PMA_SET_RX1_ADDR(ep, rx1_addr); \
	USBD_PMA_SET_RX0_COUNT(ep, rx_count); \
	*USBD_EP_REG(ep) = USBD_EP_CONFIGURATION(ep_val, type, USB_EP_KIND, ep, (USB_EP_STAT_TX_DISABLED | USB_EP_STAT_RX_DISABLED), USBD_EP_T); \
}while(0)

/************************************************
* @brief Stall the IN direction of an endpoint.
***********************************************/
#define USBD_EP_SET_TX_STALL(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	if (GET(ep, USB_EP_STAT_TX) != USB_EP_STAT_TX_DISABLED) \
	*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, USB_EP_STAT_TX_STALL, USB_EP_STAT_TX); \
}while(0)

/************************************************
* @brief Stall the OUT direction of an endpoint.
***********************************************/
#define USBD_EP_SET_RX_STALL(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	if (GET(ep, USB_EP_STAT_RX) != USB_EP_STAT_RX_DISABLED) \
	{ \
		*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, USB_EP_STAT_RX_STALL, USB_EP_STAT_RX); \
	}\
}while(0)

/************************************************
* @brief Remove stall condition of the IN 
* direction of an endpoint.
***********************************************/
#define USBD_EP_CLEAR_TX_STALL(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	if (GET(ep, USB_EP_STAT_TX) == USB_EP_STAT_TX_STALL) \
	{ \
		*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, USB_EP_STAT_TX_NAK, (USB_EP_STAT_TX | USB_EP_DTOG_TX)); \
	} \
}while(0)

/************************************************
* @brief Remove stall condition of the OUT
* direction of an endpoint.
***********************************************/
#define USBD_EP_CLEAR_RX_STALL(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	if (GET(ep, USB_EP_STAT_RX) == USB_EP_STAT_RX_STALL) \
	{ \
		*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, USB_EP_STAT_RX_VALID, (USB_EP_STAT_RX | USB_EP_DTOG_RX)); \
	} \
}while(0)

/************************************************
* @brief Get the stall condition of the selected
* direction of an endpoint.
***********************************************/
#define USBD_EP_GET_STALL(ep, dir) !(dir) \
	? (GET(*USBD_EP_REG(ep), USB_EP_STAT_RX) == USB_EP_STAT_RX_STALL) \
	: (GET(*USBD_EP_REG(ep), USB_EP_STAT_TX) == USB_EP_STAT_TX_STALL)

/************************************************
* @brief Set the status of an IN endpoint.
***********************************************/
#define USBD_EP_SET_STAT_TX(ep, flag) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, flag, USB_EP_STAT_TX); \
}while(0)

/************************************************
* @brief Set the status of an OUT endpoint.
***********************************************/
#define USBD_EP_SET_STAT_RX(ep, flag) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(ep) = USBD_EP_SET_TOGGLE(ep_val, flag, USB_EP_STAT_RX); \
}while(0)

/************************************************
* @brief Set the kind bit of an endpoint. This is
* only useful for control transactions, or if
* you want to change a BULK endpoint from single 
* to a double buffer. 
***********************************************/
#define USBD_EP_SET_KIND(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(ep) = USBD_EP_SET_RW(ep_val, USB_EP_KIND); \
}while(0)

/************************************************
* @brief Clear the kind bit of an endpoint. This
* is only useful for control transactions, or if
* you want to change a BULK endpoint from double
* to a single buffer.
***********************************************/
#define USBD_EP_CLEAR_KIND(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(ep) = USBD_EP_CLEAR_RW(ep_val, USB_EP_KIND); \
}while(0)

/************************************************
* @brief Get the kind bit value of an endpoint.
***********************************************/
#define USBD_EP_GET_KIND(ep) GET(*USBD_EP_REG(ep), USB_EP_KIND)

/************************************************
* @brief Clear an OUT direction interrupt flag 
* of an endpoint.
***********************************************/
#define USBD_EP_CLEAR_CTR_RX(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(ep) = USBD_EP_CLEAR_RC_W0(ep_val, USB_EP_CTR_RX); \
}while(0)

/************************************************
* @brief Clear an IN direction interrupt flag
* of an endpoint.
***********************************************/
#define USBD_EP_CLEAR_CTR_TX(ep) do \
{ \
	uint16_t ep_val = *USBD_EP_REG(ep); \
	*USBD_EP_REG(ep) = USBD_EP_CLEAR_RC_W0(ep_val, USB_EP_CTR_TX); \
}while(0)

/************************************************
* @brief Get the setup bit value of an endpoint.
***********************************************/
#define USBD_EP_GET_SETUP(ep) (*USBD_EP_REG(ep) & USB_EP_SETUP)

/************************************************
* @brief Notify the host for a device error
* condition. Only used for endpoint 0.
***********************************************/
#define USBD_EP0_SET_STALL() do \
{ \
	uint16_t ep_val = *USBD_EP_REG(EP0); \
	*USBD_EP_REG(EP0) = USBD_EP_SET_TOGGLE(ep_val, (USB_EP_STAT_RX_STALL | USB_EP_STAT_TX_STALL), (USB_EP_STAT_RX | USB_EP_STAT_TX)); \
}while(0)

#endif /*USBD_HW_H*/