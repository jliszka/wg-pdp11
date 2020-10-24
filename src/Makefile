
M11=../bin/macro11
OBJ2BIN=../bin/obj2bin
OBJ2BINOPTS=--binary --rt11
GCC=pdp11-aout-gcc -nostdlib
MKTERP=python ../bin/mkterp.py

%.obj: %.m11
	$(M11) -l $*.lst -o $@ $<

%.s: %.c
	$(GCC) -S -o $@ $<

%.obj: %.s
	$(M11) -l $*.lst -o $@ $<

%.bin: %.obj
	$(OBJ2BIN) $(OBJ2BINOPTS) --outfile=$@ $^

SRCS=$(wildcard *.m11)
BINS=$(SRCS:.m11=.bin)

CSRCS=$(wildcard *.c)
CBINS=$(CSRCS:.c=.bin)

all: $(BINS)
cbins: $(CBINS)

install:
	cp *.bin ../

clean:
	rm -f *.obj *.bin *.lst *.s
