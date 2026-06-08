#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include<linux/proc_fs.h>
#include <linux/err.h>

// adjust according to your kernel verison
#define LINUX_KERNEL_VERSION 700

// IOCTL commands
#define WR_VALUE _IOW('a','a',int32_t *)
#define RD_VALUE _IOR('a','b',int32_t *)

int32_t value = 0;
char etx_array[20] = "try_proc_array\n";
static int len = 1;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
static struct proc_dir_entry *parent;

static int      __init etx_driver_init(void);
static void     __exit etx_driver_exit(void);
/*************** Driver Functions **********************/
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 
/***************** Procfs Functions *******************/
static int      open_proc(struct inode *inode, struct file *file);
static int      release_proc(struct inode *inode, struct file *file);
static ssize_t  read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset);
static ssize_t  write_proc(struct file *filp, const char *buff, size_t len, loff_t * off);


static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = etx_open,
    .read = etx_read,
    .write = etx_write,
    .release = etx_release,
    .unlocked_ioctl = etx_ioctl,
};

#if (LINUX_KERNEL_VERSION > 505)

static struct proc_ops proc_fops = {
    .proc_open = open_proc,
    .proc_write = write_proc,
    .proc_read = read_proc,
    .proc_release = release_proc,
};

#else 
static struct proc_ops proc_fops = {
    .open = open_proc,
    .write = write_proc,
    .read = read_proc,
    .release = release_proc,
};

#endif

static int open_proc(struct inode *inode, struct file *file)
{
    pr_info("proc file opend.....\t");
    return 0;
}
/*
** This function will be called when we close the procfs file
*/
static int release_proc(struct inode *inode, struct file *file)
{
    pr_info("proc file released.....\n");
    return 0;
}

static ssize_t read_proc(struct file *filp,char __user *buffer,size_t length,loff_t * offset){
    pr_info("proc file read...\n");
    if(len){
        len = 0;
    }
    else{
        len = 1;
        return 0;
    }
    if(copy_to_user(buffer,etx_array,20)){
        pr_err("[-] Error sending data!!\n");
    }
    return length;
}

static ssize_t write_proc(struct file *filp,const char *buffer,size_t length,loff_t * offset){
    pr_info("proc write file...\n");
    if(copy_from_user(etx_array,buffer,length)){
        pr_err("[-] Error writing data..\n");
    }
    return length;
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
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read function\n");
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write Function\n");
        return len;
}

static long etx_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
    switch(cmd){
        case WR_VALUE:
            if(copy_from_user(&value,(int32_t *)arg,sizeof(value))){
                pr_err("[-] Error Writing data...\n");
            }
            pr_info("value = %d\n",value);
            break;
        case RD_VALUE:
            if(copy_to_user((int32_t *)arg,&value,sizeof(value))){
                pr_err("[-] Error reading data\n");
            }
            break;
        default:
            pr_info("Default..\n");
            break;
    }
    return 0;
}
static int __init etx_driver_init(void){
    
    // Allocate the chardevice 
    if((alloc_chrdev_region(&dev,0,1,"Etx_proc_dev")) < 0){
        pr_err("[-] Unable to allocate major number..\n");
        return -1;
    }
    pr_info("Major: %d\t Minor: %d\n",MAJOR(dev),MINOR(dev));

    cdev_init(&etx_cdev,&fops);

    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("[-] Cannot add the device...\n");
        goto r_class;
    }
    // Creating struct class 
    if(IS_ERR(dev_class = class_create("etx_proc_class"))){
        pr_err("[-] could not create class\n");
        goto r_class;
    }
    // Create the device 
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_Proc_Dev"))){
        pr_err("[-] Could not create device...\n");
        goto r_device;
    }
    
    // Create the proc directory
    parent = proc_mkdir("etx",NULL);
    
    if(parent == NULL){
        pr_err("Error creating dir...\n");
        goto r_device;
    }

    // Create the proc entry under /proc/etc
    proc_create("etx_proc",0666,parent,&proc_fops);
    pr_info("Device Driver with proc communication created successfully\n");
    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}

static void __exit etx_driver_exit(void){
    // remove the proc_entry 
    proc_remove(parent);

    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev,1);
    pr_info("[+] cleanup complete exiting...\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("pratham popatiya <prathampopatiya17@gmail.com>");
MODULE_DESCRIPTION("Simple Linux device driver (procfs)");
MODULE_VERSION("1.6");
