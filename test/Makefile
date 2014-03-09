# The test Makefile

tests: recompile test-accounts run-as access

recompile:
	make -C .. compile

# -- testcases ----------------------------------------------------------------

run-as: 
	./run-as-setup.sh
	./roundup run-as-test.sh

access:
	./access-setup.sh
	./roundup access-test.sh
	

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
