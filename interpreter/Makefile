# CPU and FREQ have to be specified while making
# $ make CPU=attiny44 FREQ=1000000

AR= avr-ar
SOURCES= opcodes.c vm.c register.c

.PHONY: gen
gen : libbrkvm.$(CPU)-$(FREQ).a

libbrkvm.$(CPU)-$(FREQ).a: $(patsubst %.c, %.$(CPU)-$(FREQ).o, $(SOURCES))
	$(AR) rcs $@ $^

%.$(CPU)-$(FREQ).o: %.c
	$(CC) $(AVRCFLAGS) $(CFLAGS) -c -o $@ $^

include ../Makefile.include
