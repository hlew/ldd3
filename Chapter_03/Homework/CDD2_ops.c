#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>		// 2.6
#include <linux/device.h>

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
#include <asm/uaccess.h>
#else
#include <linux/uaccess.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,0,1)
#include <linux/vmalloc.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <linux/moduleparam.h>
#endif

#include "CDD2.h"

extern struct CDDdev_struct myCDD;
extern struct file_operations CDD_fops;

int CDD_open (struct inode *inode, struct file *file)
{

	struct CDDdev_struct *thisCDD=
   	container_of(inode->i_cdev, struct CDDdev_struct, cdev);

	if ( file->f_flags & O_TRUNC )  
	{
		printk(KERN_ALERT "file '%s' opened O_TRUNC\n",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
			file->f_path.dentry->d_name.name);
#else
			file->f_dentry->d_name.name);
#endif
    thisCDD->CDD_storage[0]=0;
    thisCDD->counter=0;
 	}

  if ( file->f_flags & O_APPEND )  
	{
		printk(KERN_ALERT "file '%s' opened O_APPEND\n",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
			file->f_path.dentry->d_name.name);
#else
			file->f_dentry->d_name.name);
#endif
  }

  file->private_data=thisCDD;

	return 0;
}

int CDD_release (struct inode *inode, struct file *file)
{

 	struct CDDdev_struct *thisCDD=file->private_data;

	if( thisCDD->counter <= 0 ) return 0; // overcome compiler warning msg.

	// MOD_DEC_USE_COUNT;

	return 0;
}

ssize_t CDD_read (struct file *file, char *buf, 
size_t count, loff_t *ppos)
{

	int err;
 	struct CDDdev_struct *thisCDD=file->private_data;

	if( thisCDD->counter <= 0 ) return 0;

  if( *ppos >= thisCDD->counter) return 0;
  else if( *ppos + count >= thisCDD->counter)
    count = thisCDD->counter - *ppos;

  if( count <= 0 ) return 0;    
	printk(KERN_ALERT "CDD_read: count=%d\n", count);

	// bzero(buf,64);  // a bogus 64byte initialization
	//memset(buf,0,64);  // a bogus 64byte initialization
	err = copy_to_user(buf,&(thisCDD->CDD_storage[*ppos]),count);
	    if (err != 0) return -EFAULT;
	
	//buf[count]=0;
	*ppos += count;
	return count;
	
}

ssize_t CDD_write (struct file *file, const char *buf, 
size_t count, loff_t *ppos)
{

	int err;
	struct CDDdev_struct *thisCDD=file->private_data;

	err = copy_from_user(&(thisCDD->CDD_storage[*ppos]),buf	,count);
	
	printk(KERN_ALERT "written from userspace app %s\n", &(thisCDD->CDD_storage[*ppos]));
	if (err != 0) return -EFAULT;

	thisCDD->counter += count;
	*ppos+=count;
	return count;

}
loff_t CDD_llseek (struct file *file, loff_t newpos, int whence)
{

  int pos;
	struct CDDdev_struct *thisCDD=file->private_data;

	switch(whence) {
		case SEEK_SET:        // CDDoffset can be 0 or +ve
			pos=newpos;
			break;
		case SEEK_CUR:        // CDDoffset can be 0 or +ve
			pos=(file->f_pos + newpos);
			break;
		case SEEK_END:        // CDDoffset can be 0 or +ve
			pos=(thisCDD->counter + newpos);
			break;
		default:
			return -EINVAL;
	}
	if ((pos < 0)||(pos>thisCDD->counter)) 
		return -EINVAL;
				
	file->f_pos = pos;
	return pos;

}


