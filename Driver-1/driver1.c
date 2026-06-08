#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>

static int __init hello_world_init(void){
    printk(KERN_INFO "First Kernel Driver\n");
    printk(KERN_INFO "This is simple driver\n");
    pr_info("kernel module inserted successfully\n");
    return 0;
}

static void __exit hello_world_exit(void){
    pr_info("Kerned module removed successfully\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pratham popatiya <prathampopatiya17@gmail.com>");
MODULE_DESCRIPTION("A simple basic linux driver");
MODULE_VERSION("1:1.0");

