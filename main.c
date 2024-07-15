#include "usbd_core.h"
#include "usbd_cdc.h"
#include "bsp.h"



int main()
{
	bsp_init();

	usbd_core_config* usbd_conf = cdc_init();
	usbd_core_init(usbd_conf);

	while (1)
	{
		//usbd_core_run();
	}
}
