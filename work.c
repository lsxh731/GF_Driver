#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>

static int irq;					
static char * devname;		

static struct work_struct mywork;	
			
 // 接收驱动模块加载时传入的参数
module_param(irq, int, 0644);
module_param(devname, charp, 0644);

// 定义驱动程序的 ID，在中断处理函数中用来判断是否需要处理			
#define MY_DEV_ID	   		1226

// 驱动程序数据结构
struct myirq
{
    int devid;
};
 
struct myirq mydev  ={ MY_DEV_ID };

#define KBD_DATA_REG        0x60  
#define KBD_STATUS_REG      0x64
#define KBD_SCANCODE_MASK   0x7f
#define KBD_STATUS_MASK     0x80

// 工作项绑定的处理函数
static void mywork_handler(struct work_struct *work)
{
    printk("mywork_handler is called. \n");
    // do some other things
}
		
//中断处理函数
static irqreturn_t myirq_handler(int irq, void * dev)
{
    struct myirq mydev;
    unsigned char key_code;
    mydev = *(struct myirq*)dev;	
	
	// 检查设备 id，只有当相等的时候才需要处理
	if (MY_DEV_ID == mydev.devid)
	{
		// 读取键盘扫描码
		key_code = inb(KBD_DATA_REG);
	
		if (key_code == 0x01)
		{
			printk("ESC key is pressed! \n");
			
			// 初始化工作项
			INIT_WORK(&mywork, mywork_handler);
			
			// 加入到工作队列 system_wq
        	schedule_work(&mywork);
		}
	}	

	return IRQ_HANDLED;
}
 
// 驱动模块初始化函数
static int __init myirq_init(void)
{
    printk("myirq_init is called. \n");

	// 注册中断处理函数
    if(request_irq(irq, myirq_handler, IRQF_SHARED, devname, &mydev)!=0)
    {
        printk("register irq[%d] handler failed. \n", irq);
        return -1;
    }

    printk("register irq[%d] handler success. \n", irq);
    return 0;
}
 
// 驱动模块退出函数
static void __exit myirq_exit(void)
{
    printk("myirq_exit is called. \n");

	// 释放中断处理函数
    free_irq(irq, &mydev);
}
 
MODULE_LICENSE("GPL");
module_init(myirq_init);
module_exit(myirq_exit);
