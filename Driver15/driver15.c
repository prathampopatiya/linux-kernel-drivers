#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/slab.h> // kmalloc()
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/err.h>
#include<linux/uaccess.h> // copy from/to user

/* Creating threads in kernel
 * threads are lightweight process executing code inside the process
 * there are generally two types of threads
 * 1. kernel threads - created by OS
 * 2. user level threads- maintained and created by user level libs
 *
 * kernel thread management:-
 * 1.create kernel thread
 * 2.start kernel thread
 * 3.stop kernel thread
 * 4.other function in kernel thread
 *
 * struct task_struct * kthread_create(
 *      int (*threadfn)(void *data),
 *      void *data,
 *      const char namefmt[],
 *      ...
 * );
 *
 * this above api is used to create kernel threads
 *
 * this creates threads but we need to wake this thread manually.
 * When woken it will run the function in the argument
 *
 * int wake_up_process(struct task_struct *p);
 * returns 1 on success 0 on process already running
 *
 * int kthread_stop(struct task_struct *p);
 * stop the threads created by kthread_create
 *
 * void kthread_bind(struct task_struct *p,unsigned int cpu);
 * use to bind newly created threads to specific cpu.
 */

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static struct task_struct *etx_thread;


/* Function prototypes */
static int          __init etx_driver_init(void);
static void         __exit etx_driver_exit(void);

static int          etx_open(struct inode *inode,struct file *file);
static int          etx_release(struct inode *inode,struct file *file);
static ssize_t      etx_read(struct file *filp,char __user *buf,size_t len,
                             loff_t *off);
static ssize_t      etx_write(struct file *filp,const char *buf,size_t len,
                              loff_t *off);

int thread_func(void *pv);


int thread_func(void *pv)
{
    int i = 0;

    while(!kthread_should_stop())
    {
        pr_info("In thread_func %d\n",i++);
        msleep(1000);
    }

    return 0;
}


/* FileOps structure */
static struct file_operations fops =
{
    .owner   = THIS_MODULE,
    .open    = etx_open,
    .release = etx_release,
    .write   = etx_write,
    .read    = etx_read,
};


static int etx_open(struct inode *inode,struct file *file)
{
    pr_info("Device File Open called\n");
    return 0;
}

static int etx_release(struct inode *inode,struct file *file)
{
    pr_info("Device File Close called\n");
    return 0;
}

static ssize_t etx_read(struct file *filp,
                        char __user *buf,
                        size_t len,
                        loff_t *off)
{
    pr_info("[+] Data Read: Donee\n");
    return 0;
}

static ssize_t etx_write(struct file *filp,
                         const char *buf,
                         size_t len,
                         loff_t *off)
{
    pr_info("[+] Device Write: Donee\n");
    return len;
}


/* module init function */
static int __init etx_driver_init(void)
{
    if((alloc_chrdev_region(&dev,0,1,"etx_Mem_Dev")) < 0)
    {
        pr_err("[-] unable to allocate major number\n");
        return -1;
    }

    pr_info("Major: %d\t Minor: %d\n",
            MAJOR(dev),
            MINOR(dev));

    /* Creating the cdev structure for fileops */
    cdev_init(&etx_cdev,&fops);

    if((cdev_add(&etx_cdev,dev,1)) < 0)
    {
        pr_err("[-] Cannot add the device to system\n");
        goto r_cdev;
    }

    /* Creating struct class */
    dev_class = class_create("etx_Devicee");

    if(IS_ERR(dev_class))
    {
        pr_err("[-] Error Creating class\n");
        goto r_class;
    }

    /* Creating the device */
    if(IS_ERR(device_create(dev_class,
                            NULL,
                            dev,
                            NULL,
                            "etx_Devi")))
    {
        pr_err("[-] Error Creating device\n");
        goto r_device;
    }

    etx_thread = kthread_create(thread_func,
                                NULL,
                                "Etx_Thread");

    /* kthread_create returns ERR_PTR on failure */
    if(IS_ERR(etx_thread))
    {
        pr_err("[-] Cannot create thread\n");
        goto r_device;
    }

    wake_up_process(etx_thread);

#if 0
    /* you can also use this method to create and run the thread */
    etx_thread = kthread_run(thread_func,NULL,"Etx_Thread");

    if(IS_ERR(etx_thread))
    {
        pr_err("[-] unable to create thread\n");
        goto r_device;
    }

    pr_info("[+] Kthread created and running Successfully\n");
#endif

    pr_info("[+] Device Driver Installed Successfully\n");
    return 0;


/* Error handling */

r_device:
    device_destroy(dev_class,dev);

r_class:
    class_destroy(dev_class);

r_cdev:
    cdev_del(&etx_cdev);

r_unregister:
    unregister_chrdev_region(dev,1);

    return -1;
}


/* module exit function */
static void __exit etx_driver_exit(void)
{
    if(!IS_ERR_OR_NULL(etx_thread))
        kthread_stop(etx_thread);

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
MODULE_VERSION("1.13");



