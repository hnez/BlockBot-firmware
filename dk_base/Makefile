PROGNAME= robot
CPU= attiny44
FREQ= 1000000
SOURCES= $(PROGNAME).c motor.c suart.c
CFLAGS+= -I../interpreter
CFLAGS+= -L../interpreter
LIBS= -lbrkvm.$(CPU)-$(FREQ)
ATGTS+= libbrkvm.$(CPU)-$(FREQ).a

FLASHDEV?= -c stk500v1 -b 19200 -P /dev/ttyUSB0

include ../Makefile.include

.PHONY: libbrkvm.$(CPU)-$(FREQ).a
libbrkvm.$(CPU)-$(FREQ).a :
	make -C ../interpreter CPU=$(CPU) FREQ=$(FREQ)


