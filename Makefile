EE_BIN = ps2loader.elf
EE_OBJS = ps2loader.o kmachine.o
EE_LIBS = -ldraw -lgraph -lmath3d -lpacket -ldma

all: target $(EE_BIN) clean_objs
	mv $(EE_BIN) target/$(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

clean_objs:
	rm -f $(EE_OBJS)

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

target:
	mkdir -p target

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
