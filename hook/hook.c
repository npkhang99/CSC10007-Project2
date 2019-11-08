#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/kallsyms.h>
#include <linux/fcntl.h>
#include <linux/mm_types.h>
#include <linux/slab.h>

#include <asm/cacheflush.h>
#include <asm/unistd.h>
#include <asm/pgtable_types.h>

/*sys_call_table address*/
void **system_call_table_addr;

/*original syscall prototype*/
asmlinkage long (*open_syscall) (const char __user *, int, int);
asmlinkage long (*write_syscall) (unsigned int, const char __user *, int);

void get_process_name(void) {
    char *pathname;
    char *p;
    struct mm_struct *mm = (void*) current->mm;
    if (mm) {
        down_read(&mm->mmap_sem);
        if (mm->exe_file) {
            pathname = (char*) kmalloc(PATH_MAX, GFP_ATOMIC);
            if (pathname) {
                p = d_path(&mm->exe_file->f_path, pathname, PATH_MAX);
                /*Now you have the path name of exe in p*/
            }
        }
        up_read(&mm->mmap_sem);
    }

    printk(KERN_INFO "Hook:    Process call: %s\n", p);
    kfree(pathname);
}

/*hook*/
asmlinkage long modified_open(const char __user * file, int flags, int mode) {
    printk(KERN_INFO "Hook: A file was opened\n");
    printk(KERN_INFO "Hook:    File: %s\n", file);
    get_process_name();
    return open_syscall(file, flags, mode);
}

asmlinkage long modified_write(unsigned int df, const char __user * buffer, int count) {
    printk(KERN_INFO "Hook: A `write` system call was called\n");
    // printk(KERN_INFO "Hook:    Content: %s\n", buffer);
    printk(KERN_INFO "Hook:    File descriptor: %u\n", df);
    printk(KERN_INFO "Hook:    Bytes writen: %d\n", count);
    get_process_name();
    return write_syscall(df, buffer, count);
}

/* Make page writeable */
int make_rw(unsigned long address) {
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    if (pte->pte &~ _PAGE_RW) {
        pte->pte |= _PAGE_RW;
    }
    return 0;
}

/* Make the page write protected */
int make_ro(unsigned long address) {
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte &~_PAGE_RW;
    return 0;
}

static int __init entry_point(void) {
    printk(KERN_INFO "Hook: Loading hook...\n");
    
    // the CORRECT way to get sys_call_table address
    system_call_table_addr = (void*) kallsyms_lookup_name("sys_call_table");
    printk(KERN_INFO "Hook: Get sys_call_table address success\n");
    
    open_syscall = system_call_table_addr[__NR_open];
    write_syscall = system_call_table_addr[__NR_write];
    printk(KERN_INFO "Hook: Get original syscall(s) success\n");
    
    make_rw((unsigned long) system_call_table_addr);
    printk(KERN_INFO "Hook: Disable page protection success\n");
    
    system_call_table_addr[__NR_open] = modified_open;
    system_call_table_addr[__NR_write] = modified_write;
    printk(KERN_INFO "Hook: Override original syscall(s) with modified one(s) success\n");
    
    printk(KERN_INFO "Hook: Hook loaded successfully...\n");
    return 0;
}

static void __exit exit_point(void) {
    printk(KERN_INFO "Hook: Unloading hook...\n");
    
    system_call_table_addr[__NR_open] = open_syscall;
    system_call_table_addr[__NR_write] = write_syscall;
    printk(KERN_INFO "Hook: Restore original syscall(s) success\n");
    
    make_ro((unsigned long) system_call_table_addr);
    printk(KERN_INFO "Hook: Re-enable page protection success\n");
    
    printk(KERN_INFO "Hook: Hook unloaded successfully...\n");
}

module_init(entry_point);
module_exit(exit_point);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Phuc Khang");
MODULE_DESCRIPTION("Hook to `open` and `write` system call");
