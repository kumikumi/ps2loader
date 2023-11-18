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

	if (SifLoadModule("cdrom0:\\DRIVERS\\SIO2MAN.IRX;1", 0, NULL) < 0)
	{
		sio_printf("loading SIO2MAN.IRX failed\n");
		goto end;
	}
	if (SifLoadModule("cdrom0:\\DRIVERS\\PADMAN.IRX;1", 0, NULL) < 0)
	{
		sio_printf("loading PADMAN.IRX failed\n");
		goto end;
	}
	if (SifLoadModule("cdrom0:\\DRIVERS\\LIBSD.IRX;1", 0, NULL) < 0)
	{
		sio_printf("loading LIBSD.IRX failed\n");
		goto end;
	}
	if (SifLoadModule("cdrom0:\\DRIVERS\\MCMAN.IRX;1", 0, NULL) < 0)
	{
		sio_printf("loading MCMAN.IRX failed\n");
		goto end;
	}
	if (SifLoadModule("cdrom0:\\DRIVERS\\MCSERV.IRX;1", 0, NULL) < 0)
	{
		sio_printf("loading MCSERV.IRX failed\n");
		goto end;
	}
	if (SifLoadModule("cdrom0:\\DRIVERS\\989SND.IRX;1", 0, NULL) < 0)
	{
		sio_printf("loading 989SND.IRX failed\n");
		goto end;
	}

	// Added: 3 drivers for ethernet connectivity

	if (SifLoadModule("cdrom0:\\DRIVERS\\PS2DEV9.IRX;1", 0, NULL) < 0)
	{
		sio_printf("loading PS2DEV9.IRX failed\n");
		goto end;
	}
	if (SifLoadModule("cdrom0:\\DRIVERS\\NETMAN.IRX;1", 0, NULL) < 0)
	{
		sio_printf("loading NETMAN.IRX failed\n");
		goto end;
	}
	// Memo: loading smap may fail if no ethernet adapter is present, this is probably fine
	if (SifLoadModule("cdrom0:\\DRIVERS\\SMAP.IRX;1", 0, NULL) < 0)
	{
		sio_printf("loading SMAP.IRX failed\n");
		goto end;
	}

	// Todo: make this work
	// if (SifLoadModule("cdrom0:\\DRIVERS\\PS2LINK.IRX;1", 0, NULL) < 0)
	// {
	// 	sio_printf("loading PS2LINK.IRX failed\n");
	// 	goto end;
	// }

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
