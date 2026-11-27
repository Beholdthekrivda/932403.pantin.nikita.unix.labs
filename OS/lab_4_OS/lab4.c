#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h> 
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/random.h>

#define NUM_WORKS 6

struct lab_work
{
    struct work_struct work;
    unsigned int id;
    unsigned int stages;
    unsigned int completed;
};

static struct workqueue_struct *queue;
static struct lab_work *item[NUM_WORKS];

static int fib(unsigned int n)
{
    int a = 0, b = 1, c = 0;
    for (int i = 0; i < n; i++)
    {   
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

static void lab_work_handler(struct work_struct *work)
{   
    unsigned int stage;
    int fib_result;
    struct lab_work *it = container_of(work, struct lab_work, work);
    for (stage = 0; stage < it->stages; ++stage)
    {
        fib_result = fib(stage + 20);

        pr_info("work %u: stage %u/%u (Fib(%u) = %d)\n", it->id, stage + 1, it->stages, stage + 20, fib_result);

        msleep(1000);
        it->completed = stage + 1;
    }
    pr_info("work %u finished\n", it->id);
}

static int __init func_init(void)
{
    pr_info("Welcome to the Tomsk State University\n");

    queue = alloc_workqueue("tsulab_queue", WQ_UNBOUND, 2);
    if (!queue) return -ENOMEM;

    for (int i = 0; i < NUM_WORKS; i++)
    {
        item[i] = kmalloc(sizeof(*item[i]), GFP_KERNEL);
        if (!item[i]) return -ENOMEM;

        item[i]->id = i + 1;
        item[i]->stages = (get_random_u32() % 5) + 1;
        item[i]->completed = 0;

        INIT_WORK(&item[i]->work, lab_work_handler);
        if (!queue_work(queue, &item[i]->work))
        {
            pr_warn("Work %u was already in queue\n", item[i]->id);
        }
    }

    return 0;
}

static void __exit func_exit(void)
{
    pr_info("Tomsk State University forever!\n");

    if (queue)
    {
        flush_workqueue(queue);
        destroy_workqueue(queue);
    }
    
    for (int i = 0; i < NUM_WORKS; i++)
    {
        if (item[i]) 
        {
            kfree(item[i]);
        }
    }
}

module_init(func_init);
module_exit(func_exit);
MODULE_LICENSE("GPL");