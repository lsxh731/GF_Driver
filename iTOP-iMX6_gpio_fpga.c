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

#define DEVICE_NAME				"gpiofpga_ctl"
#define DRIVER_NAME             "gpiofpga_ctl"

static int gpios[] = {
	
	/*****************added by nxb 20181206************************/
	IMX_GPIO_NR(2,23),// EIM_CS0
	IMX_GPIO_NR(2,24),// EIM_CS1
	IMX_GPIO_NR(2,25),// EIM_OE
	IMX_GPIO_NR(2,26),// EIM_WE
	
	IMX_GPIO_NR(3,0),// EIM_DA0
	IMX_GPIO_NR(3,1),// EIM_DA1
	IMX_GPIO_NR(3,2),// EIM_DA2
	IMX_GPIO_NR(3,3),// EIM_DA3
	
	IMX_GPIO_NR(3,4),// EIM_DA4
	IMX_GPIO_NR(3,5),// EIM_DA5
	IMX_GPIO_NR(3,6),// EIM_DA6
	IMX_GPIO_NR(3,7),// EIM_DA7
	
	IMX_GPIO_NR(3,16),// EIM_D16 12
	IMX_GPIO_NR(3,17),// EIM_D17
	IMX_GPIO_NR(3,18),// EIM_D18
	IMX_GPIO_NR(3,19),// EIM_D19
	
	IMX_GPIO_NR(3,20),// EIM_D20
	IMX_GPIO_NR(3,21),// EIM_D21
	IMX_GPIO_NR(3,22),// EIM_D22
	IMX_GPIO_NR(3,23),// EIM_D23
	
	IMX_GPIO_NR(3,24),// EIM_D24
	IMX_GPIO_NR(3,25),// EIM_D25
	IMX_GPIO_NR(3,26),// EIM_D26
	IMX_GPIO_NR(3,27),// EIM_D27
	
	IMX_GPIO_NR(3,28),// EIM_D28
	IMX_GPIO_NR(3,29),// EIM_D29
	IMX_GPIO_NR(3,30),// EIM_D30
	IMX_GPIO_NR(3,31),// EIM_D31
	
	/*****************************************/
};

static int itop6x_gpio_open(struct inode *inode, struct file *file) {
		return 0;
}

static int itop6x_gpio_close(struct inode *inode, struct file *file) {
	return 0;
}

/*static int itop6x_gpio_read(struct inode *inode, struct file *file) {
		int i,gpiovalue=0;
        //printk(KERN_EMERG " \n hello read! ");
		for(i=12; i<sizeof(gpios)/sizeof(gpios[0]); i++){
		gpio_direction_input(gpios[i]);
		int gpiovalue = gpio_get_value(gpios[i]);
		//printk("reading gpio[%d] = %d!\n",i,gpiovalue);
		}
		return 0;
		
}*/

static int itop6x_gpio_read(struct inode *inode, int *buffer_read) {
		int i;
        //printk(KERN_EMERG " \n hello read! ");
		for(i=12; i<sizeof(gpios)/sizeof(gpios[0]); i++){
			gpio_direction_input(gpios[i]);
			buffer_read[i-12] = gpio_get_value(gpios[i]);
		//printk("reading gpio[%d] = %d!\n",i,gpiovalue);
		}
		return 0;
		
}

static long itop6x_gpio_ioctl(struct file *filep, unsigned int cmd, unsigned long arg){
	
	//printk("writeing %d to %d\n", cmd, arg);
	switch(cmd) {
		case 0:
			if  (arg > 30 )
			{
				return -EINVAL;
			}
			gpio_direction_output(gpios[arg],0);
			gpio_set_value(gpios[arg], 0);
			break;
		case 1:
			if  (arg > 30)
			{
				return -EINVAL;
			}
			gpio_direction_output(gpios[arg],0);
			gpio_set_value(gpios[arg], 1);
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static struct file_operations itop6x_gpio_ops = {
	.owner			= THIS_MODULE,
	.open			= itop6x_gpio_open,
	.read			= itop6x_gpio_read,
	.release		= itop6x_gpio_close, 
	.unlocked_ioctl	= itop6x_gpio_ioctl,
	
};

static struct miscdevice itop6x_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &itop6x_gpio_ops,
};

static int itop6x_gpio_probe(struct platform_device *pdev)
{  
	int ret,i,gpiovalue=0;
	printk( "itop6x_gpio_probe!! \n");
	
	//write
	for(i=0; i<sizeof(gpios)/sizeof(gpios[0]); i++){
		gpio_free(gpios[i]);
		ret = gpio_request(gpios[i], "gpio");
		if (ret){
			printk("\n request GPIO %d failed!!! \n", gpios[i]);
			}
			gpio_direction_output(gpios[i],0);
			gpio_set_value(gpios[i],1);
			printk("gpio[%d] value is 1!\n",i);
	}
	
	//read 
	for(i=0; i<sizeof(gpios)/sizeof(gpios[0]); i++){
			gpio_free(gpios[i]);
			ret = gpio_request(gpios[i], "gpio");
			if (ret){
				printk("\n request GPIO %d failed!!! \n", gpios[i]);
				}
			gpio_direction_input(gpios[i]);	
			gpiovalue = gpio_get_value(gpios[i]);
			printk("gpio_ctl value is %d!\n",gpiovalue);
	}
	ret = misc_register(&itop6x_misc_dev);
	printk(DEVICE_NAME "\tinitialized\n");
	return 0;

}

static int itop6x_gpio_remove (struct platform_device *pdev)
{	
	int i;
	misc_deregister(&itop6x_misc_dev);
	for(i=0; i<sizeof(gpios)/sizeof(gpios[0]); i++){
		gpio_free(gpios[i]);	
	}
	printk( "itop6x_gpio_remove !! \n");
	return 0;
}

static int itop6x_gpio_suspend (struct platform_device *pdev, pm_message_t state)
{
	printk("gpio_ctl suspend:power off!\n");
	return 0;
}

static int itop6x_gpio_resume (struct platform_device *pdev)
{
	printk("gpio_ctl resume:power on!\n");
	return 0;
}

static struct platform_driver itop6x_gpio_driver = {
	.probe = itop6x_gpio_probe,
	.remove = itop6x_gpio_remove,
	.suspend = itop6x_gpio_suspend,
	.resume = itop6x_gpio_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init itop6x_gpio_init(void) {
	return platform_driver_register(&itop6x_gpio_driver);
	printk( "itop6x_gpio_init !! \n");
	
}

static void __exit itop6x_gpio_exit(void) {
	platform_driver_unregister(&itop6x_gpio_driver);
	printk( "itop6x_gpio_exit !! \n");
	}
	

module_init(itop6x_gpio_init);
module_exit(itop6x_gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TOPEET Inc.");
MODULE_DESCRIPTION("I.MX6 LED Driver");
