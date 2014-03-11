default: compile
	make --no-print-directory -C test tests

compile: bin/run-as bin/run-asd

bin/run-as: src/run-as.c
	mkdir -p bin
	gcc -o $@ -Wall -Werror $< 

bin/run-asd: src/run-as.c
	mkdir -p bin
	gcc -o $@ -Wall -Werror -DSTRICT_MODE=0 $< 
