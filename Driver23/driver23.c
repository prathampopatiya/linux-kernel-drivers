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
#include <linux/completion.h>           // Required for the completion
#include <linux/err.h>

uint32_t read_count = 0;
static struct task_struct *wait_thread;

//DECLARE_COMPLETION(data_read_done);
struct completion data_read_done;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
int completion_flag = 0;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

static int etx_open(struct inode *inode,struct file *file);
static int etx_release(struct inode *inode,struct file *file);
static ssize_t etx_read(struct file *filp,char __user *buf,size_t len,loff_t *off);
static ssize_t etx_write(struct file *filp,const char *buf,size_t len,loff_t *off);


static struct file_operations fops = {
    .read = etx_read,
    .owner = THIS_MODULE,
    .write = etx_write,
    .release = etx_release,
    .open = etx_open,
};

static int wait_function(void *unused){

    while(1){
        pr_info("[*] Waiting for the event to occur..\n");
        wait_for_completion(&data_read_done);
        if(completion_flag == 2){
            pr_info("[*] Event came from exit function..\n");
            return 0;
        }
        pr_info("[*] Event came from read function - %d\n",++read_count);
        completion_flag = 0;
    }
    do_exit(0);
    return 0;
}

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
        pr_info("[*] Read Function\n");
        completion_flag = 1;
        if(!completion_done (&data_read_done)) {
            complete (&data_read_done);
        }
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("[*] Write function\n");
        return len;
}

static int __init etx_driver_init(void){

    if((alloc_chrdev_region(&dev, 0, 1,"etx_Dev")) < 0){
        pr_err("[-] Failed to allocate minor and major number..\n");
        return -1;
    }
    pr_info("[+] Major: %d\t Minor: %d\n",MAJOR(dev),MINOR(dev));

    // Creating the dev structure 
    cdev_init(&etx_cdev,&fops);
    etx_cdev.ops = &fops;
    
    // Add the char device to the system
    if(cdev_add(&etx_cdev,dev,1) < 0){
        pr_err("[-] Error adding the chrdev to system...\n");
        goto r_class;
    }
    // create the struct class
    if(IS_ERR(dev_class = class_create("etx_claass"))){
        pr_err("[-] Error creating class...\n");
        goto r_class;
    }
    // create the device
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
        pr_err("Cannot create the device\n");
        goto r_device;
    }

    // Create the kernel thread with name <thread_name>
    wait_thread = kthread_create(wait_function,NULL,"Wait_Thread");
    if(wait_thread){
        pr_info("[+] Thread Created successfully\n");
        wake_up_process(wait_thread); // start the thread
    }else{
        pr_err("[-] thread creationg failed\n");
    }

    // Initialize the completion 
    init_completion(&data_read_done); 
    
    pr_info("Device Driver created and inserted successfully\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}

static void __exit etx_driver_exit(void){
    
    completion_flag = 2;
    if(!completion_done(&data_read_done)){
        complete(&data_read_done);
    }
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
MODULE_DESCRIPTION("Static method for completion in linux kernel drivers\n");
MODULE_VERSION("1.19");