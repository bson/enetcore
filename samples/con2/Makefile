# -*- Makefile -*-

VPATH=.

ENETCORE = enetcore
TOOLSET ?= gnuc

TOOLCHAIN ?= /opt/gcc-arm-none-eabi-10-2020-q4-major
TOOLVER ?= 10.2.1

CONFIG ?= debug

TOOLDIR=$(TOOLCHAIN)/bin

ifeq ($(CONFIG),opt)
# Runtime bounds checking only adds a trivial amount of code
CFLAGS = -Os -DTRACE -I. -I$(ENETCORE)/$(TOOLSET) -DCHECK_BOUNDS=1 -funsigned-char
else
CFLAGS = -g -O0 -DTRACE -DDEBUG -I. -I$(ENETCORE)/$(TOOLSET) -funsigned-char
AFLAGS = --defsym DEBUG=1
endif

ODIR ?= build_$(CONFIG)

AFLAGS +=-ahls
LFLAGS=-Map $(ODIR)/image.map

# Project components
#SRCS += main.cxx usb.cxx ui.cxx
SRCS += main.cxx

SRCS += buildinfo.cxx sounds.cxx

# UI
#SRCS += uidefs.cxx

include makefile.$(TOOLSET)
include $(ENETCORE)/core/makefile.$(TOOLSET)
include $(ENETCORE)/ui/makefile.$(TOOLSET)

LOG ?= $(ODIR)/image.log
#LOG=/dev/stderr

TOUCH_ODIR=$(ODIR)/.touch

CFLAGS += -I$(ODIR)

OBJS=$(patsubst %.s,$(ODIR)/%.o, $(patsubst %.cxx, $(ODIR)/%.o, $(SRCS)))
DEPS=$(patsubst %.s,$(ODIR)/%.d, $(patsubst %.cxx, $(ODIR)/%.d, $(SRCS)))


all: $(ODIR)/image

etags: TAGS

TAGS:
	(for dir in . $(ARCH) $(SOCDIR) $(COREDIR) $(ENETCORE)/$(TOOLSET) $(UIDIR) $(ENETCORE)/audio; do                        \
	  find $$dir -name '*.h' ; find $$dir -name '*.cxx';  \
    done) | etags --language=c++ --declarations -

.phony: etags clean

clean:
	-rm -rf $(ODIR) TAGS

$(ODIR)/image: $(ODIR)/startup.o $(ODIR)/uidecls.h $(OBJS) link.cmd 
	@echo Linking $@
	@echo $(LD) $(LFLAGS) -o $@ $(ODIR)/startup.o $(OBJS) $(LIBS) >>$(LOG)
	@$(LD) $(LFLAGS) -o $@ $(ODIR)/startup.o $(OBJS) $(LIBS) 2>&1 >>$(LOG)
	@echo Updating vector checksum
	@$(CHECKSUM) $@ >>$(LOG)
	@$(SIZE) $@ 2>&1 >>$(LOG)
	@$(SIZE) $@

$(ODIR)/startup.o: startup.s $(TOUCH_ODIR)
	@echo startup.s
	@echo $(AS) $(AFLAGS) startup.s -o $@ -a=$(ODIR)/startup.lst >>$(LOG)
	@$(AS) $(AFLAGS) startup.s -o $@ -a=$(ODIR)/startup.lst 2>&1 >>$(LOG)

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

$(ODIR)/sounds.cxx: $(ENETCORE)/audio/bell.wav
	@echo Generating $@
	@mkdir -p $(ODIR_TREE)
	@python3 $(ENETCORE)/audio/convert.py $(ENETCORE)/audio/bell.wav >$@

$(ODIR)/sounds.o: $(ODIR)/sounds.cxx
	@echo $<
	@echo $(CC) $(CFLAGS) -MD -o $@ -c $< >>$(LOG)
	@$(CC) $(CFLAGS) -MD -o $@ -c $< 2>&1 >>$(LOG)

$(ODIR)/uidecls.h: ui.def
	@echo Generating $@
	@mkdir -p $(ODIR_TREE)
	@python $(ENETCORE)/ui/builder/build.py --decls $< >$@ 2>>$(LOG)

$(ODIR)/uidefs.cxx: ui.def
	@echo Generating $@
	@mkdir -p $(ODIR_TREE)
	@python $(ENETCORE)/ui/builder/build.py --defs $< >$@ 2>>$(LOG)

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
	@mkdir -p $(ODIR_TREE)
	@touch $(TOUCH_ODIR)

-include $(DEPS)

.phony: clean
