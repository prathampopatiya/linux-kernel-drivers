#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/err.h>
#include<linux/uaccess.h> // copy from/to user
#include<linux/mutex.h>

/*   
 * Read-write spin locks 
 * why do we need read-write spin locks. now let's assume a scenario where there are 4 threads 
 * and 1 thread it trying to update the variable while others are trying to read it.
 *
 * assume that thread-1 is lost and threads 2-4 are trying to read the variable in they are 
 * trying to lock for read operation. guess what this redcues perfomance because they are spinning 
 * for the same thing variables value is not gonna change because threads are trying to read only 
 *
 * working of read-write spinlock 
 * 1. When there is no thread in the critical section, any reader or writer thread can enter 
 * into a critical section. respective read or write lock. But only one thread can enter into 
 * a critical section.
 *
 * 2.if the reader thread is in the critical section, the new reader thread can enter arbitrarily, 
 * but the writer thread cannot enter. The writer thread has to wait until all the reader thread 
 * finishes their process.
 *
 * 3. If the writer thread is in the critical section, no reader thread or writer thread can enter.
 * 
 * 4. If one or more reader threads are in the critical section by taking its lock,  the writer thread 
 * can of course not enter the critical section, but the writer thread cannot prevent the entry of 
 * the subsequent read thread.He has to wait until the critical section has a reader thread. 
 * So this read-write spinlock is giving importance to the reader thread and not the writer thread.
 * If you want to give importance to the writer thread than the reader thread, then another 
 * lock is available in Linux which is seqlock.
 * 
 * static method to create r-w-spin locks -> DEFINE_RWLOCK(etx_rwlock);
 * rwlock_t etx_rwlock; rwlock_init(&etx_rwlock); -> Dynamic method 
 *
 * Method-1 :- If you share data with user context (between Kernel Threads), then you can use this approach. 
 * read_lock(rwlock_t *lock); -> locks read_unlock(rwlock_t *lock) -> unlocks 
 * write_lock(rwlock_t *lock) -> locks write_unloock(rwlock_t * lock) -> unlocks 
 *
 * Method-2 :- Locking b/w bottom halves same as method-1 
 * Method-3 :- Locking b/w user context with bottom half this use this 
 * read_lock_bn(rwlock_t *lock); read_unlock_bh(rwlock_t *lock); same for write just add _bh suffix
 *
 * Method-4 :-  Locking b/w irq's if you share data with hardware ISR IRQ's needs to be disabled. 
 * read_lock_irq(rwlock_t *lock); read_unlock_irq(rwlock_t *lock);
 * 
 */ 

// static method 
static DEFINE_RWLOCK(etx_rwlock);
// Dynamic method
//rwlock_t etx_rwlock;
unsigned long etx_global_var = 0;


dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static struct task_struct *etx_thread;
static struct task_struct *etx_thread1;

/* Function prototypes */
static int          __init etx_driver_init(void);
static void         __exit etx_driver_exit(void);

static int          etx_open(struct inode *inode,struct file *file);
static int          etx_release(struct inode *inode,struct file *file);
static ssize_t      etx_read(struct file *filp,char __user *buf,size_t len,
                             loff_t *off);
static ssize_t      etx_write(struct file *filp,const char *buf,size_t len,
                              loff_t *off);

int thread_func1(void *pv);
int thread_func2(void *pv);


int thread_func1(void *pv)
{
    while(!kthread_should_stop())
    {
        write_lock(&etx_rwlock);
        etx_global_var++;
        pr_info("In thread_func %lu\n",etx_global_var);
        write_unlock(&etx_rwlock);
        msleep(1000);
    }

    return 0;
}
int thread_func2(void *pv){
    while (!kthread_should_stop()) {
        read_lock(&etx_rwlock);
        pr_info("In thread_func2 %lu\n",etx_global_var);
        read_unlock(&etx_rwlock);
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

    etx_thread = kthread_create(thread_func1,
                                NULL,
                                "Etx_Thread");

    /* kthread_create returns ERR_PTR on failure */
    if(IS_ERR(etx_thread))
    {
        pr_err("[-] Cannot create thread\n");
        goto r_device;
    }
    etx_thread1 = kthread_run(thread_func2,NULL,"etx_thread2");
    if(etx_thread1){
        pr_info("[+] thread2 created and running successfully!!\n");
    }else{
        pr_err("[-] unable to create thread2 ) - - )\n");
        goto r_device;
    }

    wake_up_process(etx_thread);

#if 0
    /* you can also use this method to create and run the thread */
    etx_thread = kthread_run(thread_func1,NULL,"Etx_Thread");

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
    kthread_stop(etx_thread);
    kthread_stop(etx_thread1);
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
MODULE_VERSION("1.16");

