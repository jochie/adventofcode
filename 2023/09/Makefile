ALL_FILES=program1.c program2.c
FILES=$(shell ls $(ALL_FILES) 2>/dev/null)
PROGS=$(FILES:%.c=%)
SYMS=$(FILES:%.c=%.dSYM)

CFLAGS=-g

all: $(PROGS)

clean:
	rm -f $(PROGS)
	rm -fr $(SYMS)
