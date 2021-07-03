# This should be the location of the enetcore source tree
VPATH=.

BOARD ?= skyblue
TOOLSET ?= gnuc

TOOLDIR ?= /home/bson/gcc-arm-none-eabi-10-2020-q4-major/bin
TOOLVER ?= 10.2.1

CONFIG ?= debug

ifeq ($(CONFIG),opt)
# Runtime bounds checking only adds a trivial amount of code
CFLAGS = -Os -DTRACE -I. -I$(TOOLSET) -DCHECK_BOUNDS=1 -funsigned-char
else
CFLAGS = -g -O0 -DTRACE -DDEBUG -I. -I$(TOOLSET) -funsigned-char
AFLAGS = --defsym DEBUG=1
endif

ODIR ?= build_$(CONFIG)

AFLAGS +=-ahls
LFLAGS=-Map $(ODIR)/image.map

# Project components
SRCS += main.cxx buildinfo.cxx usb.cxx ui.cxx

# UI
SRCS += uidefs.cxx

include $(BOARD)/makefile.$(TOOLSET)
include core/makefile.$(TOOLSET)
include ui/makefile.$(TOOLSET)

LOG=$(ODIR)/image.log
#LOG=/dev/stderr

TOUCH_ODIR=$(ODIR)/.touch

CFLAGS += -I$(ODIR)

OBJS=$(patsubst %.s,$(ODIR)/%.o,$(patsubst %.cxx,$(ODIR)/%.o,$(SRCS)))
DEPS=$(patsubst %.s,$(ODIR)/%.d, $(patsubst %.cxx, $(ODIR)/%.d, $(SRCS)))


all: $(ODIR)/image

etags: TAGS

TAGS:
	(find . -name '*.h' ; find . -name '*.cxx') | etags --language=c++ --declarations -

.phony: etags clean

clean:
	-rm -rf $(ODIR)

$(ODIR)/image: $(ODIR)/startup.o $(ODIR)/uidecls.h $(OBJS) $(BOARD)/link.cmd 
	@echo Linking $@
	@echo $(LD) $(LFLAGS) -o $@ $(ODIR)/startup.o $(OBJS) $(LIBS) >>$(LOG)
	@$(LD) $(LFLAGS) -o $@ $(ODIR)/startup.o $(OBJS) $(LIBS) 2>&1 >>$(LOG)
	@echo Updating vector checksum
	@$(CHECKSUM) $@ >>$(LOG)
	@$(SIZE) $@ 2>&1 >>$(LOG)
	@$(SIZE) $@

$(ODIR)/startup.o: $(BOARD)/startup.s $(TOUCH_ODIR)
	@echo $(BOARD)/startup.s
	@echo $(AS) $(AFLAGS) $(BOARD)/startup.s -o $@ -a=$(ODIR)/startup.lst >>$(LOG)
	@$(AS) $(AFLAGS) $(BOARD)/startup.s -o $@ -a=$(ODIR)/startup.lst 2>&1 >>$(LOG)

$(ODIR)/buildinfo.cxx:
	@echo Generating $@
	@mkdir -p $(ODIR_TREE)
	@echo "extern const char _build_branch[] = \"`git rev-parse --abbrev-ref HEAD`\";" >>$@
	@echo "extern const char _build_commit[] = \""`git rev-parse HEAD | cut -c-8`"\";" >>$@
	@echo "extern const char _build_date[] = \""`date '+%Y-%m-%d %T%z'`"\";" >>$@
	@echo "extern const char _build_user[] = \""${LOGNAME}"@"`hostname`"\";" >>$@

$(ODIR)/buildinfo.o: $(ODIR)/buildinfo.cxx
	@mkdir -p $(ODIR_TREE)
	@echo $<
	@echo $(CC) $(CFLAGS) -MD -o $@ -c $< >>$(LOG)
	@$(CC) $(CFLAGS) -MD -o $@ -c $< 2>&1 >>$(LOG)

$(ODIR)/uidecls.h: ui.def
	@echo Generating $@
	@mkdir -p $(ODIR_TREE)
	@python ui/builder/build.py --decls $< >$@ 2>>$(LOG)

$(ODIR)/uidefs.cxx: ui.def
	@echo Generating $@
	@mkdir -p $(ODIR_TREE)
	@python ui/builder/build.py --defs $< >$@ 2>>$(LOG)

$(ODIR)/uidefs.o: $(ODIR)/uidefs.cxx
	@echo $<
	@echo $(CC) $(CFLAGS) -MD -o $@ -c $< >>$(LOG)
	@$(CC) $(CFLAGS) -MD -o $@ -c $< 2>&1 >>$(LOG)

$(ODIR)/%.o: %.cxx $(TOUCH_ODIR)
	@echo $<
	@echo $(CC) $(CFLAGS) -MD -o $@ -c $< >>$(LOG)
	@$(CC) $(CFLAGS) -MD -o $@ -c $< 2>&1 >>$(LOG)

$(ODIR)/%.o: %.s $(TOUCH_ODIR)
	@echo $<
	@echo $(AS) $(AFLAGS) $< -o $@ -a=$(patsubst %.o,%.lst,$@) >>$(LOG)
	@$(AS) $(AFLAGS) $< -o $@ -a=$(patsubst %.o,%.lst,$@)  2>&1 >>$(LOG)

$(TOUCH_ODIR):
	mkdir -p $(ODIR_TREE)
	touch $(TOUCH_ODIR)

-include $(DEPS)

.phony: clean
