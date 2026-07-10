#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>             //kernel threads
#include <linux/sched.h>               //task_struct 
#include <linux/delay.h>
#include <linux/seqlock.h>
#include <linux/err.h>
//Seqlock variable

seqlock_t etx_seq_lock;
unsigned long etx_global_variable = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

static struct task_struct *etx_thread1;
static struct task_struct *etx_thread2;

// Driver functions

static int etx_open(struct inode *inode,struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);

int thread1_function(void *pv);
int thread2_function(void *pv);

// Kernel thread functions 

int thread1_function(void *pv){

    while(!kthread_should_stop()){
        write_seqlock(&etx_seq_lock);
        etx_global_variable++;
        write_sequnlock(&etx_seq_lock);
        msleep(1000);
    }
    return 0;
}
int thread2_function(void *pv){
    unsigned int seq_no;
    unsigned long read_value;
    while(!kthread_should_stop()){
        do{
            seq_no = read_seqbegin(&etx_seq_lock);
            read_value = etx_global_variable;
        }while(read_seqretry(&etx_seq_lock,seq_no));
        pr_info("[*] in Thread2 Function: Read Value %lu\n",read_value);
        msleep(1000);
    }
    return 0;
}

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

    // Allocate the major and minor numbers
    if((alloc_chrdev_region(&dev,0,1,"Etx_seq_lockDev")) < 0){
        pr_err("[-] Error allocating Major Number\n");
        return -1;
    }
    pr_info("[+] Major:%d\t Minor: %d\n",MAJOR(dev),MINOR(dev));

    // Create the cdev
    cdev_init(&etx_cdev,&fops);

    // Add the cdev to the system
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("[-] Error adding the device to the system\n");
        goto r_class;
    }

    // Create the class 
    if(IS_ERR(dev_class = class_create("etx_seqlock_class"))){
        pr_err("[-] Error creating the class\n");
        goto r_class;
    }
    // Create the device
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etc_seqlock_device"))){
        pr_err("[-] Error creating the device\n");
        goto r_device;
    }
    // Create the threads 

    etx_thread1 = kthread_run(thread1_function,NULL,"etx_thread1");
    if(etx_thread1){
        pr_err("[+] Thread1 created and running successfully\n");
    }else{
        pr_err("[-] Error creating threads\n");
        goto r_device;
    }

    etx_thread2 = kthread_run(thread2_function,NULL,"etx_thread2");
    if(etx_thread2){
        pr_err("[+] Thread2 created and running successfully\n");
    }else{
        pr_err("[-] Error creating threads\n");
        goto r_device;
    }

    // Initialize the seqlock
    seqlock_init(&etx_seq_lock);

    pr_info("[+] Driver and threads created successfully\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    cdev_del(&etx_cdev);
    return -1;
}

static void __exit etx_driver_exit(void){
    
    kthread_stop(etx_thread1);
    kthread_stop(etx_thread2);
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
MODULE_DESCRIPTION("Atomic Variables in kernel drivers\n");
MODULE_VERSION("1.22");