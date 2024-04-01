#include <linux/module.h>     /* Для всех модулей */
#include <linux/kernel.h>     /* KERN_INFO */
#include <linux/init.h>       /* Макросы */
#include <linux/fs.h>         /* Макросы для устройств */
#include <linux/cdev.h>       /* Функции регистрации символьных устройств */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maxim Khabarov");
MODULE_DESCRIPTION("nulldump --- device that returns EOF on read and write is always succesful with copy to dmesg.");
MODULE_VERSION("0.1");

static ssize_t nulldump_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    pr_info("nulldump: read 0 bytes\n");
    return 0;
}

#define BUFFER_SIZE (1024)

static ssize_t nulldump_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    char buffer[BUFFER_SIZE];
    size_t current_read;
    size_t written = 0;
    ssize_t res;

    pr_info("nulldump write start.\n");
    pr_info("pid: %u, command: %s\n", current->pid, current->comm);

    while (written < len) {
        current_read = (len - written >= BUFFER_SIZE ? BUFFER_SIZE : len - written);
        written += current_read;
        if (copy_from_user(buffer, buf, current_read) != 0) {
            res = -EFAULT;
            goto finish;
        }
        for (size_t i = 0; i < current_read; ++i) {
            pr_cont("%02x", buffer[i]);
        }
        buf += current_read;
    }

    res = written;

finish:
    pr_info("nulldump write finished. Written: %lu bytes.\n", written);

    return res;
}

static struct file_operations nulldump_ops =
{
    .owner      = THIS_MODULE,
    .read       = nulldump_read,
    .write      = nulldump_write,
};

dev_t dev = 0;
static struct cdev nulldump_cdev;
static struct class *nulldump_class;

static int __init chrdev_start(void)
{
    int res;

    if ((res = alloc_chrdev_region(&dev, 0, 1, "nulldump")) < 0)
    {
        pr_err("Error allocating major number\n");
        goto finish;
    }
    pr_info("CHRDEV load: Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

    cdev_init (&nulldump_cdev, &nulldump_ops);
    if ((res = cdev_add (&nulldump_cdev, dev, 1)) < 0)
    {
        pr_err("CHRDEV: device registering error\n");
        goto unregister_chrdev_region;
    }

    if (IS_ERR(nulldump_class = class_create ("nulldump_class")))
    {
        res = -1;
        goto cdev_del;
    }

    if (IS_ERR(device_create(nulldump_class, NULL, dev, NULL, "nulldump_device")))
    {
        pr_err("nulldump: error creating device\n");
        res = -1;
        goto class_destroy;
    }

    goto finish;

class_destroy:
    class_destroy(nulldump_class);
cdev_del:
    cdev_del(&nulldump_cdev);
unregister_chrdev_region:
    unregister_chrdev_region(dev, 1);
finish:
    return res;
}

static void __exit chrdev_end(void)
{
    device_destroy (nulldump_class, dev);
    class_destroy (nulldump_class);
    cdev_del (&nulldump_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("nulldump: unload\n");
}

module_init(chrdev_start);
module_exit(chrdev_end);
