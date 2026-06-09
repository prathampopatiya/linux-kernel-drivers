/* 
 * Steps to implement IOCTL in linux device Drivers 
 * Create IOCTL command in the driver 
 * Write the IOCTL function in the driver 
 * Create IOCTL command in the userspace application 
 * use IOCTL system call in userspace 
*/

#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/ioctl.h>
#include<linux/device.h>
#include<linux/interrupt.h>
#include<asm/io.h> 
#include<linux/err.h>
#define SIGETX 51 

#define REG_CURRENT_TASK _IOW('a','a',int32_t *) 

#define IRQ_NO 11 

static struct task_struct *task = NULL;
static int signum = 0;

int32_t value = 0;
dev_t dev = 0;
static struct cdev etx_cdev;
static struct class *dev_class;

// Func prototypes 
static int      __init etx_driver_init(void);
static void     __exit etx_driver_exit(void);
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

// File ops structure 
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = etx_open, 
    .release = etx_release,
    .read = etx_read,
    .write = etx_write,
    .unlocked_ioctl = etx_ioctl,
};

// interrupt handler for the IRQ_NO 11 
static irqreturn_t irq_handler(int irq,void *dev_id){
    struct kernel_siginfo info;
    pr_info("[*] Shared IRQ: interrupt Occurred");

    // sending singal to app 
    memset(&info,0,sizeof(struct kernel_siginfo));
    info.si_signo = SIGETX;
    info.si_code = SI_QUEUE;
    info.si_int = 1;

    if(task != NULL){
        pr_info("[*] sending signal to user application\n");
        if(send_sig_info(SIGETX,&info,task) < 0){
            pr_err("[+] Failed to send singal\n");
        }
    }
    return IRQ_HANDLED;
}

static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}

static int etx_release(struct inode *inode, struct file *file)
{
    struct task_struct *ref_task = get_current();
    pr_info("[*] Device File Closed...!!!\n");

    // delete the task 
    if(ref_task == task){
        task = NULL;
    }
    return 0;
}

static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{    
    pr_info("[*] Read Function\n");
    asm("int $0x3b");
    return 0;
}

static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    pr_info("[*] Write function\n");
    return len;
}

// IOCTL function defination
// Step-2 
static long etx_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
   if(cmd == REG_CURRENT_TASK){
       pr_info("[*] REG_CURRENT_TASK\n");
       task = get_current();
       signum = SIGETX;
   }
   return 0;
}

static int __init etx_driver_init(void){
    // Allocate the Major number
    if((alloc_chrdev_region(&dev,0,1,"etx_Ioctl_dev")) < 0){
        pr_err("[-] Error allocating Major Number\n");
        return -1;
    }
    pr_info("Major: %d\t Minor: %d\n",MAJOR(dev),MINOR(dev));
    // init cdev struct for the file ops 
    cdev_init(&etx_cdev,&fops);

    // Add the character device to system 
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("[-] Failed to add device!!\n");
        goto r_cdev;
    }
    if(IS_ERR(dev_class = class_create("etc_dev_class"))){
        pr_err("[-] Failed to create class\n");
        goto r_class;
    }
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
        pr_info("[-] Failed to create device\n");
        goto r_device;
    }
    if(request_irq(IRQ_NO,irq_handler,IRQF_SHARED,"etx_device",(void *)(irq_handler))){
        pr_info("[-] unable to request IRQ\n");
        goto r_irq;
    }
    pr_info("[+] Driver successfully created\n");
    return 0;
r_irq:
    free_irq(IRQ_NO,(void *)(irq_handler));
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
r_cdev:
    unregister_chrdev_region(dev,1);
    return -1;
}

static void __exit etx_driver_exit(void){
    free_irq(IRQ_NO,(void *)(irq_handler));
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev,1);
    pr_info("Cleanup successfull Exiting ---+---***\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pratham Popatiya");
MODULE_DESCRIPTION("Simple Linux device driver Signals (IOCTL)");
MODULE_VERSION("1.17");
