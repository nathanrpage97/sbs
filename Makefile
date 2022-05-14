CFLAGS = -std=c99 -Wall -pedantic

ifdef DEBUG
	CFLAGS += -g
else
	CFLAGS += -O2
endif

all: sbs

# sbs-test: sbs.c sbs.h testhelp.h
# 	$(CC) -o sbs-test sbs.c -Wall -std=c99 -pedantic -O2 -Dsbs_TEST_MAIN
# 	@echo ">>> Type ./sbs-test to run the sbs.c unit tests."

sbs:
	$(CC) $(CFLAGS) -shared -o libsbs.so sbs.c

clean:
	rm -f *.so