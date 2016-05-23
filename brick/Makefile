PROGNAME= brick
CPU= attiny85
FREQ= 8000000
SOURCES= $(PROGNAME).c
CFLAGS+= -I../rdbuf
CFLAGS+= -L../rdbuf
LIBS= -lrdbuf.$(CPU)-$(FREQ)
ATGTS+= librdbuf.$(CPU)-$(FREQ).a

FLASHDEV?= -c stk500v1 -b 19200 -P /dev/ttyUSB0

include ../Makefile.include

.PHONY: librdbuf.$(CPU)-$(FREQ).a
librdbuf.$(CPU)-$(FREQ).a :
	make -C ../rdbuf CPU=$(CPU) FREQ=$(FREQ)
