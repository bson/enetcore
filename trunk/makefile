PATH:=/opt/local/bin:$(PATH)

CC      = arm-elf-g++-4.3.2
LD      = arm-elf-ld -v
AR      = arm-elf-ar
AS      = arm-elf-as
OCP      = arm-elf-objcopy
OD		= arm-elf-objdump

CFLAGS  = -I. -c -fno-common -O0 -g -fno-exceptions -fno-rtti -DBUSY_WAIT
AFLAGS  = -ahls -mapcs-32 -o crt.o
LFLAGS  =  -Map main.map -nostartfiles -Tlink.cmd
CPFLAGS = -O binary
HEXFLAGS = -O ihex
ODFLAGS	= -x --syms

SRCS=main.cpp hwinit.cpp

OBJS=$(patsubst %.cpp,%.o,$(SRCS))

all: main

clean:
	-rm -f crt.lst main.lst crt.o $(OBJS) main.out main.hex main.map main.dmp main.bin

main: crt.o $(OBJS) link.cmd 
	$(LD) $(LFLAGS) -o $@ crt.o $(OBJS)
	$(OCP) $(CPFLAGS) main main.bin
	$(OD) $(ODFLAGS) main > main.dmp
	$(OCP) $(HEXFLAGS) main main.hex

crt.o: crt.s
	$(AS) $(AFLAGS) crt.s > crt.lst

%.o: %.cpp
	$(CC) $(CFLAGS) $<
