M11=macro11
OBJ2BIN=obj2bin
OBJ2BINOPTS=--binary --rt11
GCC=pdp11-aout-gcc
AR=pdp11-aout-ar
# -nostdlib not to try linking with any system libraries.
# -N put the data section immediately after the code section and not on its own page.
# -fno-inline suppresses inlining functions to make it easier to read assembly.
GCCOPTS=-nostdlib -fno-builtin -N -fno-inline -Os -Ilib -Llib
OBJDUMP=pdp11-aout-objdump
AOUT2LDA=aout2lda.py

%.obj: %.m11
	$(M11) -l $*.lst -o $@ $<

%.bin: %.obj
	$(OBJ2BIN) $(OBJ2BINOPTS) --outfile=$@ $^

# Generate assembly from a .c file using DEC syntax
%.s: %.c
	$(GCC) $(GCCOPTS) -mdec-asm -S -o $@ $<

%.o: %.c
	$(GCC) $(GCCOPTS) -c $<

.PHONY: lib
lib:
	make -C lib

# Compile and link a c file into a.out format
%.aout: %.o lib
	$(GCC) $(GCCOPTS) -Ttext 0x2000 -e _start -g -o $@ crt0.s $< -lc

# Convert from a.out format into the tape loader format
%.bin: %.aout
	$(AOUT2LDA) --aout $< --lda $@ --data-align 2 --text 0x2000

%.dasm: %.aout
	$(OBJDUMP) -d $< > $@

##########################################################

BINS=hello.bin files.bin rm.bin ln.bin mkdir.bin rmdir.bin mv.bin cp.bin touch.bin cat.bin hexdump.bin ls.bin halt.bin load.bin mbr.bin ar.bin

bin.a: $(BINS)
	mkdir -p bin && tar cf - --xform 's/.bin$$//' $^ | tar xf - -C bin && $(AR) rc $@ bin/* && rm -rf bin

all: $(BINS) bin.a

install:
	cp *.bin *.a /opt/pidp11/systems/pdos/

clean:
	rm -f *.o *.obj *.bin *.lst *.aout *.a && make -C lib clean
