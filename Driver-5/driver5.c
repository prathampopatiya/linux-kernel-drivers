#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/err.h>
#include<linux/cdev.h>
#include<linux/device.h>


dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

// Function Prototypes
static int          __init etx_driver_init(void);
static void         __exit etx_driver_exit(void);
static int          etx_open(struct inode *inode,struct file *file);
static int          etx_release(struct inode *inode,struct file * file);
static ssize_t      etx_read(struct file *filp,char __user *buf,size_t len,loff_t *off);
static ssize_t      etx_write(struct file *filp,const char *buf,size_t len,loff_t *off);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = etx_read,
    .write = etx_write,
    .open = etx_open,
    .release = etx_release,
};

// This func will be called when we open the device
static int etx_open(struct inode *inode,struct file *file){
    pr_info("Driver open function called\n");
    return 0;
}
// This function will be called when we close the device 
static int etx_release(struct inode *inode,struct file *file){
    pr_info("Driver Release fucntion called\n");
    return 0;
}
// This func will be called when we try to read from the device Driver
static ssize_t etx_read(struct file *filp,char __user *buf,size_t len,loff_t *off){
    pr_info("Driver Read Function called\n");
    return 0;
}
static ssize_t etx_write(struct file *filp,const char *buf,size_t len,loff_t *off){
    pr_info("Driver write function called\n");
    return len;
}
static int __init etx_driver_init(void){
    
    // Allocate the char device 
    if((alloc_chrdev_region(&dev,0,1,"etx_DEvv")) < 0){
        pr_err("[-] Failed to Allocate Major number to device\n");
        return -1;
    }
    pr_info("Major: %d\t Minor: %d\n",MAJOR(dev),MINOR(dev));

    // Creating cdev structure
    cdev_init(&etx_cdev,&fops);

    // Adding the character device to the system
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("[-] Cannot add the device\n");
        goto r_class;
    }

    // Creating class and device from prev tutorial
    if(IS_ERR(dev_class = class_create("etx_Class"))){
        pr_err("[-] Cannot create the class struct\n");
        goto r_class;
    }
    // Creating device
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_Device"))){
        pr_err("[-] Cannot create the device\n");
        goto r_device;
    }
    pr_info("Device Driver Insert...Done!!\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}
static void __exit etx_driver_exit(void){
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev,1);
    pr_info("Device Driver removed... Done\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pratham popatiya");
MODULE_DESCRIPTION("Simple file operation on device drivers");
MODULE_VERSION("1.3");
