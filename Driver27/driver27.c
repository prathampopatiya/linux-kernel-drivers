#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

// This function will be called when we open the Misc device file
static int etx_misc_open(struct inode *inode, struct file *file)
{
    pr_info("[*] misc device open\n");
    return 0;
}

// This function will be called when we close the Misc Device file

static int etx_misc_close(struct inode *inodep, struct file *filp)
{
    pr_info("[*] misc device close\n");
    return 0;
}

// This function will be called when we write the Misc Device file
static ssize_t etx_misc_write(struct file *file, const char __user *buf,
               size_t len, loff_t *ppos)
{
    pr_info("[*] misc device write\n");    
    return len; 
}
 

// This function will be called when we read the Misc Device file

static ssize_t etx_misc_read(struct file *filp, char __user *buf,
                    size_t count, loff_t *f_pos)
{
    pr_info("[*] misc device read\n");
    return 0;
}

static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .write          = etx_misc_write,
    .read           = etx_misc_read,
    .open           = etx_misc_open,
    .release        = etx_misc_close,
    .llseek         = noop_llseek,
};

struct miscdevice etx_misc_device = {
    .minor =  MISC_DYNAMIC_MINOR,
    .name = "simple_misc_driver",
    .fops = &fops,
};

static int __init misc_init(void){
    int error;

    error = misc_register(&etx_misc_device);
    if(error){
        pr_err("[-] Error registering device\n");
        return error;
    }
    pr_info("[+] misc driver registeres successfully\n");
    return 0;
}

static void __exit misc_exit(void){
    misc_deregister(&etx_misc_device);
    pr_info("[+] Cleaning up.....\n");
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pratham Popatiya");
MODULE_DESCRIPTION("Misc Driver examples in kernel Drivers\n");
MODULE_VERSION("1.24");