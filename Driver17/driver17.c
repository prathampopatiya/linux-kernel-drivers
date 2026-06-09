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

/* Race Condition 
 * A race condition occurs when two or more threads can access shared data and they try to change
 * it at the same time. Because the thread scheduling algorithm can swap between threads at any time,
 * we don’t know the order in which the threads will attempt to access the shared data.
 *
 * Mutex :- A mutex is a mutual exclusion lock. Only one thread can hold the lock.
 * A mutex can be used to prevent the simultaneous execution of a block of code by multiple threads
 * that are running in single or multiple processes. 
 *
 * Mutex is used as a synchronization primitive in situations where a resource 
 * has to be shared by multiple threads simultaneously. 
 * A mutex has ownership. The thread that locks a Mutex must also unlock it.
 *
 * struct mutex{
 *  atomic_t count;
 *  spinclock_t wait_lock;
 *  struct list_head wait_list;
 * };
 * Again there are two methods to create mutexes 1. Static Method 2. Dynamic Method
 * DEFINE_MUTEX(name); -> Static method 
 * mutex_init(struct mutex *lock); This is used to lock/acquire the mutex exclusively for the current task.
 * If the mutex is not available, the current task will sleep until it acquires the Mutex. 
 *
 * The mutex must, later on, be released by the same task that acquired it. Recursive locking 
 * is not allowed.  The task may not exit without first unlocking the mutex.
 *
 * int mutex_lock_interruptible(struct mutex *lock); Locks the mutex like mutex_lock, and 
 * returns 0 if the mutex has been acquired or sleeps until the mutex becomes available. 
 * if a signal arrives while waiting for the lock then this function returns -EINTR. 
 *
 * int mutex_trylock(struct mutex *lock); This will try to acquire the mutex, without waiting 
 * (will attempt to obtain the lock, but will not sleep).  Returns 1 if the mutex has been 
 * acquired successfully, and 0 on contention.
 *
 * void mutex_unlock(struct mutex *lock); This is used to unlock/release a mutex that has been 
 * locked by a task previously. This function must not be used in an interrupt context.
 * Unlocking of a not locked mutex is not allowed.
 *
 * int mutex_is_locked(struct mutex *lock); This function is used to check whether mutex has been 
 * locked or not. 
 */ 

struct mutex etx_mutex;
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
        mutex_lock(&etx_mutex);
        etx_global_var++;
        pr_info("In thread_func %lu\n",etx_global_var);
        mutex_unlock(&etx_mutex);
        msleep(1000);
    }

    return 0;
}
int thread_func2(void *pv){
    while (!kthread_should_stop()) {
        mutex_lock(&etx_mutex);
        etx_global_var++;
        pr_info("In thread_func2 %lu\n",etx_global_var);
        mutex_unlock(&etx_mutex);
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
MODULE_VERSION("1.15");



