#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/crypto.h>
#include <linux/random.h>

#include <crypto/rng.h>

static dev_t first;

static struct cdev c_dev;
static struct class *c1;

// buffer size >= 131072 is required
static char result[131072];

// dummy open (no use)
static int my_open(struct inode *i, struct file *f) {
    printk(KERN_INFO "Driver: open\n");
    return 0;
}

// dummy close (no use)
static int my_close(struct inode *i, struct file *f) {
    printk(KERN_INFO "Driver: close\n");
    return 0;
}

// reverse a cstring
// kinh nghiệm 3 năm competitive programming tốn 10p tự hỏi tại sao hàm không chạy :)
// for (i = 0; i < len; i++) { ... }
// ngu người :)
static void reverse(char *str, long len) {
    long i;
    for (i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

// convert `value` from uint to cstring `buffer` with size `len`
static void my_itoa(uint value, char *buffer, long len) {
    long i = 0;
    while (i < len && value > 0) {
        int c = value % 10;
        value /= 10;
        buffer[i] = (char)(c + 48);
        ++i;
    }

    if (i < len) {
        buffer[i] = '\0';
    }

    reverse(buffer, i);
}

// handle read from an application from user-space
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "Driver: read (%lld)\n", *off);

    // generate a random value for unsigned int `i`
    uint i;
    // fill `sizeof(uint)` bytes of random values, start at &i
    get_random_bytes(&i, sizeof(uint));
    // convert to cstring
    my_itoa(i, result, sizeof(result));

    // debugging
    printk(KERN_INFO "Driver: Generated number: %u - %s\n", i, result);
    // copy from result to user buffer
    if (copy_to_user(buf, result, len) != 0) {
        // return exit code -14
        return -EFAULT;
    }

    // success
    return 0;
}

// dummy write (no use)
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "Driver: write\n");
    return len;
};

static struct file_operations pugs_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static int __init ofcd_init(void) {
    if (alloc_chrdev_region(&first, 0, 1, "Khang") < 0) {
        return -1;
    }

    if ((c1 = class_create(THIS_MODULE, "chardrv")) == NULL) {
        unregister_chrdev_region(first, 1);
        return -1;
    }

    if (device_create(c1, NULL, first, NULL, "rng") == NULL) {
        class_destroy(c1);
        unregister_chrdev_region(first, 1);
        return -1;
    }

    cdev_init(&c_dev, &pugs_fops);
    if (cdev_add(&c_dev, first, 1) == -1) {
        device_destroy(c1, first);
        class_destroy(c1);
        unregister_chrdev_region(first, 1);
        return -1;
    }

    printk(KERN_INFO "Driver: RNG registered");
    return 0;
}

static void __exit ofcd_exit(void) {
    cdev_del(&c_dev);
    device_destroy(c1, first);
    class_destroy(c1);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "Driver: RNG unregistered");
}

module_init(ofcd_init);
module_exit(ofcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Phuc Khang");
MODULE_DESCRIPTION("RNG Character Driver");
