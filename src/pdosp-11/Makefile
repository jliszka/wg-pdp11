
M11=../../bin/macro11
OBJ2BIN=../../bin/obj2bin
OBJ2BINOPTS=--binary --rt11

%.obj: %.m11
	$(M11) -l $*.lst -o $@ $<

SRCS=$(wildcard *.m11)
OBJS=$(patsubst %.m11,%.obj,$(SRCS))
BINS=$(SRCS:.m11=.bin)

all: pdosp-11.bin

pdosp-11.bin: $(OBJS)
	$(OBJ2BIN) $(OBJ2BINOPTS) --outfile=$@ $^

install:
	cp pdosp-11.bin ../../

clean:
	rm -f *.obj *.bin
