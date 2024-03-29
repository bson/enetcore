# -*- Makefile -*-

VPATH=.

ENETCORE = enetcore
TOOLSET ?= gnuc

#TOOLCHAIN ?= /opt/gcc-arm-none-eabi-10-2020-q4-major
TOOLCHAIN ?= /opt/gcc-arm-none-eabi-10.3-2021.10
TOOLVER ?= 10.3.1

CONFIG ?= debug

TOOLDIR=$(TOOLCHAIN)/bin

ifeq ($(CONFIG),opt)
# Runtime bounds checking only adds a trivial amount of code
CFLAGS = -Os -DTRACE -DCHECK_BOUNDS=1 -funsigned-char
endif
ifeq ($(CONFIG),debug)
CFLAGS = -g3 -O0 -DTRACE -DDEBUG -funsigned-char
AFLAGS = --defsym DEBUG=1
endif
ifeq ($(CONFIG),release)
CFLAGS = -Os -DCHECK_BOUNDS=1 -funsigned-char -fwhole-file
endif

ODIR ?= build_$(CONFIG)

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

CFLAGS += -I. -I$(ENETCORE) -I$(ENETCORE)/$(TOOLSET) -I$(ODIR)

FATSRCS:=$(filter-out %.s, $(SRCS)) $(patsubst %.cxx, $(ODIR)/%.cxx, $(GENSRCS))
FATASRCS:=$(filter %.s, $(SRCS))
FATAOBJS=$(patsubst %.s,$(ODIR)/%.o, $(FATASRCS))
FATOBJ=$(ODIR)/fat_src.o

SRCS += $(GENSRCS)

OBJS=$(patsubst %.s,$(ODIR)/%.o, $(patsubst %.cxx, $(ODIR)/%.o, $(SRCS)))
DEPS=$(patsubst %.s,$(ODIR)/%.d, $(patsubst %.cxx, $(ODIR)/%.d, $(SRCS)))

ODIR_TOUCH=$(ODIR)/.touch

all: $(ODIR)/image

etags: TAGS

TAGS:
	@echo Updating TAGS
	@(for dir in . $(ARCH) $(SOCDIR) $(COREDIR) $(ENETCORE)/$(TOOLSET) $(UIDIR) \
		$(ENETCORE)/audio; do                        \
			find $$dir -name '*.h' ; find $$dir -name '*.cxx';  \
      done) | sed -e "/fat_src\\.cxx/d" | etags --language=c++ --declarations -

.phony: etags clean

clean:
	-rm -rf $(ODIR) TAGS

$(SRCS): $(ODIR)/uidecls.h

ifeq ($(CONFIG),release)
$(ODIR)/image: $(ODIR)/uidecls.h $(FATAOBJS) $(FATOBJ) link.cmd $(ODIR_TOUCH) TAGS
	@echo Linking $@
	@echo $(LD) $(LFLAGS) -o $@ $(FATAOBJS) $(ODIR)/fat_src.o $(LIBS) >>$(LOG)
	@$(LD) $(LFLAGS) -o $@ $(FATAOBJS) $(ODIR)/fat_src.o $(LIBS) 2>&1 >>$(LOG)
	@echo Updating vector checksum
	@$(CHECKSUM) $@ >>$(LOG)
	@$(SIZE) $@ 2>&1 >>$(LOG)
	@$(SIZE) $@
else
$(ODIR)/image: $(ODIR)/uidecls.h $(OBJS) link.cmd  $(ODIR_TOUCH) TAGS
	@echo Linking $@
	@echo $(LD) $(LFLAGS) -o $@ $(OBJS) $(LIBS) >>$(LOG)
	@$(LD) $(LFLAGS) -o $@ $(OBJS) $(LIBS) 2>&1 >>$(LOG)
	@echo Updating vector checksum
	@$(CHECKSUM) $@ >>$(LOG)
	@$(SIZE) $@ 2>&1 >>$(LOG)
	@$(SIZE) $@
endif

$(ODIR)/buildinfo.cxx: $(ODIR_TOUCH)
	@echo Generating $@
	@echo "extern const char _build_branch[] = \"`git rev-parse --abbrev-ref HEAD`\";" >$@
	@echo "extern const char _build_commit[] = \""`git rev-parse HEAD | cut -c-8`"\";" >>$@
	@echo "extern const char _build_date[] = \""`date '+%Y-%m-%d %T%z'`"\";" >>$@
	@echo "extern const char _build_user[] = \""${LOGNAME}"@"`hostname`"\";" >>$@

$(ODIR)/sounds.cxx: $(ENETCORE)/audio/*.wav $(ODIR_TOUCH)
	@echo Generating $@
	@python3 $(ENETCORE)/audio/convert.py $< >$@

$(ODIR)/uidecls.h: ui.def $(ODIR_TOUCH)
	@echo Generating $@
	@python $(ENETCORE)/ui/builder/build.py --decls $< >$@ 2>>$(LOG)

$(ODIR)/uidefs.cxx: ui.def $(ODIR_TOUCH)
	@echo Generating $@
	@python $(ENETCORE)/ui/builder/build.py --defs $< >$@ 2>>$(LOG)

$(ODIR)/fat_src.cxx: $(FATSRCS) $(ODIR_TOUCH)
	@echo Generating $@
	@echo cat $(FATSRCS) \> $(ODIR)/fat_src.cxx >>$(LOG)
	@cat $(FATSRCS) > $(ODIR)/fat_src.cxx 2>>$(LOG)

$(ODIR)/%.o: $(ODIR)/%.cxx $(ODIR_TOUCH)
	@echo $<
	@echo $(CC) $(CFLAGS) -MD -o $@ -c $< >>$(LOG)
	@$(CC) $(CFLAGS) -MD -o $@ -c $< 2>&1 >>$(LOG)

$(ODIR)/%.o: %.cxx $(ODIR_TOUCH)
	@echo $<
	@echo $(CC) $(CFLAGS) -MD -o $@ -c $< >>$(LOG)
	@$(CC) $(CFLAGS) -MD -o $@ -c $< 2>&1 >>$(LOG)

$(ODIR)/%.o: %.s $(ODIR_TOUCH)
	@echo $<
	@echo $(AS) $(AFLAGS) $< -o $@ -a=$(patsubst %.o,%.lst,$@) >>$(LOG)
	@$(AS) $(AFLAGS) $< -o $@ -a=$(patsubst %.o,%.lst,$@)  2>&1 >>$(LOG)

$(ODIR_TOUCH):
	@mkdir -p $(ODIR_TREE)
	@touch $(ODIR_TOUCH)

-include $(DEPS)

