#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/err.h>

//Timer Variable
#define TIMEOUT 3000    //milliseconds

static struct timer_list etx_timer;
static unsigned int count = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
static void timer_callback(struct timer_list *data);

/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);

static struct file_operations fops  = {
    .owner = THIS_MODULE,
    .read = etx_read,
    .write = etx_write,
    .release = etx_release,
    .open = etx_open
};

// When the timer expires this function will be called

static void timer_callback(struct timer_list *data){

    pr_info("Timer function callback here: %d\n",count++);

    // Renable the timer becuase it will be called only once.
    // By doing this it will be periodic 
    mod_timer(&etx_timer,jiffies + msecs_to_jiffies(TIMEOUT));
}

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
static ssize_t etx_read(struct file *filp, 
                                    char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function\n");
    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, 
                                const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}

static int __init etx_driver_init(void){

    if((alloc_chrdev_region(&dev,0,1,"etx_Dev")) < 0){
        pr_err("[-] Failed to allocate number\n");
        return -1;
    }
    pr_info("Major: %d\t Minor: %d\n",MAJOR(dev),MINOR(dev));

    // create the cdev struct
    cdev_init(&etx_cdev,&fops);

    // add the character device to the list
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("[-] Failed to add to the list\n");
        goto r_class;
    }
    // create the struct class
    if(IS_ERR(dev_class = class_create("etx_class"))){
        pr_err("[-] Failed to create class\n");
        goto r_class;
    }
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
        pr_err("Cannot create the Device \n");
        goto r_device;
    }

    // Setup the timer to call the call backfunction
    timer_setup(&etx_timer,timer_callback,0);
    // add_timer(&etx_timer);

    // setup the timer interval according to TIMEOUT macro
    mod_timer(&etx_timer,jiffies + msecs_to_jiffies(TIMEOUT));

    pr_info("[*] Device Driver Creation Done!!!\n");
    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}

static void __exit etx_driver_exit(void){

    timer_delete(&etx_timer); // del_timer,del_timer_sync has been depricated in newer kernels please refer the docs
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev,1);
    pr_info("[*] Driver cleanup successfull\n");
}
module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pratham Popatiya");
MODULE_DESCRIPTION("Timers in Linux kernel drivers");
MODULE_VERSION("1.18");