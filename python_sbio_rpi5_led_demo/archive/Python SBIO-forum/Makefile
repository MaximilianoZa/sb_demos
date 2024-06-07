#
SB_ROOT=../runtimes/linux-raspberry-armle-opengles_2.0-GST01-obj
SB_INC=$(SB_ROOT)/include
SB_LIB=$(SB_ROOT)/lib 

CC=gcc
CFLAGS = -I$(SB_INC) -Wall `python-config --cflags`
LDFLAGS = -L$(SB_LIB) -lgreio `python-config --ldflags`

ifeq ($(DEBUG), TRUE)
    CCFLAGS += -g
endif

src = sbio_python.c
obj = $(src:.c=.o)
dep = $(obj:.o=.d)  # one dependency file for each source

sbio_test: $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

# rule to generate a dep file by using the C preprocessor
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -f $(obj) sbio_python

.PHONY: cleandep
cleandep:
	rm -f $(dep)

