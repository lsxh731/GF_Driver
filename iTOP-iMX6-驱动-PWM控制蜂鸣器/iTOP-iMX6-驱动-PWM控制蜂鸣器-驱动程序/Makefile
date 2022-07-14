obj-m += IM6-driver-pwm.o
KDIR = /home/topeet/neo/imx6/20171121/iTOP-iMX6_android4.4.2/kernel_imx/
PWD ?= $(shell pwd)
all:
	make -C  $(KDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=$(KDIR)/../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-
clean:
	rm -rf modules.order *.o workqueue.o  Module.symvers *.mod.c *.ko
