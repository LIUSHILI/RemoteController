/*
 *  LED 驱动
 *
 *   @author  duyunfu
 *   @date     2011-8-6
 */
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/irq.h>
//#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/unistd.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-gpio.h>

#include <plat/gpio-cfg.h>
#include <mach/gpio-bank-e.h>
#include <mach/gpio-bank-k.h>

#define DEVICE_NAME "led_dev" 

/************************************************************************************
**功能：LED亮灭控制
**说明：cmd=0 置高电平，熄灭   cmd=1 置低电平，点亮
**		arg=1~4 对应 LED1~LED4
************************************************************************************/
static long mini6410_led_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd) 
	{
		unsigned tmp;
		case 0:
		case 1:
			if (arg > 5 || (arg < 1)) 
			{
				printk (DEVICE_NAME": arg=%d cmd=%d\n", arg, cmd);
				return -EINVAL;
			}
		tmp = readl(S3C64XX_GPKDAT);   /* 读GPK */
		tmp &= ~(1 << (3 + arg));	   /* 清零arg+3对应的位 */
		tmp |= ( (!cmd) << (3 + arg) );/* 若cmd=0 置位arg+3对应的位 */
		writel(tmp, S3C64XX_GPKDAT);   /* 写GPK */
		//printk (DEVICE_NAME": %d %d\n", arg, cmd);
		return 0;
	default:
		return -EINVAL;
	}
}

/* LED 操作函数 */
static struct file_operations LED_dev_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= mini6410_led_dev_ioctl,  /* LED 亮灭控制函数 */
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,    /* 设备名 */
	.fops = &LED_dev_fops,
};

/* LED设备初始化 */
static int __init LED_dev_init(void)
{
	int ret;
	unsigned tmp;
	
	tmp = readl(S3C64XX_GPKCON);
	tmp = (tmp & ~(0xffffU<<16))|(0x1111U<<16);
	writel(tmp, S3C64XX_GPKCON);
		
	tmp = readl(S3C64XX_GPKDAT);
	tmp |= (0xF << 4);
	writel(tmp, S3C64XX_GPKDAT);  /* 先熄灭所有LED */

	ret = misc_register(&misc);   /* 注册 */

	printk (DEVICE_NAME"\t initialized.\n");  /* 内核打印信息，提示已经初始化 */

	return ret;
}

/* LED设备退出 */
static void __exit LED_dev_exit(void)
{
	misc_deregister(&misc);     
}

module_init(LED_dev_init);
module_exit(LED_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Du Yunfu");