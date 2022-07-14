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
#include <linux/mm.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>

/*寄存器头文件*/
#include <mach/iomux-mx6q.h>
#define BUZZER_GPIO			IMX_GPIO_NR(1, 19)
#define PWM2_KEY_ROW5       IMX_GPIO_NR(1, 1)
#define PWM3_CSI0_RSTB      IMX_GPIO_NR(1, 17)
#define PWM4_ACCL_INTIN     IMX_GPIO_NR(1, 18)

#define PWM_2  0x02
#define PWM_3  0x03
#define PWM_4  0x04

#define PWM_FREQENCY 1

static unsigned char array[10]={9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
static unsigned char *buffer;

struct pwm_device *gfzk_pwm2=NULL;
struct pwm_device *gfzk_pwm3=NULL;
struct pwm_device *gfzk_pwm4=NULL;

#define DEVICE_NAME				"gfpwm"
#define DRIVER_NAME 			"gfpwm"

static int itop6x_pwm_test_map(struct file *filep, struct vm_area_struct *vma)
{    
	unsigned long phys;

	phys = virt_to_phys(buffer);    
	
	if(remap_pfn_range(vma,
			vma->vm_start,
			phys >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot))
		return -1;

	return 0;
}

static int itop6x_pwm_test_open(struct inode *inode, struct file *file) 
{
   //printk(" %s  ! ! !\n",__FUNCTION__);
   
   //PWM2_KEY_ROW5
   mxc_iomux_v3_setup_pad(MX6Q_PAD_GPIO_1__PWM2_PWMO);
   gfzk_pwm2 = pwm_request(1, "gfzk_pwm2");
   if ( gfzk_pwm2 == NULL ) {
       printk(KERN_ALERT"gfzk_pwm2 open error.\n");
   }
   
   //PWM3_CSI0_RSTB 
   mxc_iomux_v3_setup_pad(MX6Q_PAD_SD1_DAT1__PWM3_PWMO);
   gfzk_pwm3 = pwm_request(2, "gfzk_pwm3");
   if ( gfzk_pwm3 == NULL ) {
       printk(KERN_ALERT"gfzk_pwm3 open error.\n");
   }
   
   //PWM4_ACCL_INTIN
   mxc_iomux_v3_setup_pad(MX6Q_PAD_SD1_CMD__PWM4_PWMO);
   gfzk_pwm4 = pwm_request(3, "gfzk_pwm4");
   if ( gfzk_pwm4 == NULL ) {
       printk(KERN_ALERT"gfzk_pwm4 open error.\n");
   }
   /*
   pwm_config(gfzk_pwm2, 1000000000/2, 1000000000);
   pwm_enable(gfzk_pwm2);
   printk("####PWM2_KEY_ROW5 open successed\n");
   
   pwm_config(gfzk_pwm3, 1000000000/2, 1000000000);
   pwm_enable(gfzk_pwm3);
   printk("####PWM3_CSI0_RSTB open successed\n");
   
   pwm_config(gfzk_pwm4, 1000000000/2, 1000000000);
   pwm_enable(gfzk_pwm4);
   printk("####PWM4_ACCL_INTIN open successed\n");
   */
   
   return 0;
}

static long itop6x_pwm_test_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)

{
	int ret,num,ccmd;
	ret =-1;
	num = (int)arg;
	//printk("cmd is %d,arg is %d\n",cmd,arg);
	//printk(" %s  ! ! !\n",__FUNCTION__);
	{
   if(arg < 0)
       return -EINVAL;
   switch (cmd) {
    case 1:
       if(num==0)
        {
           pwm_disable(gfzk_pwm2);
		   pwm_disable(gfzk_pwm3);
		   pwm_disable(gfzk_pwm4);
        }
       else
       {
        ret = gpio_get_value(PWM2_KEY_ROW5);
		printk("####PWM2_KEY_ROW5 value is = %d\n",ret); 
		ret = gpio_get_value(PWM3_CSI0_RSTB);
		printk("####PWM3_CSI0_RSTB value is = %d\n",ret);
		ret = gpio_get_value(PWM4_ACCL_INTIN);
		printk("####PWM4_ACCL_INTIN value is = %d\n",ret);  
		 }
		break;
	case 6:
       if(num==0)
        {
           pwm_disable(gfzk_pwm2);
        }
       else
       {
		   pwm_config(gfzk_pwm2, 2000000000/num/3, 1000000000/num);  //第二个参数表示高电平时间，
           pwm_enable(gfzk_pwm2);
		   printk("####PWM2_KEY_ROW5 open successed\n");
		 }
		break;
	case 3:
       if(num==0)
        {
           pwm_disable(gfzk_pwm3);
        }
       else
       {
           pwm_config(gfzk_pwm3, 1000000000/num/2, 1000000000/num);
           pwm_enable(gfzk_pwm3);
		   printk("####PWM3_CSI0_RSTB open successed\n");
		 }
		break;
	case 4:
       if(num==0)
        {
           pwm_disable(gfzk_pwm4);
        }
       else
       {
           pwm_config(gfzk_pwm4, 1000000000/num/2, 1000000000/num);
           pwm_enable(gfzk_pwm4);
		   printk("####PWM4_ACCL_INTIN open successed\n");
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
   pwm_disable(gfzk_pwm2);               // 关闭pwm
   pwm_free(gfzk_pwm2);                  // 释放pwm
   gfzk_pwm2 = NULL;
   
   pwm_disable(gfzk_pwm3);               // 关闭pwm
   pwm_free(gfzk_pwm3);                  // 释放pwm
   gfzk_pwm3 = NULL;
   
   pwm_disable(gfzk_pwm4);               // 关闭pwm
   pwm_free(gfzk_pwm4);                  // 释放pwm
   gfzk_pwm4 = NULL;
   return 0;
}


static struct file_operations itop6x_pwm_test_ops = {
	.owner			= THIS_MODULE,
	.open			= itop6x_pwm_test_open, 
	.release        = itop6x_pwm_test_release,
	.unlocked_ioctl	= itop6x_pwm_test_ioctl,
	.mmap 			= itop6x_pwm_test_map,

};

static struct miscdevice itop6x_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &itop6x_pwm_test_ops,
};

static int itop6x_pwm_test_probe(struct platform_device *pdev)
{
	int ret;
	unsigned char i;
	printk(" %s  ! ! !\n",__FUNCTION__);
	
	//内存分配
	buffer = (unsigned char *)kmalloc(PAGE_SIZE,GFP_KERNEL);
	//driver起来时初始化内存前10个字节数据
	for(i = 0;i < 10;i++)
		buffer[i] = array[i];
	//将该段内存设置为保留
	SetPageReserved(virt_to_page(buffer));
	
	//PWM2_KEY_ROW5
	gpio_free(PWM2_KEY_ROW5);
	ret = gpio_request(PWM2_KEY_ROW5,"my_test");
	if(ret){
		printk("gpio_request PWM2_KEY_ROW5 failed!\n");
	}
	gpio_free(PWM2_KEY_ROW5);
	printk("gpio_free(PWM2_KEY_ROW5) ! ! !\n");
	//PWM3_CSI0_RSTB
	gpio_free(PWM3_CSI0_RSTB);
	ret = gpio_request(PWM3_CSI0_RSTB,"my_test");
	if(ret){
		printk("gpio_request PWM3_CSI0_RSTB failed!\n");
	}
	gpio_free(PWM3_CSI0_RSTB);
	printk("gpio_free(PWM3_CSI0_RSTB) ! ! !\n");
	//PWM4_ACCL_INTIN
	gpio_free(PWM4_ACCL_INTIN);
	ret = gpio_request(PWM4_ACCL_INTIN,"my_test");
	if(ret){
		printk("gpio_request PWM4_ACCL_INTIN failed!\n");
	}
	gpio_free(PWM4_ACCL_INTIN);
	printk("gpio_free(PWM4_ACCL_INTIN) ! ! !\n");
	
	ret = misc_register(&itop6x_misc_dev);
	return 0;
}


static int itop6x_pwm_test_remove (struct platform_device *pdev)
{
	printk(" %s  ! ! !\n",__FUNCTION__);
	misc_deregister(&itop6x_misc_dev);
	gpio_free(PWM2_KEY_ROW5);	
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


