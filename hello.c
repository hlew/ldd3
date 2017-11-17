#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/schedh.>


MODULE_AUTHOR("Hiram");
MODULE_LICENSE("GPL");


static int hello_init(void) {
	printk(KERN_INFO "Hello. Called by pid (%d)", current->pid);
}

module_init(hello_init);
modlue_exit(hello_init);

