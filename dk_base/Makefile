PROGNAME= robot
CPU= atmega8
FREQ= 12000000
SOURCES= $(PROGNAME).c motor.c timer.c buttons.c leds.c vm_ram.c
CFLAGS+= -I../interpreter -I ../saxparse
CFLAGS+= -L../interpreter -L ../saxparse
LIBS= -lbrkvm.$(CPU)-$(FREQ) -lmsgsax.$(CPU)-$(FREQ)
ATGTS+= libbrkvm.$(CPU)-$(FREQ).a libmsgsax.$(CPU)-$(FREQ).a

FUSES= -U efuse:w:0xFF:m -U hfuse:w:0xDF:m -U lfuse:w:0xe2:m

FLASHDEV?= -c usbasp

include ../Makefile.include

.PHONY: libbrkvm.$(CPU)-$(FREQ).a
libbrkvm.$(CPU)-$(FREQ).a :
	make -C ../interpreter CPU=$(CPU) FREQ=$(FREQ)

.PHONY: libmsgsax.$(CPU)-$(FREQ).a
libmsgsax.$(CPU)-$(FREQ).a :
	make -C ../saxparse CPU=$(CPU) FREQ=$(FREQ)
