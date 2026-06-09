#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/slab.h> // kmalloc()
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/device.h> 
#include<linux/err.h> 
#include<linux/uaccess.h> // copy from/to user 


#define MEMSIZE 1024

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
uint8_t *kernel_buffer;

// Function prototypes
static int          __init etx_driver_init(void);
static void         __exit etx_driver_exit(void);

static int          etx_open(struct inode *inode,struct file *file);
static int          etx_release(struct inode *inode,struct file *file);
static ssize_t      etx_read(struct file *filp,char __user *buf,size_t len,loff_t *off);
static ssize_t      etx_write(struct file *filp,const char *buf,size_t len,loff_t *off);

// FileOps structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = etx_open,
    .release = etx_release,
    .write = etx_write,
    .read = etx_read,
};

static int  etx_open(struct inode *inode,struct file *file){
    pr_info("Device File Open called\n");
    return 0;
}
static int etx_release(struct inode *inode,struct file *file){
    pr_info("Device File Close called\n");
    return 0;
}

static ssize_t  etx_read(struct file *filp,char __user *buf,size_t len,loff_t *off){
    if(copy_to_user(buf,kernel_buffer,MEMSIZE)){
        pr_err("[-] Error reading data!!\n");
    }
    pr_info("[+] Data Read: Donee \n");
    return MEMSIZE;
}
static ssize_t etx_write(struct file *filp,const char *buf,size_t len,loff_t *off){
    if(copy_from_user(kernel_buffer,buf,len)){
        pr_err("[-] Error writing data!!!!\n");
    }
    pr_info("[+] Device Write: Donee \n");
    return len;
}
// module init function 
static int __init etx_driver_init(void){
    if((alloc_chrdev_region(&dev,0,1,"etx_Mem_Dev"))< 0){
        pr_err("[-] unable to allocate major number\n");
        return -1;
    }
    pr_info("Major: %d\t Minor: %d\n",MAJOR(dev),MINOR(dev));
    
    // Creating the cdev structure for fileops 
    cdev_init(&etx_cdev,&fops);
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("[-] Cannot add the device to system\n");
        goto r_class;
    }
    // Creating struct class 
    if(IS_ERR(dev_class = class_create("etx_Devicee"))){
        pr_err("[-] Error Creating class\n");
        goto r_class;
    }
    // Creating the device 
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_Devicee"))){
        pr_err("[-] Error Creating device\n");
        goto r_device;
    }
    if((kernel_buffer = kmalloc(MEMSIZE,GFP_KERNEL)) == 0){
        pr_err("[-] Cannot allocate memory!!\n");
        goto r_device;
    }
    strcpy(kernel_buffer, "hello universe!!");
    pr_info("[+] Device Driver Installed Successfully\n");
    return 0;
r_device:
    device_destroy(dev_class,dev);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}
static void __exit etx_driver_exit(void){
    kfree(kernel_buffer);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev,1);
    pr_info("[+] Cleanup done!!\n");
}
module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pratham popatiya");
MODULE_DESCRIPTION("Simple device Driver allocating memory and reading from/to userspace");
MODULE_VERSION("1.4");




