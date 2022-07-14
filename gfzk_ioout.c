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

#define DEVICE_NAME				"ioout"
#define DRIVER_NAME             "ioout"

#define EIM_DA15_COOLING				IMX_GPIO_NR(3,15) //COOLING				0
#define EIM_D16_FIELD				    IMX_GPIO_NR(3,16) //FIELD  				1
#define EIM_D17_FAN				        IMX_GPIO_NR(3,17) //FAN					2
#define EIM_D18_ST				        IMX_GPIO_NR(3,18) //ST					3
#define EIM_D20_EXT21				    IMX_GPIO_NR(3,20) //EXT21				4
#define EIM_D21_HB				        IMX_GPIO_NR(3,21) //HB					5
#define EIM_D24_MUX_A0				    IMX_GPIO_NR(3,24) //MUX_A0				6
#define EIM_D25_MUX_A1				    IMX_GPIO_NR(3,25) //MUX_A1				7
#define EIM_D26_MUX_A2				    IMX_GPIO_NR(3,26) //MUX_A2				8
#define EIM_D27_MUX_A3			        IMX_GPIO_NR(3,27) //MUX_A3				9
#define CSI0_DAT14_CUS11EN		        IMX_GPIO_NR(6,0)  //Custom11_nEN		10
#define CSI0_DAT15_1K0				    IMX_GPIO_NR(6,1)  //1K0					11
#define CSI0_DAT16_1K1				    IMX_GPIO_NR(6,2)  //1K1					12
#define CSI0_DAT17_2K0				    IMX_GPIO_NR(6,3)  //2K0					13
#define DISP0_DAT6_2K1				    IMX_GPIO_NR(4,27) //2K1					14
#define DISP0_DAT7_3K0				    IMX_GPIO_NR(4,28) //3K0					15
#define DISP0_DAT8_3K1				    IMX_GPIO_NR(4,29) //3K1					16
#define DISP0_DAT9_4K0					IMX_GPIO_NR(4,30) //4K0					17
#define DISP0_DAT4_4K1					IMX_GPIO_NR(4,25) //4K1					18
#define CSI0_DAT18_RUN					IMX_GPIO_NR(6,4)  //RUN					20
#define CSI0_DAT19_CUS12EN			    IMX_GPIO_NR(6,5)  //Custom12_nEN		21

//22 pin

static int gpios[] = {
		IMX_GPIO_NR(3,15),//EIM_DA15_COOLING
		IMX_GPIO_NR(3,16),//EIM_D16_FIELD
		IMX_GPIO_NR(3,17),//EIM_D17_FAN
		IMX_GPIO_NR(3,21),//EIM_D21_HB
		IMX_GPIO_NR(3,24),//EIM_D24_MUX_A0
		IMX_GPIO_NR(3,25),//EIM_D25_MUX_A1
		IMX_GPIO_NR(3,26),//EIM_D26_MUX_A2
		IMX_GPIO_NR(3,27),//EIM_D27_MUX_A3
		IMX_GPIO_NR(6,1), //CSI0_DAT15_1K0
		IMX_GPIO_NR(6,2), //CSI0_DAT16_1K1
		IMX_GPIO_NR(6,3), //CSI0_DAT17_2K0
		IMX_GPIO_NR(4,27),//DISP0_DAT6_2K1
		IMX_GPIO_NR(4,28),//DISP0_DAT7_3K0
		IMX_GPIO_NR(4,29),//DISP0_DAT8_3K1
		IMX_GPIO_NR(4,30),//DISP0_DAT9_4K0
		IMX_GPIO_NR(4,25),//DISP0_DAT4_4K1
		IMX_GPIO_NR(6,4), //CSI0_DAT18_RUN
		IMX_GPIO_NR(6,5), //CSI0_DAT19_Custom12_nEN
		IMX_GPIO_NR(3,23),//EIM_D23_AD760601_CA
		IMX_GPIO_NR(3,18),//EIM_D18_ST      ad760602 test using
		IMX_GPIO_NR(3,20),//EIM_D20_EXT21   ad760602 test using
};


static int itop6x_ioout_open(struct inode *inode, struct file *file) {
		return 0;
}

static int itop6x_ioout_close(struct inode *inode, struct file *file) {
	return 0;
}

static long itop6x_ioout_ioctl(struct file *filep, unsigned int cmd,
		unsigned long arg)
{
	int ret;
	//printk("%s: arg = %d cmd = %d\n", __FUNCTION__, arg, cmd);
	switch(cmd) {
		case 0:
			gpio_set_value(gpios[arg], 0);
			break;
		case 1:
			gpio_set_value(gpios[arg], 1);
			break;
		default:
			return -EINVAL;
	}
	//ret = gpio_get_value(gpios[arg]);
	//printk("####gpios[%d] = %d###\n",arg,ret);
	return 0;
}

static ssize_t itop6x_ioout_write(struct file *filp, char *buffer, size_t count, loff_t *ppos)
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
		gpio_set_value(EIM_D17_FAN, 1);
	else
		gpio_set_value(EIM_D17_FAN, 0);
#endif
	return count;
}

static struct file_operations itop6x_ioout_ops = {
	.owner			= THIS_MODULE,
	.open			= itop6x_ioout_open,
	.release		= itop6x_ioout_close, 
	.unlocked_ioctl	= itop6x_ioout_ioctl,
	.write			= itop6x_ioout_write,
};

static struct miscdevice itop6x_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &itop6x_ioout_ops,
};

static int itop6x_ioout_probe(struct platform_device *pdev)
{
	int ret,i;
	
	for(i=0; i<sizeof(gpios)/sizeof(gpios[0]); i++){
		gpio_free(gpios[i]);
		ret = gpio_request(gpios[i], "gpio");
		if (ret){
			printk("\n request GPIO %d failed!!! \n", gpios[i]);
			}
			gpio_direction_output(gpios[i],0);
			//gpio_set_value(gpios[i],0);
			//ret = gpio_get_value(gpios[i]);
			//printk("####gpio[%d] value is = %d\n", i, ret);
	}
	/*
	gpio_set_value(EIM_D17_FAN, 1);
	ret = gpio_get_value(EIM_D17_FAN);
	printk("####EIM_D17_FAN value = %d\n", ret);
	gpio_set_value(EIM_D17_FAN, 0);
	ret = gpio_get_value(EIM_D17_FAN);
	printk("####EIM_D17_FAN value = %d\n", ret);
	
	gpio_set_value(DISP0_DAT8_3K1, 1);
	ret = gpio_get_value(DISP0_DAT8_3K1);
	printk("####DISP0_DAT8_3K1 value = %d\n", ret);
	gpio_set_value(DISP0_DAT8_3K1, 0);
	ret = gpio_get_value(DISP0_DAT8_3K1);
	printk("####DISP0_DAT8_3K1 value = %d\n", ret);
	
	gpio_set_value(EIM_D17_FAN, 1);
	ret = gpio_get_value(EIM_D17_FAN);
	printk("####EIM_D17_FAN value = %d\n", ret);
	
	gpio_set_value(DISP0_DAT8_3K1, 1);
	ret = gpio_get_value(DISP0_DAT8_3K1);
	printk("####DISP0_DAT8_3K1 value = %d\n", ret);
	*/
	

	ret = misc_register(&itop6x_misc_dev);

	printk(DEVICE_NAME "#### ioout ### tinitialized\n");

	return 0;
}

static int itop6x_ioout_remove (struct platform_device *pdev)
{
	int i;
	
	for(i=0; i<sizeof(gpios)/sizeof(gpios[0]); i++)
		gpio_free(gpios[i]);
	misc_deregister(&itop6x_misc_dev);
	printk(DEVICE_NAME "#### ioout ### remove\n");

	return 0;
}


static struct platform_driver itop6x_ioout_driver = {
	.probe = itop6x_ioout_probe,
	.remove = itop6x_ioout_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init itop6x_ioout_init(void) {
	return platform_driver_register(&itop6x_ioout_driver);
}

static void __exit itop6x_ioout_exit(void) {
	platform_driver_unregister(&itop6x_ioout_driver);
}

module_init(itop6x_ioout_init);
module_exit(itop6x_ioout_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TOPEET Inc.");
MODULE_DESCRIPTION("I.MX6 ioout Driver");
