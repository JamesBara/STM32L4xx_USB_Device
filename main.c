#include "usbd_core.h"
#include "bsp.h"

static usbd_core_config usbd_conf;

int main()
{
	bsp_init();
	usbd_core_init(&usbd_conf);

	while (1)
	{
		usbd_core_run();
	}
}
