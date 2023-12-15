ALL_FILES=program.c program1.c program2.c
FILES=$(shell ls $(ALL_FILES) 2>/dev/null)
PROGS=$(FILES:%.c=%)
SYMS=$(FILES:%.c=%.dSYM)

CFLAGS=-g
LDFLAGS=

# When using openssl:

# This may require the following in Debian land:
# apt-get install libssl-dev
# apt-get install pkg-config
#
CFLAGS=-g $(shell pkg-config --cflags openssl)
LDFLAGS=$(shell pkg-config --libs openssl)

all: $(PROGS)

.c:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(PROGS)
	rm -fr $(SYMS)
