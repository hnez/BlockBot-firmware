PROGNAME= robot
CPU= attiny44
FREQ= 1000000
SOURCES= $(PROGNAME).c motor.c
OBJECTS+= ../interpreter/interpreter.$(CPU)-$(FREQ).a

FLASHDEV?= -c stk500v1 -b 19200 -P /dev/ttyUSB0

include ../Makefile.include

../interpreter/opcodes.$(CPU)-$(FREQ).a :
	make -C ../interpreter CPU=$(CPU) FREQ=$(FREQ)


