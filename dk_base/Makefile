PROGNAME= robot
CPU= attiny44
FREQ= 1000000
SOURCES= $(PROGNAME).c motor.c
CFLAGS+= -I../interpreter
CFLAGS+= -L../interpreter -lbrkvm.$(CPU)-$(FREQ)
TARGETS+= libbrkvm.$(CPU)-$(FREQ)

FLASHDEV?= -c stk500v1 -b 19200 -P /dev/ttyUSB0

include ../Makefile.include

libbrkvm.$(CPU)-$(FREQ).a :
	make -C ../interpreter CPU=$(CPU) FREQ=$(FREQ)


