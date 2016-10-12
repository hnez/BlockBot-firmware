PROGNAME= bootloader
CPU= attiny85
FREQ= 8000000
FUSES= -U efuse:w:0xFE:m -U hfuse:w:0xDF:m -U lfuse:w:0xE2:m

LDFLAGS= -Wl,-Ttext=0x1C00

SOURCES= $(PROGNAME).c

FLASHDEV?= -c usbasp

include ../Makefile.include
