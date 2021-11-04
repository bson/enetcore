# -*- Makefile -*-

VPATH=.

ENETCORE = enetcore
TOOLSET ?= gnuc

TOOLCHAIN ?= /opt/gcc-arm-none-eabi-10-2020-q4-major
TOOLVER ?= 10.2.1

CONFIG ?= debug

TOOLDIR=$(TOOLCHAIN)/bin

release: CONFIG := release

ifeq ($(CONFIG),opt)
# Runtime bounds checking only adds a trivial amount of code
CFLAGS = -Os -DTRACE -DCHECK_BOUNDS=1 -funsigned-char
endif
ifeq ($(CONFIG),debug)
CFLAGS = -g -O0 -DTRACE -DDEBUG -funsigned-char
AFLAGS = --defsym DEBUG=1
endif
ifeq ($(CONFIG),release)
CFLAGS = -Os -DCHECK_BOUNDS=1 -funsigned-char -fwhole-file
endif

ODIR ?= build_$(CONFIG)

all: $(ODIR)/image

CFLAGS += -ffinite-math-only -fsingle-precision-constant -fno-signed-zeros -fno-math-errno

AFLAGS +=-ahls
LFLAGS=-Map $(ODIR)/image.map

# Project components
SRCS += main.cxx ui.cxx init.cxx startup.s
GENSRCS = buildinfo.cxx sounds.cxx

# UI
#GENSRCS += uidefs.cxx

include makefile.$(TOOLSET)
include $(ENETCORE)/core/makefile.$(TOOLSET)
include $(ENETCORE)/ui/makefile.$(TOOLSET)

LOG ?= $(ODIR)/image.log
#LOG=/dev/stderr

TOUCH_ODIR=$(ODIR)/.touch

CFLAGS += -I. -I$(ENETCORE) -I$(ENETCORE)/$(TOOLSET) -I$(ODIR)

FATSRCS:=$(filter-out %.s, $(SRCS)) $(patsubst %.cxx, $(ODIR)/%.cxx, $(GENSRCS))
FATASRCS:=$(filter %.s, $(SRCS))
FATAOBJS=$(patsubst %.s,$(ODIR)/%.o, $(FATASRCS))

SRCS += $(GENSRCS)

OBJS=$(patsubst %.s,$(ODIR)/%.o, $(patsubst %.cxx, $(ODIR)/%.o, $(SRCS)))
DEPS=$(patsubst %.s,$(ODIR)/%.d, $(patsubst %.cxx, $(ODIR)/%.d, $(SRCS)))


etags: TAGS

TAGS:
	(for dir in . $(ARCH) $(SOCDIR) $(COREDIR) $(ENETCORE)/$(TOOLSET) $(UIDIR) $(ENETCORE)/audio; do                        \
	  find $$dir -name '*.h' ; find $$dir -name '*.cxx';  \
    done) | etags --language=c++ --declarations -

.phony: etags clean release

clean:
	-rm -rf $(ODIR) TAGS

release: $(ODIR)/release

$(ODIR)/release: $(ODIR)/uidecls.h $(FATAOBJS) $(ODIR)/fat_src.o link.cmd
	@echo Linking $@
	@echo $(LD) $(LFLAGS) -o $@ $(FATAOBJS)$(ODIR)/fat_src.o $(LIBS) >>$(LOG)
	@$(LD) $(LFLAGS) -o $@ $(FATAOBJS) $(ODIR)/fat_src.o $(LIBS) 2>&1 >>$(LOG)
	@echo Updating vector checksum
	@$(CHECKSUM) $@ >>$(LOG)
	@$(SIZE) $@ 2>&1 >>$(LOG)
	@$(SIZE) $@

$(ODIR)/fat_src.o: $(FATSRCS) $(TOUCH_ODIR)
	@mkdir -p $(ODIR_TREE)
	@echo $<
	@echo cat $(FATSRCS) \> $(ODIR)/fat_src.cxx >>$(LOG)
	@cat $(FATSRCS) > $(ODIR)/fat_src.cxx 2>&1 2>>$(LOG)
	@echo $(CC) $(CFLAGS) -o $@ -c $(ODIR)/fat_src.cxx >>$(LOG)
	@$(CC) $(CFLAGS) -o $@ -c $(ODIR)/fat_src.cxx 2>&1 >>$(LOG)

$(ODIR)/image: $(ODIR)/uidecls.h $(OBJS) link.cmd 
	@echo Linking $@
	@echo $(LD) $(LFLAGS) -o $@ $(OBJS) $(LIBS) >>$(LOG)
	@$(LD) $(LFLAGS) -o $@ $(OBJS) $(LIBS) 2>&1 >>$(LOG)
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
