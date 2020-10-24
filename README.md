# wg-pdp11

Sample MACRO-11 assembly programs for #wg-pdp11

## Setup

Install the macro11 assembler:
```
$ mkdir bin
$ git clone https://github.com/j-hoppe/MACRO11.git
$ cd macro11
$ make
$ cp macro11 ../bin
$ cd ..
```

Install the linker:
```
$ git clone https://github.com/AK6DN/obj2bin.git
$ cp obj2bin/obj2bin.pl bin/obj2bin
$ chmod a+x bin/obj2bin
```

Clean up:
```
$ rm -rf macro11 obj2bin
```

## Building and running the samples

```
$ cd src
$ make all install
```

If you want to key it in manually, the generated `.lst` file will give you all the opcodes in octal.

An easier way is to load the binary in the SimH repl.
```
$ ./launch.sh blank.simh_pdp11
sim> load counter.bin
```

Then load the start address on the console and depress the `START` switch.

## Connecting a terminal

For programs that require a terminal, use the `terminal.simh_pdp11` SimH start script:
```
$ ./launch.sh terminal.simh_pdp11
sim> load counter.bin
```

Then in a separate window:
```
$ telnet localhost 9000
```

## gcc cross compiler

Reference: http://docs.cslabs.clarkson.edu/wiki/Developing_for_a_PDP-11

1. Build binutils

Download from http://gnu.mirrors.hoobly.com/binutils/binutils-2.35.tar.gz

```o
cd binutils-2.35
mkdir build-pdp11-aout
../configure --target=pdp11-aout --prefix=/usr/local
echo "\n\nGet up and stretch\n\n" && sleep 2 && make -j8
sudo make install
```

2. Build gcc

Download from http://gnu.askapache.com/gcc/gcc-10.2.0/

```
cd gcc-10.2.0
contrib/download_prerequisites
mkdir build-pdp11-aout
cd build-pdp11-aout
../configure --target=pdp11-aout --prefix=/usr/local --enable-languages=c --disable-libstdc++-v3 --disable-libssp --disable-libgfortran --disable-libobjc
echo "\n\nGo get a coffee or a snack.\nThis will take a while.\n\n" && sleep 2 && make -j8
sudo make install
```

