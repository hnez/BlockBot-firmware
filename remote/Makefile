PROGNAME= remote
CPU= atmega8
FREQ= 12000000
SOURCES= $(PROGNAME).c led.c power.c

FUSES= -U hfuse:w:0xC9:m -U lfuse:w:0xFF:m

FLASHDEV?= -c usbasp

include ../Makefile.include
