
/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
//#include <linux/vmalloc.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <asm/param.h> /* include HZ */
#include <linux/string.h> /* string operations */
#include <linux/timer.h> /* timer gizmos */
#include <linux/list.h> /* include list data struct */
#include <asm/gpio.h>

#include <linux/interrupt.h>
#include <asm/arch/pxa-regs.h>
#include <asm-arm/arch/hardware.h>

#define MYGPIO 17
#define MYGPIO2 101
#define MYGPIO3 117

MODULE_LICENSE("Dual BSD/GPL");

/* Declaration of memory.c functions */
static int mygpio_open(struct inode *inode, struct file *filp);
static int mygpio_release(struct inode *inode, struct file *filp);
static ssize_t mygpio_read(struct file *filp,char *buf, size_t count, loff_t *f_pos);
static ssize_t mygpio_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos);
static void mygpio_exit(void);
static int mygpio_init(void);
static int fasync_example_fasync(int fd, struct file *filp, int mode);

static void LEDLighter(unsigned v);

static void timer_handler(unsigned long data);
static void counterRead_handler(unsigned long data);
static void t_handler(unsigned long data);

/* Structure that declares the usual file */
/* access functions */
struct file_operations mygpio_fops = {
	read: mygpio_read,
	write: mygpio_write,
	open: mygpio_open,
	release: mygpio_release,
	fasync: fasync_example_fasync
};

/* Declaration of the init and exit functions */
module_init(mygpio_init);
module_exit(mygpio_exit);

static unsigned capacity = 2560;
static unsigned bite = 2560;
module_param(capacity, uint, S_IRUGO);
module_param(bite, uint, S_IRUGO);

/* Global variables of the driver */
/* Major number */
static int mygpio_major = 61;

/* Buffer to store data */
static char *mygpio_buffer;
/* length of the current message */
static int mygpio_len;

//from fasync
struct fasync_struct *async_queue;


/* timer pointer */
static struct timer_list button_timer[2];

unsigned button[2], LED[4];
int countDir, countVal;
unsigned counter=0;


int dutyCounter = 0;
int counterTimerVal = 2000;

char speedChar = 'H';


int timePer = 2000;

int irq1 = 0;
int irq2 = 0;
int irq3 = 0;

int isRunning = 0;
int isUp = 0;
static long unsigned int window = 0;
char direction = 'R';

long timeStamp = 0;

irqreturn_t gpio_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	int ret;
	if (irq == irq3) {
			// pwm
			//printk("PWM \n");
			//direction = 'D';

			if (dutyCounter == 0) {
				dutyCounter = 1;
			}
			else if (dutyCounter == 1) {
				dutyCounter = 2;
			}
			else {
				dutyCounter = 0;
			}
	}
	else {
		ret = mod_timer(&button_timer[2],jiffies + msecs_to_jiffies(500));
		if (ret == 0) {
			if (irq == irq1) {
				
				direction = 'L';
				timeStamp = jiffies;

				if (isRunning == 0)
					isRunning = 1;
				else
					isRunning = 0;
			}
			else if (irq == irq2) {
				direction = 'R';
				timeStamp = jiffies;

				if (isUp == 0)
					isUp = 1;
				else
					isUp = 0;
			}
		}
		else {
			countVal = 15;
			LEDLighter(countVal);
			if (irq == irq2) {

				if (isRunning == 0)
					isRunning = 1;
				else
					isRunning = 0;
			}
			else if (irq == irq1) {
				if (isUp == 0)
					isUp = 1;
				else
					isUp = 0;
			}
		}
	}

	return IRQ_HANDLED;
}

static int mygpio_init(void)
{
	int result, ret;
	pxa_gpio_mode(MYGPIO | GPIO_IN);
	pxa_gpio_mode(MYGPIO2 | GPIO_IN);
	pxa_gpio_mode(MYGPIO3 | GPIO_IN);

	irq1 = IRQ_GPIO(MYGPIO);
	irq2 = IRQ_GPIO(MYGPIO2);
	irq3 = IRQ_GPIO(MYGPIO3);



	button[0] = 17;	// button 0
	button[1] = 101; // button 1
	button[2] = 117;
	LED[0] = 16;
	LED[1] = 29;
	LED[2] = 30;
	LED[3] = 31;
	
	gpio_direction_input(button[0]);
	gpio_direction_input(button[1]);
	gpio_direction_input(button[2]);

	//gpio_direction_output(LED[0],0);
	gpio_direction_output(LED[1],0);
	gpio_direction_output(LED[2],0);
	gpio_direction_output(LED[3],0);

	pxa_gpio_set_value(button[0],0);
	pxa_gpio_set_value(button[1],0);
	pxa_gpio_set_value(button[2],0);
	pxa_gpio_set_value(LED[0],1);
	pxa_gpio_set_value(LED[1],1);	
	pxa_gpio_set_value(LED[2],1);
	pxa_gpio_set_value(LED[3],1);
	
	countDir = -1;
	countVal = 15;



	
	if (request_irq(irq1, &gpio_irq, SA_INTERRUPT | SA_TRIGGER_RISING,
				"mygpio", NULL) != 0 ) {
                printk ( "irq not acquired \n" );
                return -1;
        }else{
               // printk ( "irq %d acquired successfully \n", irq1 );
	}

	if (request_irq(irq2, &gpio_irq, SA_INTERRUPT | SA_TRIGGER_RISING,
				"mygpio", NULL) != 0 ) {
                printk ( "irq not acquired \n" );
                return -1;
        }else{
                //printk ( "irq %d acquired successfully \n", irq2 );
	}
	if (request_irq(irq3, &gpio_irq, SA_INTERRUPT | SA_TRIGGER_RISING,
				"mygpio", NULL) != 0 ) {
                printk ( "irq not acquired \n" );
                return -1;
        }else{
                //printk ( "irq %d acquired successfully \n", irq3 );
	}



	/* Registering device */
	result = register_chrdev(mygpio_major, "mygpio", &mygpio_fops);
	if (result < 0)
	{
		/*printk(KERN_ALERT "mygpio: cannot obtain major number %d\n", mygpio_major);*/
		return result;
	}

	/* Allocating mygpio for the buffer */
	mygpio_buffer = kmalloc(capacity, GFP_KERNEL);
	/* Check if all right */
	if (!mygpio_buffer)
	{ 
		//printk(KERN_ALERT "Insufficient kernel memory\n"); 
		result = -ENOMEM;
		goto fail; 
	} 

	memset(mygpio_buffer, 0, capacity);
	mygpio_len = 0;
	//+printk(KERN_ALERT "Inserting mygpio module\n"); 
	
	setup_timer(&button_timer[0], timer_handler, 0);
	ret = mod_timer(&button_timer[0], jiffies + msecs_to_jiffies(timePer/2));
	
	setup_timer(&button_timer[1], counterRead_handler, 0);
	setup_timer(&button_timer[2], t_handler, 0);


return 0;

fail: 
	mygpio_exit(); 
	return result;
}



static void mygpio_exit(void)
{
	/* Freeing the major number */
	unregister_chrdev(mygpio_major, "mygpio");

	/* Freeing buffer memory */
	if (mygpio_buffer)
		kfree(mygpio_buffer);
	/* Freeing timer list */
	//if (the_timer)
	//	kfree(the_timer);

	free_irq(IRQ_GPIO(MYGPIO), NULL);
	free_irq(IRQ_GPIO(MYGPIO2), NULL);
	free_irq(IRQ_GPIO(MYGPIO3), NULL);
	
	if (timer_pending(&button_timer[0]))
		del_timer(&button_timer[0]);
	if (timer_pending(&button_timer[1]))
		del_timer(&button_timer[1]);
	if (timer_pending(&button_timer[2]))
		del_timer(&button_timer[2]);

	//printk(KERN_ALERT "Removing mygpio module\n");

}

static int mygpio_open(struct inode *inode, struct file *filp)
{
	/*printk(KERN_INFO "open called: process id %d, command %s\n",
		current->pid, current->comm);*/
	/* Success */
	return 0;
}

static int mygpio_release(struct inode *inode, struct file *filp)
{
	/*printk(KERN_INFO "release called: process id %d, command %s\n",
		current->pid, current->comm);*/
	/* Success */
	return 0;
}



static ssize_t mygpio_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	int temp, flag;
	char tbuf[256], *tbptr = tbuf;
	int valSet;
	

	flag = 0;

	//printk(KERN_ALERT "In write\n");
		
	
	/* end of buffer reached */
	if (*f_pos >= capacity)
	{
		return -ENOSPC;
	}

	/* do not eat more than a bite */
	if (count > bite) count = bite;

	/* do not go over the end */
	if (count > capacity - *f_pos)
		count = capacity - *f_pos;

	if (copy_from_user(mygpio_buffer + *f_pos, buf, count))
	{
		return -EFAULT;
	}


	temp = *f_pos;
	
	// echo "f1" > /dev/mygpio for 1/2 second
	//printk("buffer: %c count: %d \n",mygpio_buffer[temp], count);
	if (mygpio_buffer[temp] == 'f' && count < 4) {
		//printk(KERN_ALERT "count =%d\n",count);
		
		for (temp = *f_pos; temp < count + *f_pos; temp++) {					  
			tbptr += sprintf(tbptr, "%c", mygpio_buffer[temp]);

			if (flag == 0) {
				flag = 1;
			}
			else if (flag == 1 &&  (mygpio_buffer[temp] - '0') > 0 && (mygpio_buffer[temp] - '0') < 4)
			{
								
				//digitR[size] = mygpio_buffer[temp] - '0';
				timePer = (mygpio_buffer[temp] - '0') * 2000;
				if (timePer == 2000)
					speedChar = 'H';
				else if (timePer == 4000)
					speedChar = 'M';
				else
					speedChar = 'L';
				flag = 9; // done
			}
		}
	}
	else if (mygpio_buffer[temp] == 'v' && count < 4) {
		for (temp = *f_pos; temp < count + *f_pos; temp++) {					  
			tbptr += sprintf(tbptr, "%c", mygpio_buffer[temp]);

			if (flag == 0) {
				flag = 1;
			}
			else if (flag == 1) {
			
				valSet = mygpio_buffer[temp] - '0';
				if (valSet >= 49 && valSet <= 54)
					countVal = valSet - 39;
				else if (valSet >=0 && valSet <= 9)
					countVal = valSet;
				flag = 9; // done
			}
		}
	}
	else if (mygpio_buffer[temp] == 'C') {
		//printk("In C... \n");
		for (temp = *f_pos; temp < count + *f_pos; temp++) {					  
			tbptr += sprintf(tbptr, "%c", mygpio_buffer[temp]);

			if (flag == 0) {
				flag = 1;
			}
			else if (flag == 1) {
			
				valSet = mygpio_buffer[temp] - '0';
				if (valSet >= 0 && valSet <= 50) {
					//printk("Create timer... \n");
					counterTimerVal = valSet;
					mod_timer(&button_timer[1], jiffies + msecs_to_jiffies(counterTimerVal*1000));
					flag = 9; // done
				}
			}
		}
	}


	*f_pos += count;
	mygpio_len = *f_pos;
	return count;
}

static ssize_t mygpio_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{ 
	char tbuf[1024];
	char dir[128], state[128], bright[128];
	int len;
	/*printk(KERN_ALERT "\n------------------------\n");
	printk(KERN_ALERT "Count Value = %d\n", countVal);
	printk(KERN_ALERT "Count Period [sec] = %d\n", timePer/1000);*/
	
	if (countDir==1)
		sprintf(dir,"Up");
	else
		sprintf(dir,"Down");

	if (isRunning != 0)
		sprintf(state,"Run");
	else
		sprintf(state,"Hold");

	if (dutyCounter == 0)
		sprintf(bright,"H");
	else if (dutyCounter == 1)
		sprintf(bright, "M");
	else
		sprintf(bright, "L");
		
	
	len = sprintf(tbuf,"%u %c ", timeStamp, direction);
	printk("IN KERNEL, JIFFIES: %u\n", timeStamp);

	//printk("BUFF: %s \n", tbuf);
	mod_timer(&button_timer[1],jiffies + msecs_to_jiffies(counterTimerVal*1000));


	strcpy(mygpio_buffer,tbuf);

	//do not go over then end
	if (count > 2560 - *f_pos)
		count = 2560 - *f_pos;
	
	if (copy_to_user(buf,mygpio_buffer, count))
	{
		return -EFAULT;	
	}

	
	// Changing reading position as best suits 
	*f_pos += count; 
	return count; 
}




static void timer_handler(unsigned long data) 
{
	//unsigned buttonValue, LEDValue;
	
	//buttonValue = pxa_gpio_get_value(buttonGPIO);
	//LEDValue = pxa_gpio_get_value(LED[0]);
	//printk(KERN_ALERT "button #0 (gpio=%d) value=%d\n",buttonGPIO, buttonValue);
	//printk(KERN_ALERT "LED #0 (gpio=%d) value=%d\n",LED[0], LEDValue);


	if (isUp != 0)
		countDir = 1;
	else
		countDir = -1;


	if (isRunning != 0)
	{
		//printk(KERN_ALERT "light OFF (gpio=%d) value =%d\n",LEDGPIO,pxa_gpio_get_value(buttonGPIO));
		countVal += countDir;
		//printk("countVal %d \n", countVal);
		if (countVal > 15)
			countVal = 0;
		else if (countVal < 0)
			countVal = 15;

	}
	LEDLighter(countVal);

	
	counter++;
	//printk(KERN_ALERT "reset timer %d\n\n",counter);
	mod_timer(&button_timer[0],jiffies + msecs_to_jiffies(timePer/2));

}	

static void counterRead_handler(unsigned long data) 
{

  if (async_queue)
  kill_fasync(&async_queue, SIGIO, POLL_IN);

	
}


static void LEDLighter(unsigned v)
{
	int i, mask, masked_v, theBIT;
	for (i=1; i<4; i++)
	{
		if (isUp == 0) {
			mask =  1 << i;
			masked_v = v  & mask;
			theBIT = masked_v >> i;
			pxa_gpio_set_value(LED[i],theBIT);
		}
		else {
			mask =  1 << i;
			masked_v = v  & mask;
			theBIT = masked_v >> i;
			pxa_gpio_set_value(LED[i],theBIT);
		}

		
	}

	if (v % 2 == 0) {
			pxa_gpio_mode(GPIO16_PWM0_MD);
			CKEN |= CKEN0_PWM0;
			PWM_PWDUTY0 = 0x00;
			PWM_PERVAL0 = 0x40;
		}
	else {
			if (dutyCounter == 0) {
				pxa_gpio_mode(GPIO16_PWM0_MD);
				CKEN |= CKEN0_PWM0;
				PWM_PWDUTY0 = 0x02*30;
				PWM_PERVAL0 = 0x40;
			}
			else if (dutyCounter == 1) {
				pxa_gpio_mode(GPIO16_PWM0_MD);
				CKEN |= CKEN0_PWM0;
				PWM_PWDUTY0 = 0x02*10;
				PWM_PERVAL0 = 0x40;
			}
			else {
				pxa_gpio_mode(GPIO16_PWM0_MD);
				CKEN |= CKEN0_PWM0;
				PWM_PWDUTY0 = 0x02;
				PWM_PERVAL0 = 0x40;
			}
	}
}

static void t_handler(unsigned long data) 
{
	// empty...
}

static int fasync_example_fasync(int fd, struct file *filp, int mode) {
	return fasync_helper(fd, filp, mode, &async_queue);
}

