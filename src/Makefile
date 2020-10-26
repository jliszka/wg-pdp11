
M11=../bin/macro11
OBJ2BIN=../bin/obj2bin
OBJ2BINOPTS=--binary --rt11

GCC=pdp11-aout-gcc
# -nostdlib not to try linking with any system libraries.
# -N put the data section immediately after the code section and not on its own page.
# -fno-inline suppresses inlining functions to make it easier to read assembly.
GCCOPTS=-nostdlib -N -fno-inline
AOUT2LDA=../tools/aout2lda.py

%.obj: %.m11
	$(M11) -l $*.lst -o $@ $<

%.bin: %.obj
	$(OBJ2BIN) $(OBJ2BINOPTS) --outfile=$@ $^

# Generate assembly from a .c file using DEC syntax
%.s: %.c
	$(GCC) $(GCCOPTS) -mdec-asm -S -o $@ $<

# Compile and link a c file into a.out format
%.aout: %.c
	$(GCC) $(GCCOPTS) -Ttext 0x200 -e _start -g -o $@ crt0.s $(CLIBS) $<

# Convert from a.out format into the tape loader format
%.bin: %.aout
	$(AOUT2LDA) --aout $< --lda $(patsubst %.aout,%.bin,$<) --data-align 2 --text 0x200 --vector0

SRCS=$(wildcard *.m11)
BINS=$(SRCS:.m11=.bin)

CLIBS=$(wildcard lib*.c)
CSRCS=$(filter-out $(CLIBS), $(wildcard *.c))

CBINS=$(CSRCS:.c=.bin)

all: $(BINS)
cbins: $(CBINS)

install:
	cp *.bin ../

clean:
	rm -f *.obj *.bin *.lst *.aout *.o *.a
