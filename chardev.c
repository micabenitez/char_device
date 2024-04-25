#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include <asm/errno.h>

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 80

static int major;

enum {
        CDEV_NOT_USED = 0,
        CDEV_EXCLUSIVE_OPEN = 1,
};

static char message[BUF_LEN + 1];

static struct class *cls;

static struct file_operations fops = {
        .read = device_read,
        .write = device_write,
        .open = device_open,
        .release = device_release,
};

static int device_open(struct inode *inode, struct file *file) {
        pr_info("device_open(%p)\n", file);

        try_module_get(THIS_MODULE);
        return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) { 
        pr_info("device_release(%p, %p)\n", inode, file);

        module_put(THIS_MODULE);
        return SUCCESS;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
	int bytes_read = 0;
	const char *message_ptr = message;
	
	int i = strlen(message) - 1;

        if (!*(message_ptr + *offset)) {
                *offset = 0;
                return 0;
        }

        message_ptr += *offset;

        while (i >= 0 && length && *message_ptr) {
                if(put_user(message[i--], buffer++) < 0) {
                	return -EFAULT;
                }
                length--;
		 bytes_read++;
        }
        
        *offset += bytes_read;

        return bytes_read;
}

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset) {
        int i;

        for (i = 0; i < length && i < BUF_LEN; i++) {
                get_user(message[i], buffer + i);
	}

        pr_info("Mensaje: %s\n", message);
        return i;
}

static int __init chardev_init(void) {
        int major = register_chrdev(0, DEVICE_NAME, &fops);

        if (major < 0) {
                return major;
        }

        cls = class_create(THIS_MODULE, DEVICE_NAME);
        device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	pr_info("El numero Mayor del controlador es %d.\n", major);
        pr_info("Para comunicarse con el char device, crear el archivo\n");
        pr_info("mknod %s c %d 0\n", DEVICE_NAME, major);
        return 0;
}

static void __exit chardev_exit(void) {
        device_destroy(cls, MKDEV(major, 0));
        class_destroy(cls);
        unregister_chrdev(major, DEVICE_NAME);
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");

