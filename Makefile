EE_BIN = ps2loader.elf
EE_OBJS = ps2loader.o kmachine.o util.o cd.o network.o
EE_LIBS = -lnetman -lps2ip -lpacket -ldma
EE_CFLAGS = -Werror -Wno-strict-aliasing

all: build iso

build: target $(EE_BIN) iop clean_objs
	mv $(EE_BIN) target/$(EE_BIN)
	$(MAKE) -C iop move_to_iso_files

clean_objs:
	rm -f $(EE_OBJS)
	$(MAKE) -C iop clean_objs

clean:
	rm -rf target/*
	$(MAKE) -C iop clean
	@rm -f ee/*_irx.o bin/*.ELF bin/*.IRX

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

iop:
	$(MAKE) -C iop

.PHONY: iop

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
