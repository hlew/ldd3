// Example 9.2a 	:  vga port

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>

#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/sched.h>
#include <linux/vmalloc.h>  // 

#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define PROC_HELLO_NUMENTRIES 20
#define PROC_HELLO_LEN 8
#define PROC_HELLO_BUFLEN 1024
#define HELLO "hello"
#define MYDEV "MYDEV"

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char *proc_hello_value;
};

static struct kmem_cache *hello_cache;
static char proc_hello_flag=0;

int order=2;

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
	int n=0;
	struct proc_hello_data *usrsp=hello_data;

	unsigned long long display=65;


	if (*eof) { *eof=0; n=0; }
	else if (proc_hello_flag) 
	{
		*eof = 1;

	// from /proc/ioports vga+=>03c0-03df
	    outl(0x3c0,display);

		proc_hello_flag=0;
		n=sprintf(buf, "Hello .. I got \"%s\": tt '%c'.\n", 
				usrsp->proc_hello_value,
				(int) display);

		vfree(usrsp->proc_hello_value);
		kmem_cache_free(hello_cache,hello_data);
	}
	else
	{
		*eof = 1;
		n=sprintf(buf,"Hello from process %d\njiffies=%ld\n", 
				(int)current->pid,jiffies);
	}

	wmb();
	
	return n;
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

	hello_data=kmem_cache_alloc(hello_cache,GFP_KERNEL);

	proc_hello_flag=0;

	// memory definition
	hello_data->proc_hello_value=(char *)vmalloc(PROC_HELLO_BUFLEN);

	length = (length<PROC_HELLO_LEN)? length:PROC_HELLO_LEN;

	if (copy_from_user(hello_data->proc_hello_value, buf, length))  {
		vfree(hello_data->proc_hello_value);
		kmem_cache_free(hello_cache,hello_data);
		return -EFAULT;
	}

	rmb();

	hello_data->proc_hello_value[length-1]=0;
	proc_hello_flag=1;
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

static int my_init (void) 
{
	int sz=sizeof(struct proc_hello_data)* PROC_HELLO_NUMENTRIES;

	hello_cache = kmem_cache_create("hello_cache",sz,0,0,NULL);

	if (!hello_cache)
		return -ENOMEM;

	proc_mydev = proc_mkdir(MYDEV,0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	if (proc_mydev)
  	proc_hello = proc_create(HELLO, 0777, proc_mydev, &proc_fops);
#else
  proc_hello = create_proc_entry(HELLO,0,proc_mydev);
  proc_hello->read_proc = read_hello;
  proc_hello->write_proc = write_hello;
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
  proc_hello->owner = THIS_MODULE;
#endif

  // module init message
  printk(KERN_ALERT "2470:9.2: main initialized!\n");
	return 0;
}

static void my_exit (void) {
	// free memory
	if (hello_cache)
		kmem_cache_destroy(hello_cache);

	// free proc entries here
	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        // module exit message
        printk(KERN_ALERT "2470:9.2: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
