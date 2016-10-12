PROGNAME= brick
CPU= attiny85
FREQ= 8000000
FUSES= -U efuse:w:0xFF:m -U hfuse:w:0xDF:m -U lfuse:w:0xE2:m

SOURCES= $(PROGNAME).c uart.c
CFLAGS+= -I../rdbuf
CFLAGS+= -L../rdbuf
CFLAGS+= -I../pktpsr
CFLAGS+= -L../pktpsr
LIBS+= -lrdbuf.$(CPU)-$(FREQ)
LIBS+= -lpktpsr.$(CPU)-$(FREQ)
ATGTS+= librdbuf.$(CPU)-$(FREQ).a
ATGTS+= libpktpsr.$(CPU)-$(FREQ).a
OBJECTS+= brick_binary.o

FLASHDEV?= -c usbasp

include ../Makefile.include

.PHONY: librdbuf.$(CPU)-$(FREQ).a
.PHONY: libpktpsr.$(CPU)-$(FREQ).a
librdbuf.$(CPU)-$(FREQ).a:
	make -C ../rdbuf CPU=$(CPU) FREQ=$(FREQ)
libpktpsr.$(CPU)-$(FREQ).a:
	make -C ../pktpsr CPU=$(CPU) FREQ=$(FREQ)
