CONFIG_MODULE_SIG=n

obj-m += nulldump.o

all:
	make -C /usr/src/kernels/6.5.6-300.fc39.x86_64 M=$(PWD) modules

test1:
	sudo insmod nulldump.ko
	cat tests/test1.txt | sudo tee /dev/nulldump_device
	sudo rmmod nulldump

test2:
	sudo insmod nulldump.ko
	cat tests/test2.txt | sudo tee /dev/nulldump_device
	sudo rmmod nulldump

test3:
	sudo insmod nulldump.ko
	cat tests/test3.txt | sudo tee /dev/nulldump_device
	sudo rmmod nulldump

clean:
	make -C /usr/src/kernels/6.5.6-300.fc39.x86_64 M=$(PWD) clean
