
.PHONY: lib
lib:
	make -C lib all

.PHONY: bin
bin:
	make -C bin all

.PHONY: test
test:
	make -C test all

all: lib bin test

install:
	make -C bin tests && make -C test install

clean:
	make -C lib clean && make -C bin clean && make -C test clean
