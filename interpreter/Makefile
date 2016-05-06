# CPU and FREQ have to be specified while making
# $ make CPU=attiny44 FREQ=1000000

SOURCES= memory.c opcodes.c vm.c

.PHONY: gen
gen : interpreter.$(CPU)-$(FREQ).a

interpreter.$(CPU)-$(FREQ).a: $(patsubst %.c, %.$(CPU)-$(FREQ).o, $(SOURCES))
	ar rcs $@ $^

%.$(CPU)-$(FREQ).o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

include ../Makefile.include
