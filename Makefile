CFLAGS = -std=c99 -Wall -pedantic

ifdef DEBUG
	CFLAGS += -g
else
	CFLAGS += -O2
endif

# for embedded use cases who may not want to support stdarg
ifdef NO_FORMAT
	CFLAGS += -DSBS_NO_FORMAT
endif

all: sbs sbs-test

sbs-test: sbs
	$(CC) $(CFLAGS) -o test-sbs test-sbs.c sbs.c

sbs:
	$(CC) $(CFLAGS) -shared -o libsbs.so sbs.c

clean:
	rm -f *.so

