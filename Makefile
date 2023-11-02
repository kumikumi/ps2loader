EE_BIN = ps2loader.elf
EE_OBJS = ps2loader.o kmachine.o util.o
EE_LIBS = -ldraw -lgraph -lmath3d -lpacket -ldma
EE_CFLAGS = -Werror

all: build iso

build: target $(EE_BIN) clean_objs
	mv $(EE_BIN) target/$(EE_BIN)

clean_objs:
	rm -f $(EE_OBJS)

clean:
	rm -rf target/*

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

target:
	mkdir -p target

iso:
	mkdir -p target/ISO
	rm -rf target/ISO/*
	cp -r iso_files/* target/ISO
	cp target/$(EE_BIN) target/ISO/GODD_000.01
	mkisofs -o target/ps2loader.iso target/ISO

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
