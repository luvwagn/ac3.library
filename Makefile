# ac3.library and ac3fpu.library: A/52 (Dolby Digital, AC-3) audio decoded
# to 16-bit PCM by liba52 0.8.0 (third_party/liba52, GPL-2.0-or-later),
# for any Amiga program, as mpega.library decodes MPEG audio.
# GPL-3.0-or-later, (c) 2026 Brian King; the interface (include/,
# ac3_lib.sfd, ac3.doc) is MIT.
#
#   make            build/ac3.library (liba52's integer build: 68020 and
#                   up, its multiplies one MULS.L each) and
#                   build/ac3fpu.library (its floating-point build, for a
#                   68040 or 68060's FPU)
#   make test       the fixtures decoded in every mode on this computer
#                   (the same C, its 64-bit multiplies in C) and through
#                   ac3.library on a 68020 under vamos: the samples' CRCs
#                   alike, and as tests/fixtures/expected.txt
#   make lha        build/ac3.lha, for Aminet (no source: it points to
#                   https://github.com/luvwagn/ac3.library)
#
# Needs m68k-amigaos-gcc (bebbo's toolchain, libnix), a C compiler for
# this computer (the IMDCT's tables are made on it), and for the tests
# amitools' vamos.

CC      = m68k-amigaos-gcc
STRIP   = m68k-amigaos-strip
HOSTCC ?= cc
L52     = third_party/liba52
LIB     = build/ac3.library
FPULIB  = build/ac3fpu.library
A52_SRC = $(addprefix $(L52)/liba52/,bitstream.c imdct.c bit_allocate.c parse.c downmix.c)
SRC     = $(A52_SRC) src/ac3lib.c
HDR     = $(wildcard $(L52)/liba52/*.h $(L52)/include/*.h src/port/*.h include/*/*.h)
FLAGS   = -std=gnu11 -Wall -Wno-unused-function -include src/port/config.h -Isrc/port \
          -I$(L52)/include -I$(L52)/liba52 -Iinclude
LIBCC   = $(CC) -O2 -noixemul -nostartfiles -fomit-frame-pointer
INT     = -m68020 -msoft-float -DLIBA52_FIXED -DA52_MUL64 -DA52_IMDCT_TABLES='"imdct_int.h"'
FPU     = -m68040 -mhard-float -DA52_IMDCT_TABLES='"imdct_fpu.h"'
TABLES  = build/imdct_int.h build/imdct_fpu.h
FIX     = $(sort $(wildcard tests/fixtures/*.ac3))

.PHONY: all test lha clean

all: $(LIB) $(FPULIB)

# liba52's IMDCT tables, computed on the build host (src/mktables.c):
# none of them on the Amiga, so no maths library and no wait at Ac3Open
build/imdct_int.h: src/mktables.c $(L52)/liba52/imdct.c $(HDR)
	@mkdir -p build
	@echo "  GEN   $@"
	@$(HOSTCC) -O2 -w -DLIBA52_FIXED -DA52_IMDCT_DUMP $(FLAGS) -o build/mktables_int \
		src/mktables.c $(L52)/liba52/imdct.c -lm
	@build/mktables_int > $@
build/imdct_fpu.h: src/mktables.c $(L52)/liba52/imdct.c $(HDR)
	@mkdir -p build
	@echo "  GEN   $@"
	@$(HOSTCC) -O2 -w -DA52_IMDCT_DUMP $(FLAGS) -o build/mktables_fpu \
		src/mktables.c $(L52)/liba52/imdct.c -lm
	@build/mktables_fpu > $@

# each built twice: build/int and build/fpu (liba52's own files quietly)
build/int/$(L52)/%.o build/fpu/$(L52)/%.o: UPSTREAM = -w
build/int/%.o: %.c $(HDR) $(TABLES)
	@mkdir -p $(dir $@)
	@echo "  CC    $< (ac3)"
	@$(LIBCC) $(INT) $(FLAGS) $(UPSTREAM) -Ibuild -c -o $@ $<
build/fpu/%.o: %.c $(HDR) $(TABLES)
	@mkdir -p $(dir $@)
	@echo "  CC    $< (ac3fpu)"
	@$(LIBCC) $(FPU) $(FLAGS) $(UPSTREAM) -Ibuild -c -o $@ $<

# Its Resident's version from src/ac3lib.c's LibVersion, so the two cannot
# differ: ramlib turns a library away whose Resident is older than the
# version asked for, whatever its lib_Version (make test checks it)
LIBVER = $(shell sed -n 's/^const UWORD LibVersion = \([0-9]*\);.*/\1/p' src/ac3lib.c)
build/libinit.o: src/libinit.c src/libinit.h src/ac3lib.c Makefile
	@mkdir -p build
	@echo "  CC    $<"
	@$(CC) -O2 -m68020 -noixemul -nostartfiles -fomit-frame-pointer -Wall -Wno-array-bounds \
		-Isrc -DLIB_RT_VERSION=$(LIBVER) -c -o $@ $<

$(LIB): build/libinit.o $(patsubst %.c,build/int/%.o,$(SRC))
	@echo "  LD    $@"
	@$(LIBCC) $(INT) -o $@.dbg $^ -lamiga
	@$(STRIP) -o $@ $@.dbg
$(FPULIB): build/libinit.o $(patsubst %.c,build/fpu/%.o,$(SRC))
	@echo "  LD    $@"
	@$(LIBCC) $(FPU) -o $@.dbg $^ -lamiga
	@$(STRIP) -o $@ $@.dbg

# ---- the test: the same CRCs on the host and through ac3.library on a
# 68020 (ac3fpu.library needs a 68040's FPU, which vamos has not)
build/host/ac3_test: tests/ac3_test.c $(SRC) $(HDR) build/imdct_int.h tests/host/exec/types.h
	@mkdir -p build/host
	@echo "  CC    $@"
	@$(HOSTCC) -O2 -DLIBA52_FIXED -DA52_MUL64 -DA52_IMDCT_TABLES='"imdct_int.h"' $(FLAGS) \
		-Wno-unused-variable -Ibuild -Itests/host -o $@ tests/ac3_test.c $(SRC)
build/68k/ac3_test: tests/ac3_test.c include/libraries/ac3.h include/inline/ac3.h $(LIB)
	@mkdir -p build/68k/Libs
	@echo "  CC    $@"
	@$(CC) -O2 -m68020 -noixemul -Wall -Iinclude -o $@ tests/ac3_test.c
	@cp $(LIB) build/68k/Libs/

test: build/host/ac3_test build/68k/ac3_test $(LIB) $(FPULIB) $(FIX)
	@for l in $(LIB) $(FPULIB); do python3 -c "import sys; d=open('$$l','rb').read(); i=d.find(bytes([0x4a,0xfc])); \
		sys.exit(0 if i >= 0 and d[i+11] == $(LIBVER) else '$$l: its Resident says version %d, not $(LIBVER)' % d[i+11])" \
		|| exit 1; done && echo "  CHECK the libraries' Residents: version $(LIBVER), as LibVersion (ramlib turns an older one away)"
	@build/host/ac3_test $(FIX) > build/host/crc.txt
	@cmp -s build/host/crc.txt tests/fixtures/expected.txt \
		&& echo "  CHECK AC-3 decoding (host) ok" || { diff build/host/crc.txt tests/fixtures/expected.txt; exit 1; }
	@cp $(FIX) build/68k/
	@cd build/68k && vamos -C 68020 -m 8192 -V prog:$(abspath build/68k) prog:ac3_test \
		$(addprefix prog:,$(notdir $(FIX))) > out.txt 2> err.txt; grep '\.ac3 ' out.txt > crc.txt; \
		cmp -s crc.txt ../../tests/fixtures/expected.txt \
		&& echo "  CHECK AC-3 decoding (68020, ac3.library) ok" || { cat out.txt err.txt; exit 1; }

# ---- for Aminet: the libraries, their interface and documentation; the
# source is at https://github.com/luvwagn/ac3.library (README.md)
lha: all
	@rm -rf build/lha && mkdir -p build/lha/ac3/Libs build/lha/ac3/Include
	@cp $(LIB) $(FPULIB) build/lha/ac3/Libs/
	@cp -R include/* build/lha/ac3/Include/
	@cp ac3.doc ac3_lib.sfd README.md LICENSE CHANGES build/lha/ac3/
	@cp $(L52)/COPYING build/lha/ac3/COPYING.liba52
	@cp ac3.readme build/lha/ac3.readme
	@cd build/lha && rm -f ../ac3.lha && lha aqo5 ../ac3.lha ac3 ac3.readme
	@echo "  LHA   build/ac3.lha"

clean:
	rm -rf build
