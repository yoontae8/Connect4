/* Device driver for push button switches */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h> /*copy_to_user()*/
#include <asm/io.h> /*inb*/

#define DEV_NAME "con_push"
#define IOM_PUSH_MAJOR_NUM  280
#define IOM_PUSH_ADDRESS    0xA8000400 /* A800_0400 */

MODULE_LICENSE("GPL");

int iom_push_init(void);
void iom_push_exit(void);
module_init(iom_push_init);
module_exit(iom_push_exit);

static int push_usage = 0;
static unsigned char *iom_push_addr;

int iom_push_open (struct inode *, struct file *);
int iom_push_release (struct inode *, struct file *);
ssize_t iom_push_read (struct file *, char __user *, size_t, loff_t *);

struct file_operations iom_push_fops={
	.open = iom_push_open,
	.release = iom_push_release,
	.read = iom_push_read,
};


int iom_push_open (struct inode *inode, struct file *filep)
{
    if (push_usage)
	return -EBUSY;
    push_usage = 1;

    return 0;
}
int iom_push_release (struct inode *inode, struct file *filep)
{
    push_usage = 0 ;
    return 0;
}
ssize_t iom_push_read (struct file *filep, char __user *buf, size_t count, loff_t *f_pos)
{
    unsigned char pushsw_value;

    pushsw_value = inb((unsigned int)iom_push_addr);
    
    if(copy_to_user(buf, &pushsw_value, count))
	return -EFAULT;

    return count;
}

int __init iom_push_init(void)
{ 
    int result;
    result = register_chrdev(IOM_PUSH_MAJOR_NUM, DEV_NAME, &iom_push_fops);

    if (result < 0) {
	printk(KERN_WARNING"%s: can't get or assign major number %d\n", DEV_NAME, IOM_PUSH_MAJOR_NUM);
	return result;
    }

    iom_push_addr = ioremap(IOM_PUSH_ADDRESS, 0x1);

    printk("Success to load the device %s. Major number is %d\n", DEV_NAME, IOM_PUSH_MAJOR_NUM);

    return 0;
}
void __exit iom_push_exit(void)
{
    iounmap(iom_push_addr);

    unregister_chrdev(IOM_PUSH_MAJOR_NUM, DEV_NAME); 
    printk("Success to unload the device %s...\n", DEV_NAME); 
}
