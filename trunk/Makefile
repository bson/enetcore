PATH:=/opt/local/bin:$(PATH)

CC      = arm-elf-g++-4.3.2
LD      = arm-elf-ld
AR      = arm-elf-ar
AS      = arm-elf-as
SIZE 	= arm-elf-size

ODIR=build
#  -fno-common
COPT ?= -O0 -DTRACE -DDEBUG

CFLAGS = -I. $(COPT) -g -fno-exceptions -fno-rtti -ffreestanding		\
-fno-threadsafe-statics -marm -mcpu=arm7tdmi-s -DBUSY_WAIT				\
-ffunction-sections -Wno-invalid-offsetof -fwrapv -freg-struct-return	\
-fshort-enums -fno-stack-limit

AFLAGS  = -mcpu=arm7tdmi-s -ahls -mapcs-32 -o $(ODIR)/crt.o
LFLAGS  =  -Map $(ODIR)/image.map -nostartfiles -Tlink.cmd --gc-sections
LIBS    = /opt/local/lib/gcc/arm-elf/4.3.2/libgcc.a
CPFLAGS = -O binary
HEXFLAGS = -O ihex
ODFLAGS	= -x --syms

SRCS=main.cpp hardware.cpp platform.cpp assert.cpp trace.cpp crc32.cpp	\
	crc16.cpp serial.cpp mem.cpp malloc.cpp freelist.cpp util.cpp		\
	arc4.cpp sha1.cpp time.cpp lookup3.cpp pstring.cpp hashtable.cpp	\
	timer.cpp thread.cpp mutex.cpp spi.cpp sdcard.cpp cs8900a.cpp		\
	netaddr.cpp network.cpp dhcp.cpp ip.cpp udp.cpp fat.cpp i2c.cpp		\
	lis302dl.cpp dns.cpp

# Omit this for now (it should go in enetkit anyway!)
# object.cpp

TOUCH_ODIR=$(ODIR)/.touch

OBJS=$(patsubst %.cpp,$(ODIR)/%.o,$(SRCS))
DEPS = $(patsubst %.cpp, $(ODIR)/%.d, $(SRCS))

OBJS += $(ODIR)/buildinfo.o

all: $(ODIR)/image

clean:
	-rm -rf $(ODIR)

$(ODIR)/image: $(ODIR)/crt.o $(OBJS) link.cmd 
	$(LD) $(LFLAGS) -o $@ $(ODIR)/crt.o $(OBJS) $(LIBS)
	$(SIZE) $@

$(ODIR)/crt.o: crt.s $(TOUCH_ODIR)
	$(AS) $(AFLAGS) crt.s > $(ODIR)/crt.lst

$(ODIR)/buildinfo.cpp: .svn/entries
	@echo Generating $@
	@echo "char _build_rev[] = \""`svn info | grep '^Last Changed Rev:' | cut -d: -f2-99 | cut -c2-99`"\";" >$@
	@echo "char _build_date[] = \""`date`"\";" >>$@
	@echo "char _build_user[] = \""${LOGNAME}"@"`hostname`"\";" >>$@
	@echo "#ifdef DEBUG" >>$@
	@echo "char _build_config[] = \"COPT="${COPT}"\";" >>$@
	@echo "char _build_url[] = \""`svn info | grep '^URL:' | cut -d: -f2-99 | cut -c2-99`"\";" >>$@
	@echo "#endif // DEBUG" >>$@

$(ODIR)/buildinfo.o: $(ODIR)/buildinfo.cpp
	$(CC) $(CFLAGS) -MD -o $@ -c $<

$(ODIR)/%.o: %.cpp $(TOUCH_ODIR)
	$(CC) $(CFLAGS) -MD -o $@ -c $<


$(TOUCH_ODIR):
	mkdir -p $(ODIR)
	touch $(TOUCH_ODIR)

-include $(DEPS)
