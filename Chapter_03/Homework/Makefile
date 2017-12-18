KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

CDDparm := $(CDDparm)
CDDparm := 0

obj-m := HW.o
HW-objs := CDD2_ops.o CDD2.o

all: 	clean run
	@make -s clean

run: load CDD2app CDD2app_append
	echo "Hello World" > /dev/CDD2;
	cat < /dev/CDD2;
	./CDD2app;
	@echo "lseek to 8 call here fails because O_TRUNC resets to file pos 0."
	echo "Hello World" > /dev/CDD2;	
	./CDD2app_append;
	@echo "lseek call here succeeds to file position 8 because append."

load: compile
	-su -c "{ insmod ./HW.ko CDDparm=$(CDDparm);} || \
		{ echo CDDparm is not set;} ";
	-su -c "cat /proc/devices | grep CDD2;"
	-su -c "mknod -m 666 /dev/CDD2 243 0;"

compile:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

###
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
###

CDD2app:
	-gcc -o CDD2app CDD2app.c;

CDD2app_append:
	-gcc -o CDD2app_append CDD2app_append.c

unload:
	-su -c "rmmod HW; rm -fr /dev/CDD2;"

clean: unload
	-@rm -fr *.o CDD2*.ko .CDD2.* .CDD2.*.* .HW*.* CDD2*.*.* CDD2app .CDD2_ops.* HW.* HW.*.* .tmp_versions .[mM]odule* [mM]o* CDD2app_append
