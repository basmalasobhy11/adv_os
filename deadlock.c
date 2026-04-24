#include <linux/module.h>      
#include <linux/kernel.h>      
#include <linux/init.h>        
#include <linux/kthread.h>     
#include <linux/mutex.h>       
#include <linux/delay.h>       

MODULE_LICENSE("GPL");         // Required for kernel module

static struct mutex lock1;     // First mutex lock
static struct mutex lock2;     // Second mutex lock
static int finished = 0;       // Counter to check completion

int thread1_func(void *arg)
{
    mutex_lock(&lock1);  // Thread 1 locks lock1 first
    printk("Thread 1: locked lock1\n");

    msleep(1000);        // Sleep to allow thread 2 to run

    mutex_lock(&lock2);  // Then tries to lock lock2 (may deadlock)
    printk("Thread 1: locked lock2\n");

    mutex_unlock(&lock2); // Unlock lock2
    mutex_unlock(&lock1); // Unlock lock1

    finished++;           // Mark completion

    return 0;
}

int thread2_func(void *arg)
{
    mutex_lock(&lock2);  // Thread 2 locks lock2 first
    printk("Thread 2: locked lock2\n");

    msleep(1000);        // Sleep to create overlap

    mutex_lock(&lock1);  // Then tries to lock lock1 (deadlock risk)
    printk("Thread 2: locked lock1\n");

    mutex_unlock(&lock1);
    mutex_unlock(&lock2);

    finished++;          // Mark completion

    return 0;
}

int monitor_func(void *arg)
{
    msleep(3000);        // Wait for threads to execute

    if (finished < 1)
    {
        printk("DEADLOCK DETECTED!\n"); // Not all threads finished
    }
    else
    {
        printk("No deadlock\n");
    }

    return 0;
}

static int __init deadlock_init(void)
{
    printk("Deadlock module loaded\n");

    mutex_init(&lock1);  // Initialize lock1
    mutex_init(&lock2);  // Initialize lock2

    // Create kernel threads
    kthread_run(thread1_func, NULL, "thread1");
    kthread_run(thread2_func, NULL, "thread2");
    kthread_run(monitor_func, NULL, "monitor");

    return 0;
}

static void __exit deadlock_exit(void)
{
    printk("Deadlock module removed\n");
}

/* Register init and exit functions */
module_init(deadlock_init);
module_exit(deadlock_exit);
