#include "usbd_hw.h"









#if 0
__STATIC_INLINE void usbd_register_ctr(usbd_hw_driver* me, usbd_ep_driver *ep_drv, uint8_t ep, uint8_t dir)
{
	ASSERT(me != NULL);
	ASSERT(ep_drv != NULL);
	ASSERT(ep < 8);
	uint8_t tmp_dir = !dir ? 0 : 1;
	me->correct_transfer = ep_drv->ep_handler[ep][tmp_dir];
	ASSERT(me->correct_transfer != NULL);
}

void usbd_pma_config(usbd_pma_driver *drv)
{
	drv->read = usbd_read_from_pma;
	drv->write = usbd_copy_to_pma;
}

USBD_EVENT usbd_hw_poll_istr(usbd_hw_driver* drv, usbd_ep_driver *ep_drv)
{
	uint32_t istr = USB->ISTR;

	if (GET(istr, USB_ISTR_CTR))
	{
		uint8_t ep = GET(istr, USB_EP_EA);
		uint8_t dir = GET(*USBD_EP_REG(ep), USB_EP_CTR_TX) ? 1 : 0;
		usbd_register_ctr(drv, ep_drv, ep, dir);
		if (USBD_EP_GET_SETUP(ep))
		{
			return SETUP_EVENT;
		}
		else
		{
			return CTR_EVENT;
		}
	}
	else if (GET(istr, USB_ISTR_RESET))
	{
		CLEAR(istr, USB_ISTR_RESET);
		return RESET_EVENT;
	}
	else if (GET(istr, USB_ISTR_WKUP))
	{
		CLEAR(istr, USB_ISTR_WKUP);		
		return WAKEUP_EVENT;
	}
	else if (GET(istr, USB_ISTR_SUSP))
	{
		CLEAR(istr, USB_ISTR_SUSP);
		return SUSPEND_EVENT;
	}
	else if (GET(istr, USB_ISTR_ESOF))
	{
		CLEAR(istr, USB_ISTR_ESOF);
		return ESOF_EVENT;
	}
	else if (GET(istr, USB_ISTR_SOF))
	{
		CLEAR(istr, USB_ISTR_SOF);
		return SOF_EVENT;
	}
	else if (GET(istr, USB_ISTR_PMAOVR))
	{
		CLEAR(istr, USB_ISTR_PMAOVR);
		return PMAOVR_EVENT;
	}
	else if (GET(istr, USB_ISTR_ERR))
	{
		CLEAR(istr, USB_ISTR_ERR);		
		return ERROR_EVENT;
	}

	USB->ISTR = istr;
	return NO_EVENT;
}
#endif