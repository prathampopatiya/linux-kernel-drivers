#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>
#include <linux/wait.h>                 // Required for the wait queues
#include <linux/err.h>


// wait queue by static method 
//
uint32_t read_count = 0;
static struct task_struct * wait_thread;
 
//DECLARE_WAIT_QUEUE_HEAD(wait_queue_etx);

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
wait_queue_head_t wait_queue_etx;
int wait_queue_flag = 0;

static int      __init etx_driver_init(void);
static void     __exit etx_driver_exit(void);
 
/*************** Driver functions **********************/
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .release        = etx_release,
};

// Thread function for the queue 
static int wait_function(void *unused){
    while(1){
        pr_info("Waiting for an event...\n");
        // wait_event_interruptible(wq, condition);
        
        /*
         * The process is put to sleep (TASK_INTERRUPTIBLE) until the condition 
         * evaluates to true or a signal is received.
         * The condition is checked each time the waitqueue wq is woken up.
         * The function will return -ERESTARTSYS if it was interrupted by a signal 
         * and 0 if condition evaluated to true.
         */

        wait_event_interruptible(wait_queue_etx,wait_queue_flag != 0);
        if(wait_queue_flag == 2){
            pr_info("Event came from Exit Function..\n");
            return 0;
        }
        pr_info("Event came from a read function - %d\n",++read_count);
        wait_queue_flag = 0;
    }
    return 0;
}
/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read Function\n");
        wait_queue_flag = 1;
        // the waitqueue sleeps untill any event wakes it up 
        // wakes up only one process from the wait queue that is in interruptible sleep
        // wake_up_interruptible(&wq); wq –-> the waitqueue to wake up
        wake_up_interruptible(&wait_queue_etx);
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");
        return len;
}

static int __init etx_driver_init(void){
    // allocate the chrdev region with minor and major number dynamically
    if((alloc_chrdev_region(&dev,0,1,"etx_dev")) < 0){
        pr_err("[-] Error allocating Major number\n");
        return -1;
    }
    pr_info("Major: %d\t Minor: %d\n",MAJOR(dev),MINOR(dev));
    
    // init the cdev struct to register device as node
    cdev_init(&etx_cdev,&fops);
    etx_cdev.ops = &fops;

    // add the cdev to the system list 
    if((cdev_add(&etx_cdev,dev,1))< 0){
        pr_err("[-] Error adding cdev to list....\n");
        goto r_class;
    }
    // Create the dev class 
    if(IS_ERR(dev_class = class_create("Etx_dev_class"))){
        pr_info("Cannot add the device to the system\n");
        goto r_class;
    }
    // Create the device 
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_wait_device"))){
        pr_err("[-] unable to create the device at the moment...\n");
        goto r_device;
    }
    // Initialize the wait queue 
    init_waitqueue_head(&wait_queue_etx);

    // Create the kernel thread for the wait queue with <your_thread_name>
    wait_thread = kthread_create(wait_function,NULL,"WaitThreadd");
    if(wait_thread){
        pr_info("[+] thread create successfully\n");
        wake_up_process(wait_thread);
    }else{
        pr_err("[-] unable to create the thread\n");
    }
    pr_info("[+] Device Driver created and installed successfully\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}
static void __exit etx_driver_exit(void){
    wait_queue_flag = 2;
    wake_up_interruptible(&wait_queue_etx);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev,1);
    pr_info("[+] Cleanup done driver exiting....\n");
}
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("pratham popatiya <prathampopatiya17@gmail.com>");
MODULE_DESCRIPTION("Simple linux driver (Waitqueue Static method)");
MODULE_VERSION("1.8");
