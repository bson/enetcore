BOARD ?= olimex_lpc_l2294
TOOLSET ?= gnuc
TOOLDIR ?= /opt/local/bin
TOOLVER ?= 4.3.2

CONFIG ?= debug

ifeq ($(CONFIG),opt)
CFLAGS = -Os -DBUSY_WAIT -DTRACE -I. -I$(TOOLSET)
else
CFLAGS = -g -O0 -DTRACE -DDEBUG -DBUSY_WAIT -I. -I$(TOOLSET)
endif

ODIR ?= build_$(CONFIG)

AFLAGS=-ahls
LFLAGS=-Map $(ODIR)/image.map

# Product components
SRCS += hardware/lis302dl.cpp

# Project components
SRCS=main.cpp

include core/makefile.$(TOOLSET)

# Default build target
all: $(ODIR)/image
