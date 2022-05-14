all: sbs-test

sbs-test: sbs.c sbs.h testhelp.h
	$(CC) -o sbs-test sbs.c -Wall -std=c99 -pedantic -O2 -Dsbs_TEST_MAIN
	@echo ">>> Type ./sbs-test to run the sbs.c unit tests."

clean:
	rm -f sbs-test