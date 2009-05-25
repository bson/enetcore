BOARD ?= olimex_lpc_l2294
TOOLSET ?= gnuc
TOOLDIR ?= /opt/local/bin
TOOLVER ?= 4.3.2
ODIR ?= build

#  -fno-common
CFLAGS = -g -O0 -DTRACE -DDEBUG -DBUSY_WAIT -I. -I$(TOOLSET)
# CFLAGS = -Os -DBUSY_WAIT -DTRACE -I. -I$(TOOLSET)

AFLAGS=-ahls
LFLAGS=-Map $(ODIR)/image.map

# Product components
SRCS += hardware/lis302dl.cpp

# Project components
SRCS=main.cpp

include core/makefile.$(TOOLSET)

# Default build target
all: $(ODIR)/image
