#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

// Driver functions 
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, 
                                char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, 
                                const char *buf, size_t len, loff_t * off);

                                
extern int etx_count; 
void etx_shared_func(void);  // function declaration is extern by default




//File operation structure
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .release        = etx_release,
};

static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("[*] Device File Opened...!!!\n");
        return 0;
}
  
static int etx_release(struct inode *inode, struct file *file)
{
        pr_info("[*] Device File Closed...!!!\n");
        return 0;
}
 

static ssize_t etx_read(struct file *filp, 
                                char __user *buf, size_t len, loff_t *off)
{
        etx_shared_func();
        pr_info("%d time() shared function called\n",etx_count);
        pr_info("[*] Data Read : Done!\n");
        return 1;
}

static ssize_t etx_write(struct file *filp, 
                                const char __user *buf, size_t len, loff_t *off)
{
        pr_info("[*] Data Write : Done!\n");
        return len;
}

static int __init etx_driver_init(void){

    // Allocate the Minor and Major Number
    if((alloc_chrdev_region(&dev,0,1,"etx_dev2")) < 0){
        pr_err("[-] Error allocating major number\n");
        return -1;
    }
    pr_info("[+] Major: %d\t Minor: %d\n",MAJOR(dev),MINOR(dev));
     
    // Create the cdev structure 
    cdev_init(&etx_cdev,&fops);

    // add the chrdev to the system
    if((cdev_add(&etx_cdev,dev,1))< 0){
        pr_err("[-] Error adding the chrdev\n");
        goto r_class;
    }
    // Create the class 
    if(IS_ERR(dev_class = class_create("etx_new_class2"))){
        pr_err("[-] Error creating the class\n");
        goto r_class;
    }
    // Create the device
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_Device2"))){
        pr_err("[-] Error creating the device\n");
        goto r_device;
    }
    pr_info("[+] Device Driver24_2 created successfully\n");
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
    pr_info("[+] cleanup done\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pratham Popatiya");
MODULE_DESCRIPTION("Exporting symbols and accessing to the global kernel table examples\n");
MODULE_VERSION("1.21");
