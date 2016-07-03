PROGNAME= brick
CPU= attiny85
FREQ= 8000000
SOURCES= $(PROGNAME).c uart.c
CFLAGS+= -I../rdbuf
CFLAGS+= -L../rdbuf
CFLAGS+= -I../pktpsr
CFLAGS+= -L../pktpsr
LIBS+= -lrdbuf.$(CPU)-$(FREQ)
LIBS+= -lpktpsr.$(CPU)-$(FREQ)
ATGTS+= librdbuf.$(CPU)-$(FREQ).a
ATGTS+= libpktpsr.$(CPU)-$(FREQ).a

FLASHDEV?= -c stk500v1 -b 19200 -P /dev/ttyUSB0

include ../Makefile.include

.PHONY: librdbuf.$(CPU)-$(FREQ).a
.PHONY: libpktpsr.$(CPU)-$(FREQ).a
librdbuf.$(CPU)-$(FREQ).a :
	make -C ../rdbuf CPU=$(CPU) FREQ=$(FREQ)
libpktpsr.$(CPU)-$(FREQ).a :
	make -C ../pktpsr CPU=$(CPU) FREQ=$(FREQ)
