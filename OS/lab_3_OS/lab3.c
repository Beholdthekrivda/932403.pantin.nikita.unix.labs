#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/fs.h> 

static struct proc_dir_entry *procfile;

static ssize_t procfile_read(struct file *file, char __user *buffer, size_t buffer_length, loff_t *offset)
{
    char buf[128];
    size_t length;

    unsigned long uptime_seconds = jiffies / HZ;

    if (*offset != 0) return 0;

    length = snprintf(buf, sizeof(buf), "System uptime: %lu seconds\n", uptime_seconds);

    if (copy_to_user(buffer, buf, length)) return -EFAULT;

    *offset = length;
    return length;
}

static const struct proc_ops proc_file_ops = {
    .proc_read = procfile_read,
};

static int __init func_init(void)
{
    pr_info("Welcome to the Tomsk State University\n");

    procfile = proc_create(
        "tsulab", 0644, NULL, &proc_file_ops
    );

    if (!procfile) return -ENOMEM;

    return 0;
}

static void __exit func_exit(void)
{
    pr_info("Tomsk State University forever!\n");
    proc_remove(procfile);
}

module_init(func_init);
module_exit(func_exit);
MODULE_LICENSE("GPL");