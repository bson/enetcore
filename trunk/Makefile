# This should be the location of the enetcore source tree
VPATH=.

BOARD ?= olimex_lpc_l2294
TOOLSET ?= gnuc

TOOLDIR ?= /opt/armgcc-4.6.0/arm-eabi/bin
TOOLVER ?= 4.6.0

CONFIG ?= debug

ifeq ($(CONFIG),opt)
# Runtime bounds checking only adds a trivial amount of code
CFLAGS = -Os -DBUSY_WAIT -DTRACE -I. -I$(TOOLSET) -DCHECK_BOUNDS=1
else
CFLAGS = -g -O0 -DTRACE -DDEBUG -DBUSY_WAIT -I. -I$(TOOLSET)
endif

ODIR ?= build_$(CONFIG)

AFLAGS=-ahls
LFLAGS=-Map $(ODIR)/image.map

# Product components
SRCS += hardware/lis302dl.cpp

# Project components
SRCS += main.cpp

include core/makefile.$(TOOLSET)

# Default build target
all: $(ODIR)/image
