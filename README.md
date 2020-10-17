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

