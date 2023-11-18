#specifies kernel module
obj-m += ko5204.o

#-C puts it in a directory .../build
#M= puts in current working directory
#modules does ?
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean