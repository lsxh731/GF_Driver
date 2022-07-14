#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
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
#include <linux/pwm.h>
#include <linux/fs.h> 
#include <linux/io.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/init.h>

/*寄存器头文件*/
#include <mach/iomux-mx6q.h>
#define BUZZER_GPIO			IMX_GPIO_NR(1, 19)

#define PWM_FREQENCY 1

struct pwm_device *my_pwm1 = NULL;

#define DEVICE_NAME				"my_pwm_test"

#define DRIVER_NAME 			"my_pwm_test"

static int itop6x_pwm_test_open(struct inode *inode, struct file *file) 
{
   printk(" %s  ! ! !\n",__FUNCTION__);
   mxc_iomux_v3_setup_pad(MX6Q_PAD_SD1_DAT2__PWM2_PWMO);
   my_pwm1 = pwm_request(1, "my_pwm1");
   if ( my_pwm1 == NULL ) {
       printk(KERN_ALERT"my_pwm1 open error.\n");
   }
   return 0;
}

static long itop6x_pwm_test_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)

{
	printk(" %s  ! ! !\n",__FUNCTION__);
	{
   if(my_pwm1 == NULL)
       return -EINVAL;
   if(arg < 0)
       return -EINVAL;
   switch (cmd) {
	   
   case PWM_FREQENCY:
       if(arg==0)
        {
           pwm_disable(my_pwm1);
        }
       else
       {
           pwm_config(my_pwm1, 1000000000/arg/2, 1000000000/arg);
           pwm_enable(my_pwm1);
		   
		 }
		break;
		   default:
				break;
		   }
		   return 0;	
	}
}

static int itop6x_pwm_test_release(struct inode *inode, struct file *filp)
{
	printk(" %s  ! ! !\n",__FUNCTION__);
   pwm_disable(my_pwm1);               // 关闭pwm
   pwm_free(my_pwm1);                  // 释放pwm
   my_pwm1 = NULL;
   return 0;
}

static struct file_operations itop6x_pwm_test_ops = {
	.owner			= THIS_MODULE,
	.open			= itop6x_pwm_test_open, 
	.release        = itop6x_pwm_test_release,
	.unlocked_ioctl	= itop6x_pwm_test_ioctl,

};

static struct miscdevice itop6x_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &itop6x_pwm_test_ops,
};

static int itop6x_pwm_test_probe(struct platform_device *pdev)
{
	int ret;
	printk(" %s  ! ! !\n",__FUNCTION__);
	gpio_free(BUZZER_GPIO);
	ret = gpio_request(BUZZER_GPIO,"my_test");
	if(ret){
		printk("gpio_request BUZZER_GPIO failed!\n");
	}
	gpio_free(BUZZER_GPIO);
	printk("gpio_free(BUZZER_GPIO) ! ! !\n");
	ret = misc_register(&itop6x_misc_dev);
	return 0;
}


static int itop6x_pwm_test_remove (struct platform_device *pdev)
{
	printk(" %s  ! ! !\n",__FUNCTION__);
	misc_deregister(&itop6x_misc_dev);
	gpio_free(BUZZER_GPIO);	
	return 0;
}

static int itop6x_pwm_test_suspend (struct platform_device *pdev, pm_message_t state)
{
	printk(" %s  ! ! !\n",__FUNCTION__);
	return 0;
}

static int itop6x_pwm_test_resume (struct platform_device *pdev)
{
	printk(" %s  ! ! !\n",__FUNCTION__);
	return 0;
}

static struct platform_driver itop6x_pwm_test_driver = {
	.probe = itop6x_pwm_test_probe,
	.remove = itop6x_pwm_test_remove,
	.suspend = itop6x_pwm_test_suspend,
	.resume = itop6x_pwm_test_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int itop6x_pwm_test_init(void)
{
	printk(" %s  ! ! !\n",__FUNCTION__);
	return platform_driver_register(&itop6x_pwm_test_driver);
}

static void itop6x_pwm_test_exit(void)
{	
	printk(" %s  ! ! !\n",__FUNCTION__);
	platform_driver_unregister(&itop6x_pwm_test_driver);
}

module_init(itop6x_pwm_test_init);
module_exit(itop6x_pwm_test_exit);


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("iTOPEET_dz");


