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

#define DRIVER_NAME    "errinput"

#define KEY_COL6	   IMX_GPIO_NR(1,9)    //GPIO_9

struct fasync_struct *Err_fasync;//定义fasync_struct结构
int pt_count,p_irq; 
struct mutex irq_lock;
static int irqs[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};
static int gpios[] = {
		IMX_GPIO_NR(3,0),//EIM_DA0
		IMX_GPIO_NR(3,1),//EIM_DA1
		IMX_GPIO_NR(3,2),//EIM_DA2
		IMX_GPIO_NR(3,3),//EIM_DA3
		IMX_GPIO_NR(3,4),//EIM_DA4
		IMX_GPIO_NR(3,5),//EIM_DA5
		IMX_GPIO_NR(3,6),//EIM_DA6
		IMX_GPIO_NR(3,7),//EIM_DA7
		IMX_GPIO_NR(3,8),//EIM_DA8
		IMX_GPIO_NR(3,9),//EIM_DA9
		IMX_GPIO_NR(3,10),//EIM_DA10
		IMX_GPIO_NR(3,11),//EIM_DA11
		IMX_GPIO_NR(3,12),//EIM_DA12
		IMX_GPIO_NR(3,13),//EIM_DA13
		IMX_GPIO_NR(1,9),  //KEY_COL6
};
 

static irqreturn_t ErrInput_interrupt(int irq,void *dev_id)
{
	int ret,i;
	mutex_lock(&irq_lock);
	//pt_count++;
	//if(pt_count > 40000)
	//{
	//	printk("#####receive a interrupt 1!#####\n");
		//pt_count = 0;
	//}
	printk("#####receive a interrupt 1!#####\n");
	p_irq = -1;
	for(i=0; i<sizeof(irqs)/sizeof(irqs[0]); i++){
		if(irq == irqs[i])
		{
			p_irq = i;
			i = 15;
		}
	}
	printk("#####interrupt id is = %d #####\n",p_irq);
	kill_fasync(&Err_fasync, SIGIO, POLL_IN);
	mutex_unlock(&irq_lock);
	return IRQ_RETVAL(IRQ_HANDLED);
}


static int ErrInput_fasync(int fd, struct file * file, int on)
{
    int err;
 //   printk("fansync_helper\n");
    err = fasync_helper(fd, file, on, &Err_fasync);  //利用fasync_helper初始化ErrInput_fasync
    if (err < 0)
        return err;
    return 0;
}


ssize_t ErrInput_write(struct file *f, const char __user *buf, size_t t, loff_t *len){
	

}

ssize_t ErrInput_read(struct file *f, const char __user *buf, size_t t, loff_t *len){
	
	copy_to_user(buf,&p_irq,1);
	printk("ErrInput_read successed,p_irq = %d\n",p_irq);
	return 0;
}
static const struct file_operations ErrInput_fops =
{
	.owner   	=   THIS_MODULE,
	.write		=   ErrInput_write,
	.fasync     =   ErrInput_fasync,
	.read		= 	ErrInput_read,
};
 
struct miscdevice ErrInput_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "errinput",
	.fops = &ErrInput_fops,
};

static int ErrInput_probe(struct platform_device *pdev)
{
	
	
	int i,ret,irq;
	pt_count=0;
	mutex_init(&irq_lock);
	
	for(i=0; i<sizeof(gpios)/sizeof(gpios[0]); i++){
		gpio_free(gpios[i]);
		ret = gpio_request(gpios[i], "gpio");
		if (ret){
			printk("\n request GPIO %d failed!!! \n", gpios[i]);
			}
			gpio_direction_input(gpios[i]);
			gpio_free(gpios[i]);
	}
	/*
	ret = gpio_request(KEY_COL6, "key_col6");
    if (ret) {
            printk("request GPIO %d for EINT1 failed, ret = %d\n",KEY_COL6, ret);
            return ret;
    }
	gpio_direction_input(KEY_COL6);
	gpio_free(KEY_COL6);
	
	*/
	for(i=0; i<sizeof(irqs)/sizeof(irqs[0]); i++){
		irqs[i] = gpio_to_irq(gpios[i]);
		request_irq(irqs[i],ErrInput_interrupt,IRQ_TYPE_EDGE_FALLING,"ErrInput_interrupt",pdev);
	}
	ret = misc_register(&ErrInput_dev);
	
	return 0;
	
}

static int ErrInput_remove (struct platform_device *pdev)
{
	int i;
	printk("ErrInput_ctl remove:power off!\n");
	misc_deregister(&ErrInput_dev);
	for(i=0; i<sizeof(irqs)/sizeof(irqs[0]); i++){
		free_irq(irqs[i],pdev);
	}
	return 0;
}

static int ErrInput_suspend (struct platform_device *pdev, pm_message_t state)
{
	printk("ErrInput_ctl suspend:power off!\n");
	return 0;
}

static int ErrInput_resume (struct platform_device *pdev)
{
	printk("ErrInput_ctl resume:power on!\n");

	return 0;
}

static struct platform_driver ErrInput_driver = {
	.probe = ErrInput_probe,
	.remove = ErrInput_remove,
	.suspend = ErrInput_suspend,
	.resume = ErrInput_resume,
	.driver = {
		.name = "errinput",
		.owner = THIS_MODULE,
	},
};

static int __init ErrInput_init(void) {
	return platform_driver_register(&ErrInput_driver);
}

static void __exit ErrInput_exit(void) {
	platform_driver_unregister(&ErrInput_driver);
} 
 
 
 
module_init(ErrInput_init);  
module_exit(ErrInput_exit);
MODULE_AUTHOR("hrtimer<hrtimer@hrtimer.cc>");
MODULE_LICENSE("GPL");
