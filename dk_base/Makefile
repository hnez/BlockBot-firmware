PROGNAME= robot
CPU= attiny44
FREQ= 1000000
SOURCES= $(PROGNAME).c motor.c suart.c timer.c buttons.c leds.c
CFLAGS+= -I../interpreter
CFLAGS+= -L../interpreter
LIBS= -lbrkvm.$(CPU)-$(FREQ)
ATGTS+= libbrkvm.$(CPU)-$(FREQ).a

FUSES= -U efuse:w:0xFF:m -U hfuse:w:0xDF:m -U lfuse:w:0xe2:m

FLASHDEV?= -c stk500v1 -b 19200 -P /dev/ttyUSB0

include ../Makefile.include

.PHONY: libbrkvm.$(CPU)-$(FREQ).a
libbrkvm.$(CPU)-$(FREQ).a :
	make -C ../interpreter CPU=$(CPU) FREQ=$(FREQ)
