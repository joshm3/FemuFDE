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



// Module metadata
MODULE_AUTHOR("Joshua Martin");
MODULE_DESCRIPTION("ko5204 - Linux Kernel Page Table Walker");
MODULE_LICENSE("GPL");

//Global struct proc_dir_entry pointer
static struct proc_dir_entry* proc_entry;
static struct task_struct* bg_thread;
static struct mm_struct* mm_glob;
static unsigned long vaddr_glob;


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

static int vaddr2accessbit(struct mm_struct *mm, unsigned long vaddr){
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep;
    int accessBit;

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
    //probably needs lock
    if (pte_present(*ptep)){
        accessBit = pte_young(*ptep);
    }
    pte_unmap(ptep);
out:
    return accessBit;
}

/**Improve your kernel module to run a kernel thread in the 
    background, the thread needs to periodically scan the access 
    bits of all the 4KB pages in the 1GB buffer and aggregate 
    the access counts every 100ms.*/
int thread_function(void *v){
    unsigned long curVaddr;
    int time;
    int accessCount = 0;

    //should be time < 10*60
    for (time = 0; time < 1; time++){ //600 100ms intervals
        for (curVaddr = vaddr_glob; curVaddr < vaddr_glob + 1024*1024*1024; curVaddr += PAGE_SIZE){
            //printk(KERN_INFO "vaddr 0x%lx\n",curVaddr);
            //printk(KERN_INFO "bit accessed,%d\n",accessBit);
            if (vaddr2accessbit(mm_glob, curVaddr)){
                accessCount++;
            }
        }
        printk(KERN_INFO "accessCount,%d\n",accessCount);
        msleep(100);   //delay for 100ms
    }
    return 0;
}

//custom write takes in 
static ssize_t custom_write(struct file* file, const char __user* va, size_t count, loff_t* offset)
{
    //starting variables
    u64 time;
    unsigned long vaddr;
    unsigned long paddr;
    struct mm_struct *mm;
    int accessBit;

    // struct vm_area_struct *vma = 0;
    
    mm = current->mm;

    //first start time
    time = ktime_get_ns();
    
    //print all available virtual address to physical address mappings
    //  for the current process's memory map - unnecesary but cool?
    // for (vma = mm->mmap; vma; vma = vma->vm_next){
    //     for (vaddr = vma->vm_start; vaddr < vma->vm_end; vaddr += PAGE_SIZE){
    //         paddr = vaddr2paddr(mm, vaddr);
    //         if (paddr != 0){
    //             printk(KERN_INFO "vaddr 0x%lx paddr 0x%lx\n",vaddr, paddr);
    //         }
    //     }
    // }


    //convert pa to va
    if(sscanf(va,"0x%lx",&vaddr)==1){
        //printk(KERN_INFO "vaddr is 0x%lx\n",vaddr);
        
        paddr = vaddr2paddr(mm, vaddr);

        if (paddr == 0){
            printk(KERN_INFO "virt2phys failed\n");
        } else {
            //printk(KERN_INFO "paddr is 0x%lx\n", paddr);
        }

        accessBit = vaddr2accessbit(mm_glob, curVaddr);
        printk(KERN_INFO "bit accessed,%d\n",accessBit);
        printk(KERN_INFO "Pagesize:%d\n",PAGE_SIZE);
    }

    //calculate total latency and convert to ms
    time = ktime_get_ns() - time;

    //Improve your kernel module to run a kernel thread...
    mm_glob = current->mm;
    vaddr_glob = vaddr;
    bg_thread = kthread_run(thread_function,NULL,"bg thread");
    if(!bg_thread) {
        pr_err("Cannot create kthread\n");
    }

    //print to "/var/log/kern.log" remember time is in ns
    printk(KERN_INFO "5204log,%lx,%lx,%llu\n",vaddr,paddr,time);
    
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
    kthread_stop(bg_thread);
    proc_remove(proc_entry);
    printk(KERN_INFO "ko5204 driver removed; /proc/5204 interface removed");
}
module_init(custom_init);
module_exit(custom_exit);








    


