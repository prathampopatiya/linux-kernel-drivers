#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/kdev_t.h>
#include<linux/err.h>
#include<linux/device.h> 
#include<linux/fs.h>

dev_t dev = 0;

/*
 * This is used to create the struct class for the device driver under /sys/class/. 
 * struct class * class_create(struct module *owner,const char *name) 
 * owner - pointer to the module that owns the struct class 
 * name pointer to the name of this class 
*/
static struct class *dev_class;

static int __init hello_world_init(void){

    // Allocate the char device dynamically
    if((alloc_chrdev_region(&dev,0,1,"etx_devv")) < 0){
        pr_err("[-] Failed to Allocate Major number for device\n");
        return -1;
    }
    pr_info("[+] Major: %d \t Minor: %d \t",MAJOR(dev),MINOR(dev));

    dev_class = class_create("etx_class");
    if(IS_ERR(dev_class)){
        pr_err("[-] Cannot create the struct class for the device\n");
        goto r_class;
    }
    // Create device this function is used by the char devices. A struct device will be created
    // in sysfs and registered to the specified class
    // struct device *device_create(struct class*,struct device *parent,dev_t dev,void *drvdata,const char *fmt,...);
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
        pr_err("[-] Cannot create the device\n");
        goto r_device;
    }
    pr_info("[+] Kernel module inserted successfully...\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}

static void __exit hello_world_exit(void){
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    unregister_chrdev_region(dev,1);
    pr_info("[-] Module removed successfully\n");
}
module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pratham popatiya <prathampopatiya17@gmail.com>");
MODULE_DESCRIPTION("Creating device file automatically");
MODULE_VERSION("1.2");
