IOP_BIN  = ps2link.irx
IOP_OBJS = net_fsys.o net_fio.o ps2link.o cmdHandler.o nprintf.o excepHandler.o imports.o

IOP_INCS += -I./include
IOP_LIBS +=
IOP_LDFLAGS +=
IOP_CFLAGS += -Werror

# Enable zero-copy on fileio writes.
ifeq ($(ZEROCOPY),1)
IOP_CFLAGS += -DZEROCOPY
endif

# Enable debug mode
ifeq ($(DEBUG),1)
IOP_CFLAGS += -DDEBUG
endif

ifeq ($(PWOFFONRESET),1)
IOP_CFLAGS += -DPWOFFONRESET
endif


all: $(IOP_BIN)

clean:
	-rm -f $(IOP_OBJS) $(IOP_BIN)

clean_objs:
	-rm -f $(IOP_OBJS)

move_to_iso_files:
	mv $(IOP_BIN) ../iso_files/DRIVERS/$(shell echo $(IOP_BIN) | tr a-z A-Z)

include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.iopglobal
