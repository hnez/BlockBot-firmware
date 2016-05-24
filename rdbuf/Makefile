# CPU and FREQ have to be specified while making
# $ make CPU=attiny44 FREQ=1000000

AR= avr-ar
SOURCES= rdbuf.c

.PHONY: gen
gen : librdbuf.$(CPU)-$(FREQ).a

librdbuf.$(CPU)-$(FREQ).a: $(patsubst %.c, %.$(CPU)-$(FREQ).o, $(SOURCES))
	$(AR) rcs $@ $^

%.$(CPU)-$(FREQ).o: %.c
	$(CC) $(AVRCFLAGS) $(CFLAGS) -c -o $@ $^

include ../Makefile.include
