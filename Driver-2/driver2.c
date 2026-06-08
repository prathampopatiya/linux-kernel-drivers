#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/moduleparam.h>

// create the parameters variables
int valueEtx,arr_valueEtx[4];
char *nameEtx;
int cb_valueEtx = 0;

// assign the parameters variables with necessary permissons and type to parameter macro
module_param(valueEtx,int,S_IRUSR | S_IWUSR); // this will create an entry in /sys/module/driver2/parameters/valueEtx
module_param(nameEtx,charp,S_IRUSR | S_IWUSR);
module_param_array(arr_valueEtx,int,NULL,S_IRUSR | S_IWUSR);

/*
 * If you want to get a notification whenever the value got to change of a certain parameter 
 * we need to register to our handler function to its file operation structure first   
*/


// callback function when value of callback variable changes
int notify_param(const char *val,const struct kernel_param *kp){
    int res = param_set_int(val,kp);
    if(res == 0){
        pr_info("Callback function called...\n");
        pr_info("New value of cb_valueEtx: %d\n",cb_valueEtx);
        return 0;
    }
    return -1;
}

const struct kernel_param_ops my_param_ops = {
    .set = &notify_param,
    .get = &param_get_int
};

module_param_cb(cb_valueEtx,&my_param_ops,&cb_valueEtx,S_IRUGO | S_IWUSR);

// module init functions

static int __init hello_world_init(void){
    int i;
    pr_info("valueEtx = %d\n",valueEtx);
    pr_info("nameEtx = %s\n",nameEtx);
    pr_info("cb_valueEtx = %d\n",cb_valueEtx);
    for(i = 0;i < (sizeof(arr_valueEtx) / sizeof(int));i++){
        pr_info("arr_valueEtx[%d] = %d\n",i,arr_valueEtx[i]);
    }
    pr_info("[-] Kernel module inserted successfully\n");
    return 0;
}
static void __exit hello_world_exit(void){
    pr_info("[+] Kernel module removed successfully\n");
}
module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pratham popatiya <prathampopatiya17@gmail.com>");
MODULE_DESCRIPTION("a simple parameter passing test module");
MODULE_VERSION("1.0");





