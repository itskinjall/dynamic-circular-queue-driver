#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/wait.h>

#define DEVICE_NAME "vicharak"
#define CLASS_NAME  "vicharak_class"

#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int *)
#define PUSH_DATA         _IOW('a', 'b', struct data_packet *)
#define POP_DATA          _IOR('a', 'c', struct data_packet *)

struct data_packet {
    int length;
    char *buffer;
};
static int major_number;
static struct class *vicharak_class = NULL;
static struct device *vicharak_device = NULL;

static DEFINE_MUTEX(queue_lock);

static char *queue_buffer = NULL;
static int queue_capacity = 0;

static int read_index = 0;
static int write_index = 0;
static int data_count = 0;

static wait_queue_head_t read_wait_queue;
static wait_queue_head_t write_wait_queue;
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/wait.h>

#define DEVICE_NAME "vicharak"
#define CLASS_NAME  "vicharak_class"

#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int *)
#define PUSH_DATA         _IOW('a', 'b', struct data_packet *)
#define POP_DATA          _IOR('a', 'c', struct data_packet *)

struct data_packet {
    int length;
    char *buffer;
};

/* 👇 STEP 2 CONTENT GOES HERE */
static int major_number;
static struct class *vicharak_class = NULL;
static struct device *vicharak_device = NULL;

static DEFINE_MUTEX(queue_lock);

static char *queue_buffer = NULL;
static int queue_capacity = 0;

static int read_index = 0;
static int write_index = 0;
static int data_count = 0;

static wait_queue_head_t read_wait_queue;
static wait_queue_head_t write_wait_queue;
static long vicharak_ioctl(struct file *file,
                           unsigned int cmd,
                           unsigned long arg)
{
    int i;
    int new_size;
    struct data_packet user_data;

    switch (cmd) {

    case SET_SIZE_OF_QUEUE:

        if (copy_from_user(&new_size, (int __user *)arg, sizeof(int)))
            return -EFAULT;

        mutex_lock(&queue_lock);

        if (queue_buffer)
            kfree(queue_buffer);

        queue_buffer = kmalloc(new_size, GFP_KERNEL);
        if (!queue_buffer) {
            mutex_unlock(&queue_lock);
            return -ENOMEM;
        }

        queue_capacity = new_size;
        read_index = 0;
        write_index = 0;
        data_count = 0;

        mutex_unlock(&queue_lock);
        break;


    case PUSH_DATA:

        if (copy_from_user(&user_data,
                           (struct data_packet __user *)arg,
                           sizeof(struct data_packet)))
            return -EFAULT;

        if (!queue_buffer)
            return -EINVAL;

        /* wait until enough space is available */
        wait_event_interruptible(write_wait_queue,
                                 data_count + user_data.length <= queue_capacity);

        mutex_lock(&queue_lock);

        for (i = 0; i < user_data.length; i++) {

            char temp_char;

            if (copy_from_user(&temp_char,
                               user_data.buffer + i,
                               sizeof(char))) {
                mutex_unlock(&queue_lock);
                return -EFAULT;
            }

            queue_buffer[write_index] = temp_char;
            write_index = (write_index + 1) % queue_capacity;
            data_count++;
        }

        mutex_unlock(&queue_lock);

        wake_up_interruptible(&read_wait_queue);
        break;


    case POP_DATA:

        if (copy_from_user(&user_data,
                           (struct data_packet __user *)arg,
                           sizeof(struct data_packet)))
            return -EFAULT;

        if (!queue_buffer)
            return -EINVAL;

        /* wait until enough data is available */
        wait_event_interruptible(read_wait_queue,
                                 data_count >= user_data.length);

        mutex_lock(&queue_lock);

        for (i = 0; i < user_data.length; i++) {

            char temp_char = queue_buffer[read_index];

            read_index = (read_index + 1) % queue_capacity;
            data_count--;

            if (copy_to_user(user_data.buffer + i,
                             &temp_char,
                             sizeof(char))) {
                mutex_unlock(&queue_lock);
                return -EFAULT;
            }
        }

        mutex_unlock(&queue_lock);

        wake_up_interruptible(&write_wait_queue);
        break;


    default:
        return -EINVAL;
    }

    return 0;
}
static struct file_operations vicharak_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = vicharak_ioctl,
};
static int __init vicharak_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &vicharak_fops);
    if (major_number < 0)
        return major_number;

    vicharak_class = class_create(CLASS_NAME);
    if (IS_ERR(vicharak_class))
        goto unregister_device;

    vicharak_device = device_create(vicharak_class,
                                     NULL,
                                     MKDEV(major_number, 0),
                                     NULL,
                                     DEVICE_NAME);

    if (IS_ERR(vicharak_device))
        goto destroy_class;

    mutex_init(&queue_lock);
    init_waitqueue_head(&read_wait_queue);
    init_waitqueue_head(&write_wait_queue);

    printk(KERN_INFO "vicharak driver loaded\n");
    return 0;

destroy_class:
    class_destroy(vicharak_class);

unregister_device:
    unregister_chrdev(major_number, DEVICE_NAME);
    return -1;
}
static void __exit vicharak_exit(void)
{
    if (queue_buffer)
        kfree(queue_buffer);

    device_destroy(vicharak_class, MKDEV(major_number, 0));
    class_destroy(vicharak_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    mutex_destroy(&queue_lock);

    printk(KERN_INFO "vicharak driver unloaded\n");
}
module_init(vicharak_init);
module_exit(vicharak_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("linux kernel developer role");
