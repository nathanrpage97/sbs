CFLAGS = -std=c99 -Wall -pedantic

ifdef DEBUG
	CFLAGS += -g
else
	CFLAGS += -O2
endif

all: sbs sbs-test

sbs-test: sbs
	$(CC) $(CFLAGS) -o test-sbs test-sbs.c sbs.c

sbs:
	$(CC) $(CFLAGS) -shared -o libsbs.so sbs.c

clean:
	rm -f *.so

