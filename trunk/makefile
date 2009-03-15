PATH:=/opt/local/bin:$(PATH)

CC      = arm-elf-g++-4.3.2
LD      = arm-elf-ld -v
AR      = arm-elf-ar
AS      = arm-elf-as
OCP      = arm-elf-objcopy
OD		= arm-elf-objdump

CFLAGS  = -I. -fno-common -O0 -g -fno-exceptions -fno-rtti -DBUSY_WAIT
AFLAGS  = -ahls -mapcs-32 -o crt.o
LFLAGS  =  -Map main.map -nostartfiles -Tlink.cmd
CPFLAGS = -O binary
HEXFLAGS = -O ihex
ODFLAGS	= -x --syms

SRCS=main.cpp hwinit.cpp

OBJS=$(patsubst %.cpp,%.o,$(SRCS))

all: image

clean:
	-rm -f crt.lst image.lst crt.o $(OBJS) image image.hex image.map image.dmp image.bin

image: crt.o $(OBJS) link.cmd 
	$(LD) $(LFLAGS) -o $@ crt.o $(OBJS)
	$(OCP) $(CPFLAGS) image image.bin
	$(OD) $(ODFLAGS) image > image.dmp
	$(OCP) $(HEXFLAGS) image image.hex

crt.o: crt.s
	$(AS) $(AFLAGS) crt.s > crt.lst

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<
