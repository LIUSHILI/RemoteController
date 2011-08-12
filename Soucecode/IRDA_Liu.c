/********************************************************************************
** ���ܣ������������
** ʱ�䣺2011.08.11
** ��������ʿ��
*********************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-gpio.h>

#include <plat/gpio-cfg.h>
#include <mach/gpio-bank-n.h>
#include <mach/gpio-bank-l.h>

#define DEVICE_NAME     "IRDA_dev"  /* �����豸�� */
static volatile char IRDA_value = '0';

unsigned char IRCOM[7];	/* ���ڴ�Ž��յĵ�ַ����� */

#define IRPORT_DATA   (readl(S3C64XX_GPNDAT)&(0x1 << 12))
#define delay_us(num)  udelay(num)

/* ��������жϽṹ */
struct IRDA_irq_desc {
    int irq;	/* �жϺ� */
    int number; /* IRD���� */
    char *name;	/* ������ */
};

/* �����������жϽṹ */
static struct IRDA_irq_desc IRDA_irq = { IRQ_EINT(12), 12,"IRDA"};

void IRDA_do_tasklet(unsigned long n);

static DECLARE_TASKLET(IRDA_tasklet, IRDA_do_tasklet, 0);		/*���岢��tastlet����*/

static volatile int ev_receive = 0;


/* �жϴ����� */
static irqreturn_t IRDA_interrupt(int irq, void *dev_id)
{
	tasklet_schedule(&IRDA_tasklet);	/* ���ȹ�������*/
    return IRQ_RETVAL(IRQ_HANDLED);
}


void IRDA_do_tasklet(unsigned long n)
{
	int i,j;
	int N=0;
	int err = 0;

	disable_irq(IRDA_irq.irq);
	printk("Enter IRDA_do_tasklet. \n");

	ev_receive = 0;

	delay_us(1400);
	if(IRPORT_DATA)
	{
		printk("IRPORT_DATA=1 error! exit! \n");
		enable_irq(IRDA_irq.irq);
		return ;	
	}
	
	while(IRPORT_DATA == 0)
		delay_us(140);
	for(i=0; i<4; i++) 					/* ����4���ֽ� */
	{			
		for(j=0; j<8; j++)
		{								/* �����λ��ʼ����λ������8λ */
			while(IRPORT_DATA)			/* �ȴ���� */
				delay_us(140);
				
			while(IRPORT_DATA==0) 		/* �ȴ���ߣ���Ӧ���͵ļ���͵�ƽ */
				delay_us(140);
				
			while(IRPORT_DATA)
			{	  						/* ��¼�ߵ�ƽ�ĳ���ʱ�䣬��Ӧ���͵�һλ���ʱ�� */
				delay_us(140);
				N ++;					  /* 0.14ms ������1 */
				if(N >30)
				{		 					 /* ��������30���������յ���β�� */
		          goto next ;
				}

			}
			IRCOM[i]=(IRCOM[i] >> 1)&(uint8_t)0x7f;   /* �������λ����0��,��0ʱ���ߵ�ƽʱ��0.56ms */
            if(N>=8){ IRCOM[i] |= 0x80; }             /* �������λ����1��,��1ʱ���ߵ�ƽʱ��1.685ms */
            N=0;
		}  
	}	    

next:

#if 0
	if( IRCOM[2] != (uint8_t)(~IRCOM[3]) )		   /* У����յļ����뷴�����ȡ���Ƿ���� */
	{
		printk("Receive IR Key value error! \n");
		return ;
	}
#endif

	printk("IRCOM[0]=0x%x \n",IRCOM[0]);
	printk("IRCOM[1]=0x%x \n",IRCOM[1]);
	printk("IRCOM[2]=0x%x \n",IRCOM[2]);
	printk("IRCOM[3]=0x%x \n",IRCOM[3]);

    IRCOM[5]=IRCOM[2]&0x0F;  /* ȡ����ĵ���λ������IRCOM[5] */
    IRCOM[6]=IRCOM[2]>>4;      /* ȡ����ĸ���λ����IRCOM[6]�ĵ�4λ */

    if(IRCOM[5]>9)
    { IRCOM[5]=IRCOM[5]+0x37;	/* ��9��������ת��Ϊ��ĸ A--0x41 */
	}
    else
	  IRCOM[5]=IRCOM[5]+0x30;	/* ����0~9 +0x30 ��Ϊ�ַ� */

    if(IRCOM[6]>9)
    { IRCOM[6]=IRCOM[6]+0x37;
	}
    else
	  IRCOM[6]=IRCOM[6]+0x30;
	
	printk("Correct Key value %x H. \n", IRCOM[2]);

	enable_irq(IRDA_irq.irq);
	ev_receive = 1;
}



/* �򿪰����豸 */
static int tiny6410_IRDA_open(struct inode *inode, struct file *file)
{
    int i;
    int err = 0;
	
    err = request_irq(IRDA_irq.irq, IRDA_interrupt, IRQ_TYPE_EDGE_FALLING, 
                          IRDA_irq.name, (void *)&IRDA_irq);
    if (err)  						 /* �������ʧ�ܣ��ͷ���������ж���Դ */
	{
 		disable_irq(IRDA_irq.irq);  /* ��ֹ�ж� */
        free_irq(IRDA_irq.irq, (void *)&IRDA_irq); /* �ͷ��ж���Դ */
        return -EBUSY;
    }

	printk("IRDA_dev opened! \n");
    ev_receive = 1;

    return 0;
}


static int tiny6410_IRDA_close(struct inode *inode, struct file *file)
{
    int i;
    
	free_irq(IRDA_irq.irq, (void *)&IRDA_irq);
	
	printk("IRDA_dev closed! \n");
    return 0;
}



/* ������������ */
static struct file_operations IRDA_dev_fops = {
    .owner   =   THIS_MODULE,
    .open    =   tiny6410_IRDA_open,
    .release =   tiny6410_IRDA_close, 
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &IRDA_dev_fops,
};

static int __init dev_init(void)
{
	int ret;

	ret = misc_register(&misc);

	printk (DEVICE_NAME"\t initialized. \n");

	return ret;
}

static void __exit dev_exit(void)
{
	misc_deregister(&misc);
}

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Du Yunfu.");
