#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#define DRIVER_NAME    "iotest"

#define KEY_COL6	   IMX_GPIO_NR(1,9)    //GPIO_9
#define KEY_DOWN       IMX_GPIO_NR(7,13)   //GPIO_18

struct fasync_struct *pwm1_fasync;//定义fasync_struct结构
int pt_count,p_irq; 
struct mutex irq_lock;
 

static irqreturn_t eint2_interrupt(int irq,void *dev_id)
{
	int ret;
	mutex_lock(&irq_lock);
	//pt_count++;
	//if(pt_count > 40000)
	//{
	//	printk("#####receive a interrupt 1!#####\n");
		//pt_count = 0;
	//}
	//kill_fasync(&pwm1_fasync, SIGIO, POLL_IN);
	printk("#####receive a interrupt 2!#####\n");
	ret = gpio_get_value(KEY_DOWN);
	printk("####gpio value is = %d\n",ret);
	mutex_unlock(&irq_lock);
	return IRQ_RETVAL(IRQ_HANDLED);
}


static int myeint1_fasync(int fd, struct file * file, int on)
{
    int err;
 //   printk("fansync_helper\n");
    err = fasync_helper(fd, file, on, &pwm1_fasync);  //利用fasync_helper初始化pwm1_fasync
    if (err < 0)
        return err;
    return 0;
}


ssize_t myeint1_write(struct file *f, const char __user *buf, size_t t, loff_t *len){
	

}
static const struct file_operations myeint1_fops =
{
	.owner   	=   THIS_MODULE,
	.write		=   myeint1_write,
	.fasync     =   myeint1_fasync,
};
 
struct miscdevice myeint1_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "iotest",
	.fops = &myeint1_fops,
};

static int myeint1_probe(struct platform_device *pdev)
{
	
	
	int ret,irq;
	pt_count=0;
	mutex_init(&irq_lock);
	ret = gpio_request(KEY_DOWN, "KEY_DOWN");
    if (ret) {
            printk("request GPIO %d for EINT1 failed\n",KEY_DOWN);
            return ret;
    }
	gpio_direction_input(KEY_DOWN);
	gpio_free(KEY_DOWN);    
	
	irq = gpio_to_irq(KEY_DOWN);
	p_irq = irq;
	request_irq(irq,eint2_interrupt,IRQ_TYPE_EDGE_FALLING,"KEY_DOWN",pdev);
	ret = misc_register(&myeint1_dev);
	printk("myeint1_probe\n");
	
	return 0;
	
}

static int myeint1_remove (struct platform_device *pdev)
{
	printk("myeint1_ctl remove:power off!\n");
	misc_deregister(&myeint1_dev);
	free_irq(p_irq,pdev);
	return 0;
}

static int myeint1_suspend (struct platform_device *pdev, pm_message_t state)
{
	printk("myeint1_ctl suspend:power off!\n");
	return 0;
}

static int myeint1_resume (struct platform_device *pdev)
{
	printk("myeint1_ctl resume:power on!\n");

	return 0;
}

static struct platform_driver myeint1_driver = {
	.probe = myeint1_probe,
	.remove = myeint1_remove,
	.suspend = myeint1_suspend,
	.resume = myeint1_resume,
	.driver = {
		.name = "iotest",
		.owner = THIS_MODULE,
	},
};

static int __init myeint1_init(void) {
	return platform_driver_register(&myeint1_driver);
}

static void __exit myeint1_exit(void) {
	platform_driver_unregister(&myeint1_driver);
} 
 
 
 
module_init(myeint1_init);  
module_exit(myeint1_exit);
MODULE_AUTHOR("hrtimer<hrtimer@hrtimer.cc>");
MODULE_LICENSE("GPL");
