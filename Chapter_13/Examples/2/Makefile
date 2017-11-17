## http://biot.com/blog/usb-sniffing-on-linux
## yum -i install wireshark
## yum -i install wireshark-gnome

all: doc

run: mount

doc:
	less /usr/src/linux/Documentation/usb/usbmon.txt

mount:
	mount -t debugfs none_debugs /sys/kernel/debug
	modprobe usbmon

clean:
	-rmmod usbmon
	-modprobe -r usbmon
	umount /sys/kernel/debug
