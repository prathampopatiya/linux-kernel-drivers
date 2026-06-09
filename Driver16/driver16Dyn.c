#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/err.h>
#include<linux/interrupt.h>
#include<asm/io.h> 

#define IRQ_NO 11 
/*Tasklets are used to queue up work to be done at later time. Tasklets can be ran in parallel 
 * but same Tasklet cannot be run on multiple CPU at the same time. each tasklet runs on specific
 * cpu they are scheduled on.
 *
 * In short, a tasklet in linux is something like a very small thread that has 
 * neither stack nor context of its own. Such “threads” work quickly and completely.
 *
 * Tasklets are atomic, so we cannot use sleep() and such synchronization primitives as 
 * mutexes, semaphores, etc. from them. But we can use spinlock.
 *
 * there are two ways to create Tasklets 1. Static 2. Dynamic 
 * struct tasklet_struct {
 *  struct tasklet_struct * next;
 *  unsigned long state;
 *  atomic_t count;
 *  void (*func)(unsigned long);
 *  unsigned long data;
 * };
 *
 * DECLARE_TASKLET(name, func, data); This macro used to create the tasklet structure 
 * and assigns the parameters to that structure.
 * If we are using this macro then the tasklet will be in the enabled state.
 *
 * DECLARE_TASKLET_DISABLED(name, func, data); The tasklet can be declared and set at a disabled state,
 * which means that the tasklet can be scheduled, but will not run until the tasklet is specifically 
 * enabled. You need to use tasklet_enable to enable.
 *
 * void tasklet_enable(struct); enable the disabled tasklet created using above method 
 *
 * void tasklet_schedule (struct tasklet_struct *t); this method is used to schedule it with normal priority
 * void tasklet_hi_schedule (struct tasklet_struct *t); this one for high priority.
 *
 */ 
static void tasklet_func(unsigned long );

//DECLARE_TASKLET(tasklet,tasklet_func); static method 

// Dynamic method 
struct tasklet_struct *tasklet = NULL;

static void tasklet_func(unsigned long data){
    pr_info("Executing tasklet function\n");
}

// add the request_irq and free_irq along with the interrupt handler 
static irqreturn_t irq_handler(int irq,void *dev_id){
    pr_info("Shared IRQ: interrupt Occurred\n");
    // schedule the task to the tasklet 
    tasklet_schedule(tasklet);
    return IRQ_HANDLED;
}


volatile int etx_value = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
struct kobject *kobj_ref;

static int      __init etx_driver_init(void);
static void     __exit etx_driver_exit(void);
 
/*************** Driver functions **********************/
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, 
                        char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, 
                        const char *buf, size_t len, loff_t * off);



/*************** Sysfs functions **********************/
static ssize_t  sysfs_show(struct kobject *kobj, 
                        struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);

struct kobj_attribute etx_attr = __ATTR(etx_value,0660,sysfs_show,sysfs_store);

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .release        = etx_release,
};

// This function will be called when we read the sysfs file 
static ssize_t sysfs_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf){
    pr_info("[*] Sysfs- Read!!\n");
    return sprintf(buf, "%d", etx_value);
}

// This function will be called when we write to the sysfs 
static ssize_t sysfs_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count){
    pr_info("[*] Sysfs - Write!!\n");
    sscanf(buf, "%d",&etx_value);
    return count;
}

/*
** This function will be called when we open the Device file
*/ 

static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("[*] Device File Opened...!!!\n");
        return 0;
}

/*
** This function will be called when we close the Device file
*/ 

static int etx_release(struct inode *inode, struct file *file)
{
        pr_info("[*] Device File Closed...!!!\n");
        return 0;
}
 
/*
** This function will be called when we read the Device file
*/

static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("[*] Read function\n");
        return 0;
}

/*
** This function will be called when we write the Device file
*/

static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("[*] Write Function\n");
        return len;
}

static int __init etx_driver_init(void){
    // allocate the chrdev for Major number
    if((alloc_chrdev_region(&dev,0,1,"etx_DevSys")) < 0){
        pr_err("[-] Error allocating Major number\n");
        return -1;
    }
    pr_info("Major: %d\t Minor: %d\n",MAJOR(dev),MINOR(dev));
    
    // Create the cdev struct 
    cdev_init(&etx_cdev,&fops);

    // Add the character device to the system 
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_info("[-] Cannot add device to the system\n");
        goto r_class;
    }

    // creating struct class 
    if(IS_ERR(dev_class = class_create("etx_class"))){
        pr_err("[-] unable to create class\n");
        goto r_class;
    }
    // create the device 
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
        pr_err("[-] unable to create the device\n");
        goto r_device;
    }

    // Create a directory in /sys/kernel 
    kobj_ref = kobject_create_and_add("etx_sysfs",kernel_kobj);
    if(sysfs_create_file(kobj_ref,&etx_attr.attr)){
        pr_err("[-] Error creating sysfs file\n");
        goto r_sysfs;
    }
    if(request_irq(IRQ_NO,irq_handler,IRQF_SHARED,"etx_device",(void *)(irq_handler))){
        pr_err("our device: cannot register IRQ\n");
        goto irq;
    }
    tasklet = kmalloc(sizeof(struct tasklet_struct ),GFP_KERNEL);
    if(tasklet == NULL){
        pr_info("[-] Failed to allocate memory");
        goto irq;
    }
    tasklet_init(tasklet,tasklet_func,0);
    pr_info("[+] Driver create successfully\n");
    return 0;
irq:
    free_irq(IRQ_NO,(void *)(irq_handler));
r_sysfs:
    kobject_put(kobj_ref); // cleanup function
    sysfs_remove_file(kernel_kobj,&etx_attr.attr);
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    cdev_del(&etx_cdev);
    return -1;
}
static void __exit etx_driver_exit(void){
    tasklet_kill(tasklet);
    free_irq(IRQ_NO,(void *)(irq_handler));
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj,&etx_attr.attr);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev,1);
    pr_info("[+] clean up successfull\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("pratham popatiya <prathampopatiya17@gmail.com>");
MODULE_DESCRIPTION("Simple Linux device driver (sysfs)");
MODULE_VERSION("1.14");
