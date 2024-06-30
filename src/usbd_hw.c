#include "usbd.h"

void usbd_copy_to_pma(uint8_t* src, uint16_t* dst, uint16_t sz)
{
	uint16_t half_cnt, tmp_val;
	half_cnt = (sz >> 0x1U);

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
	if (sz & 0x1U)
	{
		tmp_val = (uint16_t)(*src);
		*dst = tmp_val;
	}
}

void usbd_read_from_pma(uint16_t* src, uint8_t* dst, uint16_t sz)
{
	uint16_t half_cnt, tmp_val;
	half_cnt = (sz >> 0x1U);

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
	if (sz & 0x1U)
	{
		tmp_val = *src;
		*dst = (uint8_t)(tmp_val & 0xFFU);
	}
}







/*Hardcoding everything that has to do with endpoints.*/



#define USBD_FS_MAXPACKETSIZE (0x40U)





















#define USBD_ENDPOINT_0_TX_PMA_BUF (uint16_t*)(USBD_SRAM_BASE_ADDR + USBD_ENDPOINT_0_IN_ADDR_OFFSET) /*!< Pointer to endpoint 0 in PMA buffer.*/
#define USBD_ENDPOINT_0_RX_PMA_BUF (uint16_t*)(USBD_SRAM_BASE_ADDR + USBD_ENDPOINT_0_OUT_ADDR_OFFSET) /*!< Pointer to endpoint 0 out PMA buffer.*/

#define USBD_ENDPOINT_1_TX_PMA_BUF  (uint16_t*)(USBD_SRAM_BASE_ADDR + USBD_ENDPOINT_1_IN_ADDR_OFFSET)/*!< Pointer to endpoint 1 in PMA buffer.*/
#define USBD_ENDPOINT_1_RX_PMA_BUF (uint16_t*)(USBD_SRAM_BASE_ADDR + USBD_ENDPOINT_1_OUT_ADDR_OFFSET) /*!< Pointer to endpoint 1 out PMA buffer.*/


/*Hardcoded pma for endpoint 0*/
#define USBD_ENDPOINT_0_CLEAR_TX_ADDR() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable)) = 0x0U; \
}while(0)

#define USBD_ENDPOINT_0_CLEAR_TX_COUNT() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 2) = 0x0U; \
}while(0)

#define USBD_ENDPOINT_0_CLEAR_RX_ADDR() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 4) = 0x0U; \
}while(0)

#define USBD_ENDPOINT_0_CLEAR_RX_COUNT() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 6) = 0x0U; \
}while(0)

#define USBD_ENDPOINT_0_SET_TX_ADDR() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable)) = (uint16_t)USBD_ENDPOINT_0_IN_ADDR_OFFSET; \
}while(0)

#define USBD_ENDPOINT_0_SET_TX_COUNT(count) do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 2) = (uint16_t)(count); \
}while(0)

#define USBD_ENDPOINT_0_SET_RX_ADDR() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 4) = (uint16_t)USBD_ENDPOINT_0_OUT_ADDR_OFFSET; \
}while(0)

#define USBD_ENDPOINT_0_SET_RX_COUNT() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 6) = USBD_SRAM_RX_COUNT_ALLOC(USBD_FS_MAXPACKETSIZE); \
}while(0)

#define USBD_ENDPOINT_0_GET_RX_COUNT() (uint16_t)((*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 6)) & USBD_SRAM_COUNT_MASK)

/*Hardcoded pma for endpoint 1*/

#define USBD_ENDPOINT_1_CLEAR_TX_ADDR() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 8) = 0x0U; \
}while(0)


#define USBD_ENDPOINT_1_CLEAR_TX_COUNT() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 10) = 0x0U; \
}while(0)

#define USBD_ENDPOINT_1_CLEAR_RX_ADDR() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 12) = 0x0U; \
}while(0)

#define USBD_ENDPOINT_1_CLEAR_RX_COUNT() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 14) = 0x0U; \
}while(0)

#define USBD_ENDPOINT_1_SET_TX_ADDR() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 8) = (uint16_t)(USBD_ENDPOINT_1_IN_ADDR_OFFSET); \
}while(0)

#define USBD_ENDPOINT_1_SET_TX_COUNT(count) do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 10) = (uint16_t)(count); \
}while(0)

#define USBD_ENDPOINT_1_SET_RX_ADDR() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 12) = (uint16_t)(USBD_ENDPOINT_1_OUT_ADDR_OFFSET); \
}while(0)

#define USBD_ENDPOINT_1_SET_RX_COUNT() do \
{ \
	*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 14) = USBD_SRAM_RX_COUNT_ALLOC(USBD_FS_MAXPACKETSIZE); \
}while(0)

#define USBD_ENDPOINT_1_GET_RX_COUNT() (uint16_t)((*(uint16_t*)(USBD_SRAM_BASE_ADDR + (USBD->btable) + 14)) & USBD_SRAM_COUNT_MASK)

/*Hardcoded configurations of endpoints.*/
#define USBD_CLEAR_ENDPOINT_0_CONFIGURATION() do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_0_REG; \
	USBD_ENDPOINT_0_CLEAR_TX_ADDR(); \
	USBD_ENDPOINT_0_CLEAR_TX_COUNT(); \
	USBD_ENDPOINT_0_CLEAR_RX_ADDR(); \
	USBD_ENDPOINT_0_CLEAR_RX_COUNT(); \
	ep_val = USBD_EP_CONFIGURATION(ep_val, 0x0U, 0x0U, USBD_ENDPOINT_0, 0x0U, USBD_EP_T_MASK); \
	(!_MACRO_GET_BIT(ep_val, USBD_EP_CTR_RX_POS)) ? (_MACRO_SET_BIT(ep_val, USBD_EP_CTR_RX_POS)) : (_MACRO_CLEAR_BIT(ep_val, USBD_EP_CTR_RX_POS)); \
	(!_MACRO_GET_BIT(ep_val, USBD_EP_CTR_TX_POS)) ? (_MACRO_SET_BIT(ep_val, USBD_EP_CTR_TX_POS)) : (_MACRO_CLEAR_BIT(ep_val, USBD_EP_CTR_TX_POS)); \
	 *USBD_ENDPOINT_0_REG = ep_val; \
}while(0)

#define USBD_CLEAR_ENDPOINT_1_CONFIGURATION() do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_1_REG; \
	USBD_ENDPOINT_1_CLEAR_TX_ADDR(); \
	USBD_ENDPOINT_1_CLEAR_TX_COUNT(); \
	USBD_ENDPOINT_1_CLEAR_RX_ADDR(); \
	USBD_ENDPOINT_1_CLEAR_RX_COUNT(); \
	ep_val = USBD_EP_CONFIGURATION(ep_val, 0x0U, 0x0U, USBD_ENDPOINT_1, 0x0U, USBD_EP_T_MASK); \
	(!_MACRO_GET_BIT(ep_val, USBD_EP_CTR_RX_POS)) ? (_MACRO_SET_BIT(ep_val, USBD_EP_CTR_RX_POS)) : (_MACRO_CLEAR_BIT(ep_val, USBD_EP_CTR_RX_POS)); \
	(!_MACRO_GET_BIT(ep_val, USBD_EP_CTR_TX_POS)) ? (_MACRO_SET_BIT(ep_val, USBD_EP_CTR_TX_POS)) : (_MACRO_CLEAR_BIT(ep_val, USBD_EP_CTR_TX_POS)); \
	 *USBD_ENDPOINT_1_REG = ep_val; \
}while(0)

#define USBD_SET_ENDPOINT_0_CONFIGURATION() do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_0_REG; \
	USBD_ENDPOINT_0_SET_TX_ADDR(); \
	USBD_ENDPOINT_0_SET_RX_ADDR(); \
	USBD_ENDPOINT_0_SET_RX_COUNT(); \
	*USBD_ENDPOINT_0_REG = USBD_EP_CONFIGURATION(ep_val, USBD_HW_EP_TYPE_CONTROL, 0x0U, USBD_ENDPOINT_0, (USBD_EP_STAT_RX_VALID | USBD_EP_STAT_TX_NAK), USBD_EP_T_MASK); \
}while(0)

#define USBD_SET_ENDPOINT_1_CONFIGURATION() do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_1_REG; \
	USBD_ENDPOINT_1_SET_TX_ADDR(); \
	USBD_ENDPOINT_1_SET_RX_ADDR(); \
	USBD_ENDPOINT_1_SET_RX_COUNT(); \
	*USBD_ENDPOINT_1_REG = USBD_EP_CONFIGURATION(ep_val, USBD_HW_EP_TYPE_BULK, 0x0U, USBD_ENDPOINT_1, (USBD_EP_STAT_RX_VALID | USBD_EP_STAT_TX_NAK), USBD_EP_T_MASK); \
}while(0)

/*Generic commands.*/

#define USBD_ENDPOINT_SET_TX_STALL(ep_num) do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_N_REG(ep_num); \
	if (_MACRO_GET_BIT_VAL(ep_val, USBD_EP_STAT_MASK, USBD_EP_STAT_TX_POS) != USBD_EP_STAT_DISABLED) \
	*USBD_ENDPOINT_N_REG(ep_num) = USBD_EP_SET_TOGGLE(ep_val, USBD_EP_STAT_TX_STALL, USBD_EP_STAT_TX_MASK); \
}while(0)

#define USBD_ENDPOINT_SET_RX_STALL(ep_num) do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_N_REG(ep_num); \
	if (_MACRO_GET_BIT_VAL(ep_val, USBD_EP_STAT_MASK, USBD_EP_STAT_RX_POS) != USBD_EP_STAT_DISABLED) \
	*USBD_ENDPOINT_N_REG(ep_num) = USBD_EP_SET_TOGGLE(ep_val, USBD_EP_STAT_RX_STALL, USBD_EP_STAT_RX_MASK); \
}while(0)

#define USBD_ENDPOINT_CLEAR_TX_STALL(ep_num) do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_N_REG(ep_num); \
	if (_MACRO_GET_BIT_VAL(ep_val, USBD_EP_STAT_MASK, USBD_EP_STAT_TX_POS) == USBD_EP_STAT_STALL) \
	*USBD_ENDPOINT_N_REG(ep_num) = USBD_EP_SET_TOGGLE(ep_val, USBD_EP_STAT_TX_NAK, (USBD_EP_STAT_TX_MASK | USBD_EP_DTOG_TX_MASK)); \
}while(0)

#define USBD_ENDPOINT_CLEAR_RX_STALL(ep_num) do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_N_REG(ep_num); \
	if (_MACRO_GET_BIT_VAL(ep_val, USBD_EP_STAT_MASK, USBD_EP_STAT_RX_POS) == USBD_EP_STAT_STALL) \
	*USBD_ENDPOINT_N_REG(ep_num) = USBD_EP_SET_TOGGLE(ep_val, USBD_EP_STAT_RX_VALID, (USBD_EP_STAT_RX_MASK | USBD_EP_DTOG_RX_MASK)); \
}while(0)

#define USBD_ENDPOINT_GET_STALL(ep_num, dir) !(dir) \
	? (_MACRO_GET_BIT_VAL(*USBD_ENDPOINT_N_REG(ep_num), USBD_EP_STAT_MASK, USBD_EP_STAT_RX_POS) != USBD_EP_STAT_STALL) ? 0x0U : 0x1U \
	: (_MACRO_GET_BIT_VAL(*USBD_ENDPOINT_N_REG(ep_num), USBD_EP_STAT_MASK, USBD_EP_STAT_TX_POS) != USBD_EP_STAT_STALL) ? 0x0U : 0x1U


#define USBD_ENDPOINT_SET_TX_VALID(ep_num) do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_N_REG(ep_num); \
	*USBD_ENDPOINT_N_REG(ep_num) = USBD_EP_SET_TOGGLE(ep_val, USBD_EP_STAT_TX_VALID, USBD_EP_STAT_TX_MASK); \
}while(0)

#define USBD_ENDPOINT_SET_RX_VALID(ep_num) do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_N_REG(ep_num); \
	*USBD_ENDPOINT_N_REG(ep_num) = USBD_EP_SET_TOGGLE(ep_val, USBD_EP_STAT_RX_VALID, USBD_EP_STAT_RX_MASK); \
}while(0)

#define USBD_ENDPOINT_GET_KIND(ep_num) GET_BIT(*USBD_ENDPOINT_N_REG(ep_num), USBD_EP_KIND_MASK)

/*Hardcoded endpoint 0 commands.*/

#define USBD_ENDPOINT_0_SET_STALL() do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_0_REG; \
	*USBD_ENDPOINT_0_REG = USBD_EP_SET_TOGGLE(ep_val, (USBD_EP_STAT_RX_STALL | USBD_EP_STAT_TX_STALL), (USBD_EP_STAT_RX_MASK | USBD_EP_STAT_TX_MASK)); \
}while(0)

#define USBD_ENDPOINT_0_TRANSMIT_ZLP() do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_0_REG; \
	USBD_ENDPOINT_0_SET_TX_COUNT(0); \
	*USBD_ENDPOINT_0_REG = USBD_EP_SET_TOGGLE(ep_val, USBD_EP_STAT_TX_VALID, USBD_EP_STAT_TX_MASK); \
}while(0)

#define USBD_ENDPOINT_0_SET_KIND() do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_0_REG; \
	*USBD_ENDPOINT_0_REG = USBD_EP_SET_RW(ep_val, USBD_EP_STATUS_OUT_MASK); \
}while(0)

#define USBD_ENDPOINT_0_CLEAR_KIND() do \
{ \
	uint16_t ep_val = *USBD_ENDPOINT_0_REG; \
	*USBD_ENDPOINT_0_REG = USBD_EP_CLEAR_RW(ep_val, USBD_EP_STATUS_OUT_MASK); \
}while(0)