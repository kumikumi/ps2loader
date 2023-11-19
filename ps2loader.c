#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <iopcontrol.h>
#include <libcdvd-common.h>
#include <util.h>
#include <loadfile.h>
#include "cd.h"
#include "network.h"

int main(int argc, char *argv[])
{
	sio_printf("Hello world!\n");
	SifInitRpc(0);
	int initCdResult = InitCD();
	sio_printf("Init cd result: %d\n", initCdResult);
	sio_printf("Rebooting IOP...\n");
	SifLoadFileExit();
	while (!SifIopReboot("cdrom0:\\DRIVERS\\IOPRP221.IMG;1"))
	{
		sio_printf("Failed, retrying\n");
	}
	while (!SifIopSync())
	{
		// sio_printf("Syncing...\n");
	}
	sio_printf("Done\n");
	SifInitRpc(0);
	initCdResult = InitCD();
	sio_printf("Init cd result: %d\n", initCdResult);

	#define LOAD_MODULE(MODULE) \
    if (SifLoadModule("cdrom0:\\DRIVERS\\" MODULE ".IRX;1", 0, NULL) < 0) { \
        sio_printf("loading " MODULE ".IRX failed\n"); \
        goto end; \
    }

	LOAD_MODULE("SIO2MAN");
	LOAD_MODULE("PADMAN");
	LOAD_MODULE("LIBSD");
	LOAD_MODULE("MCMAN");
	LOAD_MODULE("MCSERV");
	LOAD_MODULE("989SND");

	// Added: drivers for ethernet connectivity, ps2link
	LOAD_MODULE("POWEROFF");
	LOAD_MODULE("PS2DEV9");
	LOAD_MODULE("NETMAN");
	// Memo: loading smap may fail if no ethernet adapter is present, this is probably fine
	LOAD_MODULE("SMAP");
	LOAD_MODULE("PS2IP_NM");
	LOAD_MODULE("UDPTTY");
	LOAD_MODULE("IOPTRAP");
	LOAD_MODULE("PS2LINK");

	if (!initNetworking()) {
		goto end;
	}

end:
	for (;;)
	{
		sio_printf("Sleep\n");
		SleepThread();
	}
	return 0;
}
