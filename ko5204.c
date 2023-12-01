#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/timekeeping.h>
#include <linux/pgtable.h>
#include <linux/mm.h>

#include <linux/kprobes.h>
#include <asm/cacheflush.h>
#include <asm/fixmap.h>

#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

//probably dont need most of these just got sick of finding them
#include <linux/kernel.h>
#include <linux/kvm_host.h>
#include <asm/virtext.h>
#include <asm/cpu.h>
#include <kvm/iodev.h>
#include <linux/kvm_host.h>
#include <linux/kvm.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/percpu.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/highmem.h>
#include <linux/file.h>
#include <linux/syscore_ops.h>
#include <linux/cpu.h>
#include <linux/sched/signal.h>
#include <linux/sched/mm.h>
#include <linux/sched/stat.h>
#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/anon_inodes.h>
#include <linux/profile.h>
#include <linux/kvm_para.h>
#include <linux/pagemap.h>
#include <linux/mman.h>
#include <linux/swap.h>
#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <linux/compat.h>
#include <linux/srcu.h>
#include <linux/hugetlb.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/bsearch.h>
#include <linux/io.h>
#include <linux/lockdep.h>
#include <linux/kthread.h>
#include <linux/suspend.h>
#include <linux/version.h>
#include <asm/processor.h>
#include <asm/ioctl.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

#define improvement

// Module metadata
MODULE_AUTHOR("Joshua Martin");
MODULE_DESCRIPTION("ko5204 - Linux Kernel Page Table Walker");
MODULE_LICENSE("GPL");

//Global struct proc_dir_entry pointer
static struct proc_dir_entry* proc_entry;
#ifdef improvement
static struct task_struct* bg_thread;
static struct mm_struct* mm_glob;
static unsigned long vaddr_glob;
#endif


static unsigned long vaddr2paddr(struct mm_struct *mm, unsigned long vaddr){
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep;
    unsigned long paddr = 0;
    struct page *page;

    pgd = pgd_offset(mm, vaddr);
    if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd))) {
        printk("Coperd,invalid pgd [%p]\n", pgd);
        goto out;
    }
    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d) || unlikely(p4d_bad(*p4d))) {
        printk("Coperd, invalid p4d [%p]", p4d);
        goto out;
    }
    pud = pud_offset(p4d, vaddr);
    if (pud_none(*pud) || unlikely(pud_bad(*pud))) {
        printk("Coperd, invalid pud [%p]", pud);
        goto out;
    }
    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd))) {
        printk("Coperd, invalid pmd [%p]", pmd);
        goto out;
    }

    if (!(ptep = pte_offset_map(pmd, vaddr)))
        goto out;
    if (!(page = pte_page(*ptep)))
        goto out;
    paddr = page_to_phys(page);
    pte_unmap(ptep);
out:
    paddr+=(vaddr&~PAGE_MASK);
    return paddr;
}

#ifdef improvement
//returns 1 upon accessed and 0 upon not accessed
static int vaddr2accessbit(struct mm_struct *mm, unsigned long vaddr){
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep;
    int accessBit = -1;
    //expect some errors for not accessed/allocated pte's
    pgd = pgd_offset(mm, vaddr);
    if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd))) {
        //printk("Coperd,invalid pgd [%p]\n", pgd);
        goto out;
    }
    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d) || unlikely(p4d_bad(*p4d))) {
        //printk("Coperd, invalid p4d [%p]", p4d);
        goto out;
    }
    pud = pud_offset(p4d, vaddr);
    if (pud_none(*pud) || unlikely(pud_bad(*pud))) {
        //printk("Coperd, invalid pud [%p]", pud);
        goto out;
    }
    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd))) {
        //printk("Coperd, invalid pmd [%p]", pmd);
        goto out;
    }

    if (!(ptep = pte_offset_map(pmd, vaddr)))
        goto out;
    //probably needs lock
    if (pte_present(*ptep)){
        accessBit = pte_young(*ptep);
        //not sure about this? 
        //resets the access bit but gets set right back
        ptep->pte &= ~(1ULL << 5);
    }
    pte_unmap(ptep);
out:
    return accessBit == 32;
}

/**Improve your kernel module to run a kernel thread in the 
    background, the thread needs to periodically scan the access 
    bits of all the 4KB pages in the 1GB buffer and aggregate 
    the access counts every 100ms.*/
int thread_function(void *v){
    unsigned long curVaddr;
    u64 startTime;
    u64 counter = 0;
    int i;
    // int* accessFreq = kzalloc(256*1024*sizeof(int), GFP_KERNEL);
    int* accessFreq = kzalloc(256*sizeof(int), GFP_KERNEL);

    printk(KERN_CONT "address,");
    for (curVaddr = vaddr_glob; curVaddr < vaddr_glob + 1024*1024*1024; curVaddr += 4096*1024){
        printk(KERN_CONT "%lu,", curVaddr);
    }
    printk(KERN_CONT "\n");

    startTime = ktime_get_real_ns();
    while(ktime_get_real_ns() < startTime + 60000000000){
        //loop through and check all accessBits
        for (curVaddr = vaddr_glob; curVaddr < vaddr_glob + 1024*1024*1024; curVaddr += 4096){
            // accessFreq[(curVaddr - vaddr_glob)/4096] += vaddr2accessbit(mm_glob, curVaddr);
            accessFreq[((curVaddr - vaddr_glob)/4096)/1024] += vaddr2accessbit(mm_glob, curVaddr);
        }

        //check if 100ms has passed and aggregation is needed
        if (ktime_get_real_ns() > startTime+counter*100000000){
            counter++;
            //aggregate just store the data and reset it?

            //check if 1s has passed and data needs to be logged
            if (counter % 10 == 0){
                //log all the last 10 100ms intervals data?
                printk(KERN_CONT ",%llu,", counter/10);
                // for(i = 0; i < 256*1024; i++){
                for(i = 0; i < 256; i++){
                    printk(KERN_CONT "%d,", accessFreq[i]/1024);
                    accessFreq[i] = 0;
                }
                printk(KERN_CONT "\n");
            }
        }
        //maximize the readings while still taking very small breaks
        usleep_range_state(1, 5, TASK_INTERRUPTIBLE); 
    }
    kfree(accessFreq);
    return 0;
}
#endif

//custom write takes in 
static ssize_t custom_write(struct file* file, const char __user* va, size_t count, loff_t* offset)
{
    //starting variables
    u64 time;
    unsigned long vaddr;
    unsigned long paddr;

    //first start time
    time = ktime_get_ns();

    //convert pa to va
    if(sscanf(va,"0x%lx",&vaddr)==1){
        //printk(KERN_INFO "vaddr is 0x%lx\n",vaddr);
        paddr = vaddr2paddr(current->mm, vaddr);
        if (paddr == 0){
            printk(KERN_ERR "virt2phys failed\n");
        }
    }

    //calculate total latency and convert to ms
    time = ktime_get_ns() - time;

    printk(KERN_ERR "starting bg thread\n");

#ifdef improvement
    //Improve your kernel module to run a kernel thread...
    mm_glob = current->mm;
    vaddr_glob = vaddr;
    bg_thread = kthread_create(thread_function,NULL,"background thread");
    if(bg_thread) {
        wake_up_process(bg_thread);
    } else {
        pr_err("Cannot create kthread\n");
    }
#endif

    //print to "/var/log/kern.log" remember time is in ns
    printk(KERN_ERR "5204log,%lx,%lx,%llu\n",vaddr,paddr,time);
    
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
    //kthread_stop(bg_thread); //not really necessary rn
    proc_remove(proc_entry);
    printk(KERN_INFO "ko5204 driver removed; /proc/5204 interface removed");
}
module_init(custom_init);
module_exit(custom_exit);





//should be time < 10*60
    // for (second = 0; second < 60; second++){ //600 100ms intervals
    //     accessCount = 0;
    //     for (time = 0; time < 10; time++){
    //         for (curVaddr = vaddr_glob; curVaddr < vaddr_glob + 1024*1024*1024; curVaddr += 4096){
    //             //printk(KERN_INFO "vaddr 0x%lx\n",curVaddr);
    //             //printk(KERN_INFO "bit accessed,%d\n",accessBit);
    //             accessCount += vaddr2accessbit(mm_glob, curVaddr);
    //         }
    //         msleep(90);   //delay for 100ms - runtime
    //     }
    //     printk(KERN_ERR "accessCount,%d\n",accessCount);
    // }
    // timet = ktime_get_ns() - timet;

    // printk(KERN_ERR "time for 60 seconds: %llu\n", timet/1000000000);





    


