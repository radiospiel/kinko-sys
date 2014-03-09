# The test Makefile

# tests-access: recompile test-setup
# 	roundup access-test.sh

tests: test-setup
	roundup run-as-test.sh
	roundup access-test.sh

test-setup: test-accounts
	./setup.sh
	./setup2.sh

recompile:
	make -C .. compile

all:
	make -C ..

# -- accounts for tests -------------------------------------------------------

test-accounts: .test-accounts

.test-accounts: Makefile
	sudo ../sbin/mkuser kinko-test-server
	sudo ../sbin/mkuser kinko-test-server-limited
	sudo ../sbin/mkuser kinko-test-client kinko-test-server kinko-test-server-limited
	sudo ../sbin/mkuser kinko-test-client-limited kinko-test-server-limited
	sudo ../sbin/mkuser $(shell whoami) \
						kinko-test-client \
						kinko-test-server \
						kinko-test-client-limited \
						kinko-test-server-limited

	touch $@

rm-accounts:
	rm -f .test-accounts
	sudo ../sbin/rmusers $(shell ../sbin/lsusers kinko-test-*)
	sudo ../sbin/rmgroups $(shell ../sbin/lsgroups kinko-test-*)
