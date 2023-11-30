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
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/reboot.h>
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


unsigned long vaddr2paddr(struct mm_struct *mm, unsigned long virt){
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep;
    unsigned long phys = 0;
    struct page *page;

    pgd = pgd_offset(mm, virt);
    if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd))) {
        printk("Coperd,invalid pgd [%p]\n", pgd);
        goto out;
    }
    p4d = p4d_offset(pgd, virt);
    if (p4d_none(*p4d) || unlikely(p4d_bad(*p4d))) {
        printk("Coperd, invalid p4d [%p]", p4d);
        goto out;
    }
    pud = pud_offset(p4d, virt);
    if (pud_none(*pud) || unlikely(pud_bad(*pud))) {
        printk("Coperd, invalid pud [%p]", pud);
        goto out;
    }
    pmd = pmd_offset(pud, virt);
    if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd))) {
        printk("Coperd, invalid pmd [%p]", pmd);
        goto out;
    }

    if (!(ptep = pte_offset_map(pmd, virt)))
        goto out;
    if (!(page = pte_page(*ptep)))
        goto out;
    phys = page_to_phys(page);
    pte_unmap(ptep);
out:
    return phys;
}

//custom write takes in 
static ssize_t custom_write(struct file* file, const char __user* va, size_t count, loff_t* offset)
{
    //starting variables
    u64 time;
    unsigned long vaddr;
    unsigned long paddr;
    struct mm_struct *mm;

    // struct vm_area_struct *vma = 0;
    
    mm = current->mm;

    //first start time
    time = ktime_get_ns();
    
    //print all available virtual address to physical address mappings
    //  for the current process's memory map
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
        printk(KERN_INFO "vaddr is 0x%lx\n",vaddr);
        *****DOES NOT INCLUDE OFFSET
        paddr = vaddr2paddr(mm, vaddr);
        if (paddr == 0){
            printk(KERN_INFO "virt2phys failed\n");
        } else {
            printk(KERN_INFO "paddr is 0x%lx\n", paddr);
        }
    }

    //calculate total latency
    time = time - ktime_get_ns();

    //print to "/var/log/kern.log"
    //printk(KERN_INFO "5204log,%s,%s,%llu\n",va,pa,time);
    
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








    


