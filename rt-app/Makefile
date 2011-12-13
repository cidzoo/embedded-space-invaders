##########################################################################
#
# Basic, target/architecture independent makefile for building an
# application that runs on its own stack. 
#
NM			= $(TOOL_PREFIX)-nm
AR			= $(TOOL_PREFIX)-ar
LD			= $(TOOL_PREFIX)-ld
AS			= $(TOOL_PREFIX)-as
CC			= $(TOOL_PREFIX)-gcc
STRIP		= $(TOOL_PREFIX)-strip
OBJCOPY		= $(TOOL_PREFIX)-objcopy
OBJDUMP		= $(TOOL_PREFIX)-objdump

# For execution on EMB board
TOOL_PREFIX	:= arm-unknown-linux-gnu

# Module is rt-app.ko
CC = $(TOOL_PREFIX)-gcc

obj-m = rt-app.o pca9554.o 
rt-app-objs := xeno-ts.o xeno-ts-lib.o xeno-i2c.o rt-app-m.o lcdlib.o
pca9554-objs := pca9554-m.o xeno-i2c.o

KDIR	= ../../src/linux-2.6.14-imx21-xenomai

PWD		= $(shell pwd)
EXTRA_CFLAGS = -Wall -I../../src/linux-2.6.14-imx21-xenomai/include -I../../src/linux-2.6.14-imx21-xenomai/include/xenomai 

all: default i2ctest

i2ctest: i2ctest.c
	$(CC) -static i2ctest.c -o i2ctest

default:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(TOOL_PREFIX)-

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf *.o
	rm -rf i2ctest
	rm -rf *.ko
	rm -rf .*.cmd


	
	




