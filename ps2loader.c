#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <iopcontrol.h>
#include <libcdvd-common.h>
#include <util.h>

int InitCD(void){
  int result;
  
  printf("Initializing CD drive\nThis may take a while ...\n");
  sceCdInit(0);
  //sceCdMmode(2);
  while( 1 ) {
    result = sceCdDiskReady(0);
    if (result != SCECdNotReady) break;
    printf("Drive not ready ... insert a disk!\n");
  }
  int diskType = sceCdGetDiskType();
  result = printf("Disk type %d\n", diskType);
  return result;
}

int main(int argc, char *argv[])
{
	printf("Hello world!\n");
	SifInitRpc(0);
	int initCDResult = InitCD();
	printf("Init cd result: %d\n", initCDResult);
	printf("Rebooting IOP...\n");
	while (!SifIopReboot("cdrom0:\\DRIVERS\\IOPRP221.IMG;1")) {
		sio_printf("Failed, retrying\n");
	}
	while (!SifIopSync()) {
		sio_printf("Syncing...\n");
	}
	sio_printf("Done\n");
	for (;;)
	{
		sio_printf("Sleep\n");
		SleepThread();
	}
	return 0;
}
