default: compile
	make -C test tests

compile: bin/run-as

bin/run-as: src/run-as.c
	mkdir -p bin
	gcc -o $@ -Wall -Werror $< 

