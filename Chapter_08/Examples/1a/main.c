// Example 8.1a	:  kmalloc() + ksize()
//		:  main.c

//  

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>

#include <linux/mm.h>
#include <linux/slab.h> // ksize() included here since 2.6.29
// see http://lwn.net/Articles/319686/
#include <asm/uaccess.h>
#include <linux/proc_fs.h>

#include <linux/sched.h>

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define PROC_HELLO_BUFLEN 1024
#define HELLO "hello"
#define MYDEV "MYDEV"

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char *proc_hello_value;
	char proc_hello_flag;
};

static struct proc_hello_data *hello_data;

static struct proc_dir_entry *proc_hello;
static struct proc_dir_entry *proc_mydev;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	static int eof[1];
static ssize_t read_hello(struct file *file, char *buf,
size_t len, loff_t *ppos)
#else
static int read_hello (char *buf, char **start, off_t offset,
    int len, int *eof, void *unused)
#endif
{

	struct proc_hello_data *usrsp=hello_data;


	if (*eof) { *eof=0; return 0; }
	else if (usrsp->proc_hello_flag) {
	  *eof = 1;
		usrsp->proc_hello_flag=0;
		return(sprintf(buf,
				"Hello .. I got \"%s\"\n", 
				usrsp->proc_hello_value)); 
	}
	else
	  *eof = 1;
		return(sprintf(buf,
				"Hello from process %d\njiffies=%ld\n", 
				(int)current->pid,jiffies));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static ssize_t write_hello(struct file *file, const char __user *buf,
  size_t count, loff_t *ppos)
#else
static int write_hello (struct file *file,const char * buf,
    unsigned long count, void *data)
#endif
{
	int length=count;
	struct proc_hello_data *usrsp=hello_data;

	length = (length<PROC_HELLO_LEN)? length:PROC_HELLO_LEN;

	if (copy_from_user(usrsp->proc_hello_value, buf, length)) 
		return -EFAULT;

	usrsp->proc_hello_value[length-1]=0;
	usrsp->proc_hello_flag=1;
	return(length);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static const struct file_operations proc_fops =
{
 .owner = THIS_MODULE,
 .read  = read_hello,
 .write = write_hello
};
#endif

static int my_init (void) {
	proc_mydev = proc_mkdir(MYDEV,0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
  proc_hello = proc_create("hello", 0777, proc_mydev, &proc_fops);
#else
  proc_hello = create_proc_entry(HELLO,0,proc_mydev);
  proc_hello->read_proc = read_hello;
  proc_hello->write_proc = write_hello;
#endif

	hello_data=(struct proc_hello_data *)
		kmalloc(sizeof(*hello_data),GFP_KERNEL);

	hello_data->proc_hello_value=(char *)
		kmalloc(PROC_HELLO_BUFLEN,GFP_KERNEL);
	
	hello_data->proc_hello_flag=0;

        // module init message
        printk(KERN_ALERT "2470:8.1: main initialized!\n");
        printk(KERN_ALERT "2470:8.1: memory allocated(hello_data) = %d!\n", ksize(hello_data));
        printk(KERN_ALERT "2470:8.1: memory allocated(hello_data) = %d!\n", ksize(hello_data->proc_hello_value));
	return 0;
}

static void my_exit (void) {
	kfree(hello_data->proc_hello_value);
	kfree(hello_data);

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        // module exit message
        printk(KERN_ALERT "2470:8.1: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
