PATH:=/opt/local/bin:$(PATH)

CC      = arm-elf-g++-4.3.2
LD      = arm-elf-ld -v
AR      = arm-elf-ar
AS      = arm-elf-as
OCP      = arm-elf-objcopy
OD		= arm-elf-objdump

#  -fno-common
COPT ?= -O0 -DTRACE -DDEBUG

CFLAGS  = -I. $(COPT) -g -fno-exceptions -fno-rtti -ffreestanding \
	-fno-threadsafe-statics -marm  -mcpu=arm7tdmi-s -DBUSY_WAIT
AFLAGS  = -mcpu=arm7tdmi-s -ahls -mapcs-32 -o crt.o
LFLAGS  =  -Map main.map -nostartfiles -Tlink.cmd
LIBS    = /opt/local/lib/gcc/arm-elf/4.3.2/libgcc.a
CPFLAGS = -O binary
HEXFLAGS = -O ihex
ODFLAGS	= -x --syms

SRCS=main.cpp hardware.cpp platform.cpp assert.cpp trace.cpp			\
	serial.cpp mem.cpp malloc.cpp freelist.cpp util.cpp arc4.cpp		\
	sha1.cpp object.cpp time.cpp lookup3.cpp pstring.cpp hashtable.cpp	\
	timer.cpp thread.cpp mutex.cpp

OBJS=$(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp, %.d, $(SRCS))

all: image.bin image.dmp image.hex

clean:
	-rm -f crt.lst image.lst crt.o $(OBJS) image image.hex image.map image.dmp image.bin

image: crt.o $(OBJS) link.cmd 
	$(LD) $(LFLAGS) -o $@ crt.o $(OBJS) $(LIBS)

%.bin: %
	$(OCP) $(CPFLAGS) $< $@

%.dmp: %
	$(OD) $(ODFLAGS) $< >$@

%.hex: %
	$(OCP) $(HEXFLAGS) $< $@

crt.o: crt.s
	$(AS) $(AFLAGS) crt.s > crt.lst

%.o: %.cpp
	$(CC) $(CFLAGS) -MD -c $<

-include $(DEPS)
