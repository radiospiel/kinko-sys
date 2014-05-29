MAKEFLAGS+=--no-print-directory -C
CFLAGS+=-Wall -Werror
OPT+=-DSTRICT_MODE=0

default: compile
	make --no-print-directory -C test tests

compile: bin/run-as bin/run-asd

bin/run-as: src/run-as.c
	mkdir -p bin
	gcc -o $@ $(CFLAGS) $< 

bin/run-asd: src/run-as.c
	mkdir -p bin
	gcc -o $@ $(CFLAGS) $(OPT) $< 

clean: 
	rm -rf bin
