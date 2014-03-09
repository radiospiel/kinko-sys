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
	sudo ../sbin/mkuser kitest-server
	sudo ../sbin/mkuser kitest-server-limited
	sudo ../sbin/mkuser kitest-client kitest-server kitest-server-limited
	sudo ../sbin/mkuser kitest-client-limited kitest-server-limited
	sudo ../sbin/mkuser $(shell whoami) \
						kitest-client \
						kitest-server \
						kitest-client-limited \
						kitest-server-limited

	touch $@

rm-accounts:
	rm -f .test-accounts
	sudo ../sbin/rmusers $(shell ../sbin/lsusers kitest-*)
	sudo ../sbin/rmgroups $(shell ../sbin/lsgroups kitest-*)
