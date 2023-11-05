#include <kernel.h>
#include <libcdvd-common.h>
#include <libcdvd-rpc.h>
#include <sifrpc.h>
#include "util.h"

#define CD_SERVER_SCMD 0x80000593

enum CD_SCMD_CMDS {
    CD_SCMD_READCLOCK = 0x01,
    CD_SCMD_WRITECLOCK,
    CD_SCMD_GETDISKTYPE,
    CD_SCMD_GETERROR,
    CD_SCMD_TRAYREQ,
    CD_SCMD_READ_ILINK_ID,
    CD_SCMD_WRITE_ILINK_ID,
    CD_SCMD_READ_NVM,
    CD_SCMD_WRITE_NVM,
    CD_SCMD_DEC_SET,
    CD_SCMD_SCMD,
    CD_SCMD_STATUS,
    CD_SCMD_SET_HD_MODE,
    CD_SCMD_OPEN_CONFIG,
    CD_SCMD_CLOSE_CONFIG,
    CD_SCMD_READ_CONFIG,
    CD_SCMD_WRITE_CONFIG,
    CD_SCMD_READ_CONSOLE_ID,
    CD_SCMD_WRITE_CONSOLE_ID,
    CD_SCMD_READ_MECHACON_VERSION,
    CD_SCMD_CTRL_AD_OUT,
    CD_SCMD_BREAK,
    CD_SCMD_READ_SUBQ,
    CD_SCMD_FORBID_DVDP,
    CD_SCMD_AUTO_ADJUST_CTRL,
    CD_SCMD_READ_MODEL_NAME,
    CD_SCMD_WRITE_MODEL_NAME,
    CD_SCMD_FORBID_READ,
    CD_SCMD_SPIN_CTRL,
    CD_SCMD_BOOT_CERTIFY,
    CD_SCMD_CANCELPOWEROFF,
    CD_SCMD_BLUELEDCTRL,
    CD_SCMD_POWEROFF,
    CD_SCMD_MMODE,
    CD_SCMD_SETTHREADPRI,
};

typedef union
{
    s32 s32arg;
    u32 u32arg;
    u8 bcertify[4];
    sceCdCLOCK clock;
    struct cdvdScmdParam scmd;
    struct cdvdDecSetParam decSet;
    struct cdvdReadWriteNvmParam nvm;
    u8 id[8];
    char mname[16];
    u8 data[0x420];
} sCmdSendParams_t;

extern volatile int cbSema;
extern int CdDebug;
extern int callbackSemaId;
extern int nCmdSemaId;
extern int sCmdNum;
extern int CdThreadId;
extern ee_thread_status_t CdThreadParam;
extern int bindSCmd;
extern SifRpcClientData_t clientSCmd;
extern int sCmdSemaId;
extern u8 sCmdRecvBuff[];
extern sCmdSendParams_t sCmdSendBuff;

int _CdSyncS(int mode)
{
    if (mode == 0) {
        if (CdDebug > 0)
            sio_printf("S cmd wait\n");
        while (SifCheckStatRpc(&clientSCmd))
            ;
        return 0;
    }

    return SifCheckStatRpc(&clientSCmd);
}

void _CdSemaInit(void)
{
    struct t_ee_sema semaParam;

    // return if both semaphores are already inited
    if (nCmdSemaId != -1 && sCmdSemaId != -1)
        return;

    semaParam.init_count = 1;
    semaParam.max_count  = 1;
    semaParam.option     = 0;
    nCmdSemaId           = CreateSema(&semaParam);
    sCmdSemaId           = CreateSema(&semaParam);

    semaParam.init_count = 0;
    callbackSemaId       = CreateSema(&semaParam);

    cbSema = 0;
}

int _CdCheckSCmd(int cur_cmd)
{
    _CdSemaInit();
    if (PollSema(sCmdSemaId) != sCmdSemaId) {
        if (CdDebug > 0)
            sio_printf("Scmd fail sema cur_cmd:%d keep_cmd:%d\n", cur_cmd, sCmdNum);
        return 0;
    }
    sCmdNum = cur_cmd;
    ReferThreadStatus(CdThreadId, &CdThreadParam);
    if (_CdSyncS(1)) {
        SignalSema(sCmdSemaId);
        return 0;
    }

    SifInitRpc(0);
    if (bindSCmd >= 0)
        return 1;
    while (1) {
        if (SifBindRpc(&clientSCmd, CD_SERVER_SCMD, 0) < 0) {
            if (CdDebug > 0)
                sio_printf("Libcdvd bind err S cmd\n");
        }
        if (clientSCmd.server != 0)
            break;

        nopdelay();
    }

    bindSCmd = 0;
    return 1;
}

int sceCdMmode(int media)
{
    int result;

    if (_CdCheckSCmd(CD_SCMD_MMODE) == 0)
        return 0;

    sCmdSendBuff.s32arg = media;
    if (SifCallRpc(&clientSCmd, CD_SCMD_MMODE, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 4, NULL, NULL) < 0) {
        SignalSema(sCmdSemaId);
        return 0;
    }

    result = *(int *)UNCACHED_SEG(sCmdRecvBuff);

    SignalSema(sCmdSemaId);
    return result;
}

int InitCD(void)
{
	int result;

	sio_printf("Initializing CD drive\nThis may take a while ...\n");
	sceCdInit(0);
	int mmodeResult = sceCdMmode(1); // 1 = cd, 2 = dvd
	sio_printf("Mmode result %d\n", mmodeResult);
	while (1)
	{
		result = sceCdDiskReady(0);
		if (result != SCECdNotReady)
			break;
		sio_printf("Drive not ready ... insert a disk!\n");
	}
	int diskType = sceCdGetDiskType();
	result = sio_printf("Disk type %d\n", diskType);
	return result;
}