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
#define DEVICE_NAME	   "errinput"
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
		IMX_GPIO_NR(3,0),	//EIM_DA0_VIBOD 	序号1	振动台过位移
		IMX_GPIO_NR(3,1),	//EIM_DA1_VIBCU 	序号2	振动台冷却
		IMX_GPIO_NR(3,2),	//EIM_DA2_VIBOT 	序号3	振动过温
		IMX_GPIO_NR(3,3),	//EIM_DA3_STOD 		序号4	滑台过位移保护
		IMX_GPIO_NR(3,4),	//EIM_DA4_STUP		序号5	滑台欠压
		IMX_GPIO_NR(3,5),	//EIM_DA5_STOT		序号6	滑台过温
		IMX_GPIO_NR(3,6),	//EIM_DA6_STOL		序号7	滑台欠液位
		IMX_GPIO_NR(3,7),	//EIM_DA7_RELAY		序号8	功放热继保护
		IMX_GPIO_NR(3,8),	//EIM_DA8_DOOR		序号9	门保护
		IMX_GPIO_NR(3,9),	//EIM_DA9_HBUP		序号10	静压轴承欠压
		IMX_GPIO_NR(3,10),	//EIM_DA10_EXT1		序号11	自定义1
		IMX_GPIO_NR(3,11),	//EIM_DA11_EXT2		序号12	自定义2
		IMX_GPIO_NR(3,12),	//EIM_DA12_EXT3		序号13	自定义3
		IMX_GPIO_NR(3,13),	//EIM_DA13_EXT4		序号14	自定义4
		IMX_GPIO_NR(3,30),	//EIM_D30_ESTOP		序号15	紧急停机
		IMX_GPIO_NR(2,22),	//EIM_A16_1OT		序号16	从柜1整流桥过温1
		IMX_GPIO_NR(2,21),	//EIM_A17_2OT		序号17	从柜1整流桥过温2
		IMX_GPIO_NR(2,20),	//EIM_A18_3OT		序号18	从柜1整流桥过温3
		IMX_GPIO_NR(2,19),	//EIM_A19_4OT		序号19	从柜1整流桥过温4
		IMX_GPIO_NR(2,18),	//EIM_A20_SESTOP	序号20	紧急停机
		IMX_GPIO_NR(2,17),	//EIM_A21_1FAULT	序号21	从柜1错误信号	
		IMX_GPIO_NR(2,16),	//EIM_A22_2FAULT	序号22	从柜2错误信号
		IMX_GPIO_NR(6,6),	//EIM_A23_3FAULT	序号23	从柜3错误信号
		IMX_GPIO_NR(5,4),	//EIM_A24_4FAULT	序号24	从柜4错误信号
		IMX_GPIO_NR(1,9),  	//KEY_COL6
		//IMX_GPIO_NR(3,17),  //EIM_D17_TEST
};
 
static irqreturn_t ErrInput_interrupt(int irq,void *dev_id,struct file * file)
{
	int ret,i;
	mutex_lock(&irq_lock);
	//pt_count++;
	//if(pt_count > 40000)
	//{
	//	printk("#####receive a interrupt 1!#####\n");
		//pt_count = 0;
	//}
	//printk("#####receive a interrupt!#####\n");
	p_irq = -1;
	for(i=0; i<sizeof(irqs)/sizeof(irqs[0]); i++){
		if(irq == irqs[i])
		{
			p_irq = i;
			i = 30;
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

static long ErrInput_ioctl(struct file *f, unsigned int cmd,
		unsigned long arg)
{
	int ret;
	ret = gpio_get_value(gpios[arg]);
	//printk("####gpios[%d] = %d###\n",arg,ret);
	return ret;
}

ssize_t ErrInput_write(struct file *f, const char __user *buf, size_t t, loff_t *len){
	

}

ssize_t ErrInput_read(struct file *f, const char __user *buf, size_t t, loff_t *len){
	
	copy_to_user(buf,&p_irq,1);
	//printk("ErrInput_read successed,p_irq = %d\n",p_irq);
	return 0;
}
static const struct file_operations ErrInput_fops =
{
	.owner   	=   THIS_MODULE,
	.write		=   ErrInput_write,
	.fasync     =   ErrInput_fasync,
	.read		= 	ErrInput_read,
	.unlocked_ioctl	= ErrInput_ioctl,
};
 
struct miscdevice ErrInput_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
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
			if((i==5)|(i==6)|(i==11)|(i==12)|(i==13)|(i==14))
			{
				gpio_direction_output(gpios[i],1);
				gpio_direction_input(gpios[i]);
			}
			else if((i==20)|(i==21)|(i==22)|(i==23)) 
		    {
				gpio_direction_output(gpios[i],0);
			    gpio_direction_input(gpios[i]);
			}
		    else
			{
				gpio_direction_output(gpios[i],1);
				gpio_direction_input(gpios[i]);
			}	
			ret = gpio_get_value(gpios[i]);
			printk("####gpio[%d] value is = %d\n", i, ret);
			//gpio_direction_input(gpios[i]);
			//gpio_free(gpios[i]);
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
		if((i==5)|(i==6)|(i==11)|(i==12)|(i==13)|(i==14))
			request_irq(irqs[i],ErrInput_interrupt,IRQ_TYPE_EDGE_FALLING,"ErrInput_interrupt",pdev);
		else if((i==20)|(i==21)|(i==22)|(i==23))
			request_irq(irqs[i],ErrInput_interrupt,IRQ_TYPE_EDGE_RISING,"ErrInput_interrupt",pdev);
		else
			request_irq(irqs[i],ErrInput_interrupt,IRQ_TYPE_EDGE_FALLING,"ErrInput_interrupt",pdev);
		printk("#####interrupt irqs[%d] = %d #####\n",i,irqs[i]);
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
		.name = DRIVER_NAME,
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
