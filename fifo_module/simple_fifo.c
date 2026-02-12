#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>

#define DEVICE_NAME "simple_fifo"
#define CLASS_NAME  "fifo_class"

#define FIFO_SIZE 256

#define IOCTL_PUSH _IOW('a', 'a', char *)
#define IOCTL_POP  _IOR('a', 'b', char *)

static int major_number;
static struct class*  fifo_class  = NULL;
static struct device* fifo_device = NULL;

static DEFINE_MUTEX(fifo_mutex);

static char buffer[FIFO_SIZE];
static int head = 0;
static int tail = 0;
static int count = 0;

static long fifo_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    char value;

    switch (cmd) {

    case IOCTL_PUSH:

        if (copy_from_user(&value, (char __user *)arg, sizeof(char)))
            return -EFAULT;

        mutex_lock(&fifo_mutex);

        if (count == FIFO_SIZE) {
            mutex_unlock(&fifo_mutex);
            return -EINVAL; // full
        }

        buffer[tail] = value;
        tail = (tail + 1) % FIFO_SIZE;
        count++;

        mutex_unlock(&fifo_mutex);
        break;

    case IOCTL_POP:

        mutex_lock(&fifo_mutex);

        if (count == 0) {
            mutex_unlock(&fifo_mutex);
            return -EINVAL; // empty
        }

        value = buffer[head];
        head = (head + 1) % FIFO_SIZE;
        count--;

        mutex_unlock(&fifo_mutex);

        if (copy_to_user((char __user *)arg, &value, sizeof(char)))
            return -EFAULT;

        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static int fifo_open(struct inode *inodep, struct file *filep)
{
    return 0;
}

static int fifo_release(struct inode *inodep, struct file *filep)
{
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = fifo_open,
    .release = fifo_release,
    .unlocked_ioctl = fifo_ioctl,
};

static int __init fifo_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0)
        return major_number;

    fifo_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(fifo_class))
        goto unregister;

    fifo_device = device_create(fifo_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(fifo_device))
        goto destroy_class;

    mutex_init(&fifo_mutex);

    printk(KERN_INFO "Simple FIFO module loaded\n");
    return 0;

destroy_class:
    class_destroy(fifo_class);
unregister:
    unregister_chrdev(major_number, DEVICE_NAME);
    return -1;
}

static void __exit fifo_exit(void)
{
    mutex_destroy(&fifo_mutex);
    device_destroy(fifo_class, MKDEV(major_number, 0));
    class_destroy(fifo_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Simple FIFO module unloaded\n");
}

module_init(fifo_init);
module_exit(fifo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple FIFO Kernel Module");
