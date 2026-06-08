#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/fs.h>

// Creating a device with our choose mjaor and minor number wirh MKDEV macro
dev_t dev = MKDEV(335,0); // MKDEV(int major,int minor)

// Init fucntion
static int __init hello_world_init(void){
    /*
     * Now we need to register a char device 
     * using int register_chrdev_region(dev_t first, unsigned int count, char *name) 
     * first is the beginning device number of the range you would like to allocate
     * count is the total number of contiguous device numbers you are requesting 
     * name is the name of the device associated with the number range it will appear in /proc/devices/ and sysfs 
    */
    register_chrdev_region(dev,1,"New Device ---(*)--- ");
    pr_info("Major = %d \t Minor = %d \n ",MAJOR(dev),MINOR(dev)); // MAJOR and MINOR are macros to retrive major and minor from dev_t 
    pr_info("[+] Kernel Module inserted successfully\n");
    return 0;
}

static void __exit hello_world_exit(void){
    // unregister the char device at the time of cleanup 
    unregister_chrdev_region(dev,1);
    pr_info("[+] Kernel Module removed successfully\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pratham popatiya <prathampopatiya17@gmail.com>");
MODULE_DESCRIPTION("Simple linux driver statically allocating minor and major number for device");
MODULE_VERSION("1.0");
