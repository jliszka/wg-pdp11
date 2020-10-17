
M11=../bin/macro11
OBJ2BIN=../bin/obj2bin
OBJ2BINOPTS=--binary --rt11

%.obj: %.m11
	$(M11) -l $*.lst -o $@ $<

%.bin: %.obj
	$(OBJ2BIN) $(OBJ2BINOPTS) --outfile=$@ $^

SRCS=$(wildcard *.m11)
BINS=$(SRCS:.m11=.bin)

all: $(BINS)

install:
	cp *.bin ../

clean:
	rm -f *.obj *.bin
