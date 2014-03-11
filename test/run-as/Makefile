# The test Makefile

SBIN=../../sbin

default: recompile tests

recompile:
	make --no-print-directory -C ../.. compile

# -- testcases ----------------------------------------------------------------

tests: accounts
	./setup.sh
	./../roundup test.sh

# -- accounts for tests -------------------------------------------------------

accounts: .accounts

.accounts: Makefile
	@ sudo $(SBIN)/mkuser kitest-server
	@ sudo $(SBIN)/mkuser kitest-server-limited
	@ sudo $(SBIN)/mkuser kitest-client kitest-server kitest-server-limited
	@ sudo $(SBIN)/mkuser kitest-client-limited kitest-server-limited
	@ touch $@

rm-accounts:
	@ rm -f .accounts
	@ sudo $(SBIN)/rmusers $(shell $(SBIN)/lsusers kitest-*)
	@ sudo $(SBIN)/rmgroups $(shell $(SBIN)/lsgroups kitest-*)
