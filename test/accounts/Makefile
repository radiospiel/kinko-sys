# The test Makefile

SBIN=../../sbin

default: recompile tests

recompile:
	make --no-print-directory -C ../.. compile

tests: 
	@ sudo ls > /dev/null         # make sure ls asks for a password here and now
	./../roundup test.sh
