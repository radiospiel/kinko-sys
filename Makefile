default: compile test-accounts
	cd test && ./setup.sh && roundup *-test.sh

compile: bin/run-as

bin/run-as: src/run-as.c
	mkdir -p bin
	gcc -o $@ -Wall -Werror $< 


# -- accounts for tests -------------------------------------------------------

test-accounts: .test-accounts

rm-accounts:
	rm -f .test-accounts
	sudo sbin/rmusers $(shell sbin/lsusers kinko-test-*)
	sudo sbin/rmgroups $(shell sbin/lsgroups kinko-test-*)
 
.test-accounts: Makefile
	sudo sbin/mkuser kinko-test-server
	sudo sbin/mkuser kinko-test-server-limited
	sudo sbin/mkuser kinko-test-client kinko-test-server kinko-test-server-limited
	sudo sbin/mkuser kinko-test-client-limited kinko-test-server-limited
	sudo sbin/mkuser $(shell whoami) \
						kinko-test-client \
						kinko-test-server \
						kinko-test-client-limited \
						kinko-test-server-limited

	touch $@
