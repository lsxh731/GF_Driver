#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/pwm.h>
#include <linux/fs.h>  
 
/************硬件相关***************/
#include <mach/iomux-mx6q.h>
 
#define PWM_FREQENCY 1
#define DEV_NAME    "mypwm"
#define PWM_DEBUG 1   
/*pwm for this my_pwm*/
struct pwm_device *pwm = NULL;
 
static int my_pwm_open(struct inode *inode, struct file *filp)
{
 
   if(pwm != NULL)
	   return -EBUSY;
 
   /**
	* my_pwm正好挂载在I.MX6的pwm4上，所以这里申请3号（从零开始算）PWM
	*/
   pwm = pwm_request(3, "my_pwm");
   if ( pwm == NULL ) {
	   printk(KERN_ALERT"my_pwm open error.\n");
   }
	#ifdef PWM_DEBUG
	printk(KERN_ALERT"my_pwm opened\n");
	#endif
   return 0;
}
 
static int my_pwm_release(struct inode *inode, struct file *filp)
{
   pwm_disable(pwm);			   // 关闭pwm
   pwm_free(pwm);				   // 释放pwm
   pwm = NULL;
	#ifdef PWM_DEBUG
		printk(KERN_ALERT"my_pwm closed\n");
    #endif
   return 0;
}
 
static long my_pwm_ioctl(struct file *filp,
					   unsigned int cmd, unsigned long arg)
{
   if(pwm == NULL)
	   return -EINVAL;
 
   if(arg < 0)
	   return -EINVAL;
 
   switch (cmd) {
   case PWM_FREQENCY:
	   if(arg==0)
	   	{
		   pwm_disable(pwm);
		   #ifdef PWM_DEBUG
		   	printk(KERN_ALERT"my_pwm off\n");
		   #endif
	   	}
	   else
	   {
		   pwm_config(pwm, 1000000000/arg/2, 1000000000/arg);
		   pwm_enable(pwm);
		   #ifdef PWM_DEBUG
		   	printk(KERN_ALERT"my_pwm on\n");
		   #endif
	   }
	   break;
   default:
	   break;
   }
 
   return 0;
}
 
static struct file_operations my_pwm_fops = {
   .owner			   = THIS_MODULE,
   .unlocked_ioctl	   = my_pwm_ioctl,
   .open			   = my_pwm_open,
   .release 		   = my_pwm_release,
};
 
static struct miscdevice my_pwm_miscdev =
{
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &my_pwm_fops,
};
 
static int __init my_pwm_init(void)
{
  	 //printk(KERN_ALERT"check my_pwm init.\n");
   	misc_register(&my_pwm_miscdev);//misc杂项设备是主设备号为10的驱动设备,自动创建节点
	mxc_iomux_v3_setup_pad(MX6Q_PAD_SD1_CMD__PWM4_PWMO);
	#ifdef PWM_DEBUG
		printk(KERN_ALERT"my_pwm init\n");
	#endif
   	return 0;
}
 
static void __exit my_pwm_exit(void)
{
  misc_deregister(&my_pwm_miscdev);
  #ifdef PWM_DEBUG
		printk(KERN_ALERT"my_pwm removed\n");
  #endif
}
 
module_init(my_pwm_init);
module_exit(my_pwm_exit);
 
MODULE_DESCRIPTION("pwm_my_pwm driver");
MODULE_LICENSE("GPL");