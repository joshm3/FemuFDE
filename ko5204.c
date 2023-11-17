#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/timekeeping.h>

#include <linux/wpt.h>

// Module metadata
MODULE_AUTHOR("Joshua Martin");
MODULE_DESCRIPTION("ko5204 - Linux Kernel Page Table Walker");
MODULE_LICENSE("GPL");


//Global struct proc_dir_entry pointer
static struct proc_dir_entry* proc_entry;

//custom write takes in 
static ssize_t custom_write(struct file* file, const char __user* va, size_t count, loff_t* offset)
{
    char* pa;
    u64 time;
    struct mm_struct *mm;
    u64 vaddr;
    u64 paddr;

    //first start time
    time = ktime_get_ns();

    //convert pa to va
    mm = current->mm;
    vaddr = (u64)(*va);
    paddr = hva2hpa(mm, vaddr);
    pa = (char *)paddr;

    //calculate total latency
    time = time - ktime_get_ns();

    //print to "/var/log/kern.log"
    printk(KERN_INFO "5204log,%s,%s,%llu\n",va,pa,time);
    
    return count;
}

static struct proc_ops my_fops={
    //.proc_read = custom_read,
    .proc_write = custom_write
};

// Custom init and exit methods
static int __init custom_init(void) {
    //change to "5204" in kernel version 6.0
    proc_entry = proc_create("5204k", 0222, NULL, &my_fops);
    printk(KERN_INFO "ko5204 driver loaded; /proc/5204 interface exposed");
    return 0;
}
static void __exit custom_exit(void) {
    proc_remove(proc_entry);
    printk(KERN_INFO "ko5204 driver removed; /proc/5204 interface removed");
}
module_init(custom_init);
module_exit(custom_exit);