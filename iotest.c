/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
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

#define DEVICE_NAME				"iotest"
#define DRIVER_NAME             "iotest"


//#define IOTEST_GPIO				IMX_GPIO_NR(1, 2)
//EIM_DA0~DA15 16pin
#define EIM_DA0				    IMX_GPIO_NR(3,0)
#define EIM_DA1				    IMX_GPIO_NR(3,1)
#define EIM_DA2				    IMX_GPIO_NR(3,2)
#define EIM_DA3				    IMX_GPIO_NR(3,3)
#define EIM_DA4				    IMX_GPIO_NR(3,4)
#define EIM_DA5				    IMX_GPIO_NR(3,5)
#define EIM_DA6				    IMX_GPIO_NR(3,6)
#define EIM_DA7				    IMX_GPIO_NR(3,7)
#define EIM_DA8				    IMX_GPIO_NR(3,8)
#define EIM_DA9				    IMX_GPIO_NR(3,9)
#define EIM_DA10			    IMX_GPIO_NR(3,10)
#define EIM_DA11			    IMX_GPIO_NR(3,11)
#define EIM_DA12			    IMX_GPIO_NR(3,12)
#define EIM_DA13			    IMX_GPIO_NR(3,13)
#define EIM_DA14			    IMX_GPIO_NR(3,14)
#define EIM_DA15				IMX_GPIO_NR(3,15)
//EIM_D16~D31 16pin
#define EIM_D16				    IMX_GPIO_NR(3,16)
#define EIM_D17				    IMX_GPIO_NR(3,17)
#define EIM_D18				    IMX_GPIO_NR(3,18)
#define EIM_D19				    IMX_GPIO_NR(3,19)
#define EIM_D20				    IMX_GPIO_NR(3,20)
#define EIM_D21				    IMX_GPIO_NR(3,21)
#define EIM_D22				    IMX_GPIO_NR(3,22)
#define EIM_D23				    IMX_GPIO_NR(3,23)
#define EIM_D24				    IMX_GPIO_NR(3,24)
#define EIM_D25				    IMX_GPIO_NR(3,25)
#define EIM_D26				    IMX_GPIO_NR(3,26)
#define EIM_D27				    IMX_GPIO_NR(3,27)
#define EIM_D28				    IMX_GPIO_NR(3,28)
#define EIM_D29				    IMX_GPIO_NR(3,29)
#define EIM_D30				    IMX_GPIO_NR(3,30)
#define EIM_D31				    IMX_GPIO_NR(3,31)
//EIM_A16~A24 9pin
#define EIM_A16				    IMX_GPIO_NR(2,22)
#define EIM_A17				    IMX_GPIO_NR(2,21)
#define EIM_A18				    IMX_GPIO_NR(2,20)
#define EIM_A19				    IMX_GPIO_NR(2,19)
#define EIM_A20				    IMX_GPIO_NR(2,18)
#define EIM_A21				    IMX_GPIO_NR(2,17)
#define EIM_A22				    IMX_GPIO_NR(2,16)
#define EIM_A23				    IMX_GPIO_NR(6,6)
#define EIM_A24				    IMX_GPIO_NR(5,4)
#define EIM_LBA				    IMX_GPIO_NR(2,27)
//DISP0_DAT4~DAT9&CSI0_DAT12~DAT19 14pin
#define DISP0_DAT4				IMX_GPIO_NR(4,25)
#define DISP0_DAT5				IMX_GPIO_NR(4,26)
#define DISP0_DAT6				IMX_GPIO_NR(4,27)
#define DISP0_DAT7				IMX_GPIO_NR(4,28)
#define DISP0_DAT8				IMX_GPIO_NR(4,29)
#define DISP0_DAT9				IMX_GPIO_NR(4,30)
#define CSI0_DAT12				IMX_GPIO_NR(5,30)
#define CSI0_DAT13				IMX_GPIO_NR(5,31)
#define CSI0_DAT14				IMX_GPIO_NR(6,0)
#define CSI0_DAT15				IMX_GPIO_NR(6,1)
#define CSI0_DAT16				IMX_GPIO_NR(6,2)
#define CSI0_DAT17				IMX_GPIO_NR(6,3)
#define CSI0_DAT18				IMX_GPIO_NR(6,4)
#define CSI0_DAT19				IMX_GPIO_NR(6,5)

//54 pin
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
		IMX_GPIO_NR(3,14),//EIM_DA14
		IMX_GPIO_NR(3,15),//EIM_DA15
		//EIM_D16~D31 16pin
		IMX_GPIO_NR(3,16),//EIM_D16
		IMX_GPIO_NR(3,17),//EIM_D17
		IMX_GPIO_NR(3,18),//EIM_D18
		IMX_GPIO_NR(3,19),//EIM_D19
		IMX_GPIO_NR(3,20),//EIM_D20
		IMX_GPIO_NR(3,21),//EIM_D21
		IMX_GPIO_NR(3,22),//EIM_D22
		IMX_GPIO_NR(3,23),//EIM_D23
		IMX_GPIO_NR(3,24),//EIM_D24
		IMX_GPIO_NR(3,25),//EIM_D25
		IMX_GPIO_NR(3,26),//EIM_D26
		IMX_GPIO_NR(3,27),//EIM_D27
		IMX_GPIO_NR(3,28),//EIM_D28
		IMX_GPIO_NR(3,29),//EIM_D29
		IMX_GPIO_NR(3,30),//EIM_D30
		IMX_GPIO_NR(3,31),//EIM_D31
		//EIM_A16~A24 9pin
		IMX_GPIO_NR(2,22),//EIM_A16
		IMX_GPIO_NR(2,21),//EIM_A17
		IMX_GPIO_NR(2,20),//EIM_A18
		IMX_GPIO_NR(2,19),//EIM_A19
		IMX_GPIO_NR(2,18),//EIM_A20
		IMX_GPIO_NR(2,17),//EIM_A21
		IMX_GPIO_NR(2,16),//EIM_A22
		IMX_GPIO_NR(6,6),//EIM_A23
		IMX_GPIO_NR(5,4),//EIM_A24
		//DISP0_DAT0~DAT5&CSI0_DAT12~DAT19 14pin
		IMX_GPIO_NR(4,27),//DISP0_DAT6
		IMX_GPIO_NR(4,28),//DISP0_DAT7
		IMX_GPIO_NR(4,29),//DISP0_DAT8
		IMX_GPIO_NR(4,30),//DISP0_DAT9
		IMX_GPIO_NR(4,25),//DISP0_DAT4
		IMX_GPIO_NR(4,26),//DISP0_DAT5
		IMX_GPIO_NR(5,30),//CSI0_DAT12
		IMX_GPIO_NR(5,31),//CSI0_DAT13
		IMX_GPIO_NR(6,0),//CSI0_DAT14
		IMX_GPIO_NR(6,1),//CSI0_DAT15
		IMX_GPIO_NR(6,2),//CSI0_DAT16
		IMX_GPIO_NR(6,3),//CSI0_DAT17
		IMX_GPIO_NR(6,4),//CSI0_DAT18
		IMX_GPIO_NR(6,5),//CSI0_DAT19
		IMX_GPIO_NR(2,27),//EIM_LBA
	
};

static int itop6x_iotest_open(struct inode *inode, struct file *file) {
		return 0;
}

static int itop6x_iotest_close(struct inode *inode, struct file *file) {
	return 0;
}

static long itop6x_iotest_ioctl(struct file *filep, unsigned int cmd,
		unsigned long arg)
{
	printk("%s: cmd = %d\n", __FUNCTION__, cmd);
	switch(cmd) {
		case 0:
			gpio_set_value(EIM_D17, 0);
			break;
		case 1:
			gpio_set_value(EIM_D17, 1);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static ssize_t itop6x_iotest_write(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
	char str[20];

	memset(str, 0, 20);

	if(copy_from_user(str, buffer, count))
	{
		printk("Error\n");

		return -EINVAL;
	}

	printk("%s", str);
#if 1
	if(!strncmp(str, "1", 1))
		gpio_set_value(EIM_D17, 1);
	else
		gpio_set_value(EIM_D17, 0);
#endif
	return count;
}

static struct file_operations itop6x_iotest_ops = {
	.owner			= THIS_MODULE,
	.open			= itop6x_iotest_open,
	.release		= itop6x_iotest_close, 
	.unlocked_ioctl	= itop6x_iotest_ioctl,
	.write			= itop6x_iotest_write,
};

static struct miscdevice itop6x_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &itop6x_iotest_ops,
};

static int itop6x_iotest_probe(struct platform_device *pdev)
{
	int ret,i;
	
	for(i=0; i<sizeof(gpios)/sizeof(gpios[0]); i++){
		gpio_free(gpios[i]);
		ret = gpio_request(gpios[i], "gpio");
		if (ret){
			printk("\n request GPIO %d failed!!! \n", gpios[i]);
			}
			gpio_direction_output(gpios[i],0);
			gpio_set_value(gpios[i],1);
			ret = gpio_get_value(gpios[i]);
			printk("####gpio[%d] value is = %d\n", i, ret);
	}
/*
	gpio_free(EIM_D17);
	ret = gpio_request(EIM_D17, DEVICE_NAME);
	if (ret) {
		printk("request GPIO %d for pwm failed\n", EIM_D17);
		return ret;
	}
	
	gpio_free(CSI0_DAT17);
	ret = gpio_request(CSI0_DAT17, DEVICE_NAME);
	if (ret) {
		printk("request GPIO %d for pwm failed\n", CSI0_DAT17);
		return ret;
	}
	
	gpio_direction_output(EIM_D17, 0);
	gpio_direction_output(CSI0_DAT17, 0);
*/	
	gpio_set_value(EIM_D17, 1);
	ret = gpio_get_value(EIM_D17);
	printk("####EIM_D17 value = %d\n", ret);
	gpio_set_value(EIM_D17, 0);
	ret = gpio_get_value(EIM_D17);
	printk("####EIM_D17 value = %d\n", ret);
	
	gpio_set_value(CSI0_DAT17, 1);
	ret = gpio_get_value(CSI0_DAT17);
	printk("####CSI0_DAT17 value = %d\n", ret);
	gpio_set_value(CSI0_DAT17, 0);
	ret = gpio_get_value(CSI0_DAT17);
	printk("####CSI0_DAT17 value = %d\n", ret);
	
	gpio_set_value(EIM_D17, 1);
	ret = gpio_get_value(EIM_D17);
	printk("####EIM_D17 value = %d\n", ret);
	
	gpio_set_value(CSI0_DAT17, 1);
	ret = gpio_get_value(CSI0_DAT17);
	printk("####CSI0_DAT17 value = %d\n", ret);
	
	

	ret = misc_register(&itop6x_misc_dev);

	printk(DEVICE_NAME "#### iotest ### tinitialized\n");

	return 0;
}

static int itop6x_iotest_remove (struct platform_device *pdev)
{
	misc_deregister(&itop6x_misc_dev);
	gpio_free(EIM_D17);	

	return 0;
}

static int itop6x_iotest_suspend (struct platform_device *pdev, pm_message_t state)
{
	printk("iotest_ctl suspend:power off!\n");
	return 0;
}

static int itop6x_iotest_resume (struct platform_device *pdev)
{
	printk("iotest_ctl resume:power on!\n");
	return 0;
}

static struct platform_driver itop6x_iotest_driver = {
	.probe = itop6x_iotest_probe,
	.remove = itop6x_iotest_remove,
	.suspend = itop6x_iotest_suspend,
	.resume = itop6x_iotest_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init itop6x_iotest_dev_init(void) {
	return platform_driver_register(&itop6x_iotest_driver);
}

static void __exit itop6x_iotest_dev_exit(void) {
	platform_driver_unregister(&itop6x_iotest_driver);
}

module_init(itop6x_iotest_dev_init);
module_exit(itop6x_iotest_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TOPEET Inc.");
MODULE_DESCRIPTION("I.MX6 IOTEST Driver");
