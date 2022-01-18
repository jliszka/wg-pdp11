M11=macro11
OBJ2BIN=obj2bin
OBJ2BINOPTS=--binary --rt11
GCC=pdp11-aout-gcc
AR=pdp11-aout-ar
# -nostdlib not to try linking with any system libraries.
# -N put the data section immediately after the code section and not on its own page.
# -fno-inline suppresses inlining functions to make it easier to read assembly.
GCCOPTS=-nostdlib -fno-builtin -N -fno-inline -Os
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

%.o: %.s
	$(GCC) $(GCCOPTS) -c $<

libc.a: stdio.o fs.o sys.o itoa.o strlen.o strncmp.o strncpy.o
	$(AR) rcs $@ $^

%.dasm: %.aout
	$(OBJDUMP) -d $< > $@

##########################################################

all: libc.a

install:
	cp *.a ../

clean:
	rm -f *.o *.obj *.bin *.lst *.aout *.a
