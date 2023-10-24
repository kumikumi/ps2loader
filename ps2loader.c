#include <stdio.h>
#include <kernel.h>

#include "kmachine.h"

int main(int argc, char *argv[])
{
	printf("Hello world!\n");
	InitParms(argc, argv);
	for (;;)
	{
		SleepThread();
	}
	return 0;
}
