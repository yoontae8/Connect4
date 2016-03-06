/*LED Kernel timer example
*/
#ifndef __LED_KERNEL_TIMER_DRIVER_
#define __LED_KERNEL_TIMER_DRIVER_

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h> /* kmalloc() */
#include <linux/timer.h> /* HZ */

#endif

#define DEV_NAME "con_led_timer"
#define IOM_LED_TIMER_MAJOR_NUM 282
#define IOM_LED_ADDRESS 0xA8000000
#define TIME_STEP (HZ/10) 
#define FLOW_MODE 1 


static int ledport_usage = 0;
static unsigned char *iom_led_addr;
static struct timer_list *ptimermgr = NULL;

static unsigned char led_data = 255;

unsigned char led_hex_data[] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};
int seq = 0;
int i = 1;

MODULE_LICENSE("GPL");

int init_led_timer(void);
void cleanup_led_timer(void);
module_init(init_led_timer);
module_exit(cleanup_led_timer);

void led_timer_timeover(unsigned long arg);
void led_timer_register(struct timer_list *pdata, unsigned long timeover);

int led_timer_open (struct inode *, struct file *);
int led_timer_release (struct inode *, struct file *);
ssize_t led_timer_write (struct file *, const char __user *, size_t, loff_t *);

struct file_operations led_timer_fops =  {
	.open = led_timer_open,
	.release = led_timer_release,
	.write = led_timer_write,
};

int __init init_led_timer(void)
{
	int result;
	//register_chrdev()
	result = register_chrdev(IOM_LED_TIMER_MAJOR_NUM, DEV_NAME, &led_timer_fops);
	if(result < 0)
	{
		printk(KERN_WARNING"%s : can't get or assign major number %d\n", DEV_NAME, IOM_LED_TIMER_MAJOR_NUM);
		return result;
	}
	printk("Success to load the device %s. Major number is %d\n", DEV_NAME, IOM_LED_TIMER_MAJOR_NUM);
	//ioremap()
	iom_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);

	//ptimermgr 선언
	ptimermgr = kmalloc(sizeof(struct timer_list), GFP_KERNEL);
	if(ptimermgr == NULL) return -ENOMEM;

	//init ptimermgr
	memset(ptimermgr, 0, sizeof(struct timer_list));

	//led_timer_register()
	led_timer_register(ptimermgr, TIME_STEP);
	
	return 0;
}


void led_timer_register(struct timer_list *pdata, unsigned long timeover)
{
	//init_timer()
	init_timer(pdata);
	pdata->expires = get_jiffies_64() + timeover;
	pdata->function = led_timer_timeover; //time over handler
	pdata->data = (unsigned long) pdata; //argument to the handler
	//add_timer()
	add_timer(pdata);
}

void led_timer_timeover(unsigned long arg)
{
	struct timer_list *pdata = NULL;

	pdata = (struct timer_list *) arg;
	if(led_data == FLOW_MODE)
	{
	    outb(led_hex_data[seq], (unsigned int) iom_led_addr);
	    seq += i;
	    if(seq == 7 || seq == 0)
		i *= -1;
	}else{
	    outb(led_data, (unsigned int) iom_led_addr);
	}
	//led_timer_register()
	led_timer_register(pdata, TIME_STEP);
}

int led_timer_open (struct inode *inode, struct file *filp)
{
	if(ledport_usage)	 return -EBUSY;
	ledport_usage = 1;
	return 0;
}

int led_timer_release (struct inode *inode, struct file *filp)
{
	ledport_usage = 0;
	return 0;
}

ssize_t led_timer_write (struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	if(copy_from_user(&led_data, buf, count)) return -EFAULT;
	return count;
}

void __exit cleanup_led_timer(void)
{
	//LED turn off
	outb(0xff, (unsigned int)iom_led_addr); //led turn off
	//del_timer()
	if(ptimermgr != NULL)
	{
		del_timer(ptimermgr);
		kfree(ptimermgr);
	}
	iounmap(iom_led_addr);
	unregister_chrdev(IOM_LED_TIMER_MAJOR_NUM, DEV_NAME);
	printk("Success to unload the device %s...\n", DEV_NAME);
}

