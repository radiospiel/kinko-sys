test: install
	roundup test/*-test.sh

install: bin/run-as users Makefile
	sudo install -o root -g kinko-test-server -m 6710 bin/run-as bin/kinko-test-server
	sudo install -o root -g kinko-test-client -m 6710 bin/run-as bin/kinko-test-client

clean:
	sudo rm -rf bin/*

bin/run-as: src/run-as.c
	mkdir -p bin
	gcc -o $@ -Wall -Werror $< 

users: .users

.users: Makefile
	sudo sbin/mkuser kinko-test-server
	sudo sbin/mkuser kinko-test-client kinko-test-server
	sudo sbin/mkuser $(shell whoami) \
						kinko-test-client \
						kinko-test-server

	touch .users
