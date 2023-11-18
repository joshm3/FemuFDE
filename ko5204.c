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

// Module metadata
MODULE_AUTHOR("Joshua Martin");
MODULE_DESCRIPTION("ko5204 - Linux Kernel Page Table Walker");
MODULE_LICENSE("GPL");

//Global struct proc_dir_entry pointer
static struct proc_dir_entry* proc_entry;

struct page* vaddr2page(struct mm_struct *mm, unsigned long addr)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep;
    struct page *page = NULL;
    spinlock_t *ptl;

    pgd = pgd_offset(mm, addr);
    if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd))) {
        printk("Coperd,invalid pgd [%p]\n", pgd);
        goto out;
    }

    p4d = p4d_offset(pgd, addr);
    if (p4d_none(*p4d) || unlikely(p4d_bad(*p4d))) {
        printk("Coperd, invalid p4d [%p]", p4d);
        goto out;
    }

    //problem...
    pud = pud_offset(p4d, addr);
    if (pud_none(*pud) || unlikely(pud_bad(*pud))) {
        printk("Coperd, invalid pud [%p]", pud);
        goto out;
    }

    pmd = pmd_offset(pud, addr);
    /* Coperd: TODO: sometimes pmd_bad() is true */
    if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd))) {
        printk("Coperd, invalid pmd [%p]", pmd);
        goto out;
    }

    //ptep = pte_offset_map(pmd, addr);
    ptep = pte_offset_map_lock(mm, pmd, addr, &ptl);
    if (!ptep) {
        printk("Coperd,%d,pt_offset_map() failed\n", __LINE__);
        goto out;
    }
	if(!pte_present(*ptep))
		goto unlock;
	page = pfn_to_page(pte_pfn(*ptep));
	if(!page)
		goto unlock;
	get_page(page);
unlock:
	pte_unmap_unlock(ptep,ptl);

    return page;

out:
    return NULL;
}


//return the paddr
static void find_page(unsigned long vaddr){
	struct mm_struct *mm = current->mm;
	struct page *page;
	unsigned long paddr;
	
	down_read(&mm->mmap_lock);
	page=vaddr2page(mm,vaddr);
	if(!page){
		printk(KERN_INFO "translation not found.\n");
		goto out;
	}
	paddr=(unsigned long)page_address(page);
	paddr+=(vaddr&~PAGE_MASK);
	printk(KERN_INFO "0x%lx\n", paddr);
out:
	up_read(&mm->mmap_lock);
}

//custom write takes in 
static ssize_t custom_write(struct file* file, const char __user* va, size_t count, loff_t* offset)
{
    //char* pa;
    u64 time;
    unsigned long vaddr;
    //u64 paddr;

    //first start time
    time = ktime_get_ns();

    //convert pa to va
    if(sscanf(va,"%lx",&vaddr)==1){
        printk(KERN_INFO "va is %lx\n",vaddr);
        find_page(vaddr);
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