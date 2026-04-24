#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define MAX_LOCKS 10 
#define MAX_THREADS 10

/* graph[i][j] = 1 -> dependency edge i -> j */
static int graph[MAX_LOCKS][MAX_LOCKS] = { 0 };

/* each thread has id, list of locks it holds, and count of locks */
struct thread
{
	int id;
	int locks[MAX_LOCKS];
	int lock_count;
};

/* list of threads in the system */
static struct thread threads[MAX_THREADS];

/* find existing thread OR create new one */
static struct thread* get_thread(int id)
{
	int i;

	/* find existing thread */
	for (i = 0; i < MAX_THREADS; i++) {
		if (threads[i].id == id) {
			return &threads[i];
		}
	}

	/* create new thread if not found */
	for (i = 0; i < MAX_THREADS; i++) {
		if (threads[i].id == 0) {
			threads[i].id = id;
			threads[i].lock_count = 0;
			return &threads[i];
		}
	}

	return NULL;
}

/* DFS used to detect cycles in graph */
static bool DFS(int v, bool visited[], bool recStack[])
{
	int i;

	visited[v] = true;
	recStack[v] = true;

	for (i = 0; i < MAX_LOCKS; i++) {

		if (graph[v][i]) {

			/* if not visited, visit it */
			if (!visited[i] && DFS(i, visited, recStack)) {
				return true;
			}
			/* if in recursion stack, cycle detected */
			else if (recStack[i]) {
				return true;
			}
		}
	}

	recStack[v] = false; /* remove from recursion stack */
	return false;
}

/* check whole graph for deadlock (cycle) */
static bool detect_deadlock(void)
{
	bool visited[MAX_LOCKS] = { false };
	bool recStack[MAX_LOCKS] = { false };
	int i;

	for (i = 0; i < MAX_LOCKS; i++) {
		if (!visited[i]) {
			if (DFS(i, visited, recStack)) {
				return true; /* deadlock detected */
			}
		}
	}

	return false;
}

/* called when a thread acquires a lock */
static void lock_acquire(int thread_id, int lock_id)
{
	struct thread* t;
	int i;

	t = get_thread(thread_id);
	if (t == NULL)
		return;

	/* add dependency edges for all locks held by the thread */
	for (i = 0; i < t->lock_count; i++) {

		int prev_lock = t->locks[i]; /* prev is the lock already held */

		graph[prev_lock][lock_id] = 1; /* add edge prev_lock -> lock_id */

		printk(KERN_INFO "Thread %d acquires lock %d, %d -> %d\n",
			thread_id, lock_id, prev_lock, lock_id);
	}

	/* check for deadlock */
	if (detect_deadlock()) {
		printk(KERN_ALERT "Deadlock detected \n");
	}

	/* store lock in thread list */
	t->locks[t->lock_count++] = lock_id;
}

/* called when a thread releases a lock */
static void lock_release(int thread_id, int lock_id)
{
	struct thread* t;
	int i, j;

	t = get_thread(thread_id);
	if (t == NULL)
		return;

	/* remove dependency edges for the released lock */
	for (i = 0; i < t->lock_count; i++) {

		int prev_lock = t->locks[i];

		graph[prev_lock][lock_id] = 0;
	}

	/* remove lock_id from thread's lock list */
	for (i = 0; i < t->lock_count; i++) {

		if (t->locks[i] == lock_id) {

			for (j = i; j < t->lock_count - 1; j++) {
				t->locks[j] = t->locks[j + 1]; /* shift left */
			}

			t->lock_count--;
			break;
		}
	}

	printk(KERN_INFO "Thread %d releases lock %d\n",
		thread_id, lock_id);
}

/* ---------------- TEST USING KTHREADS ---------------- */

static struct task_struct* t1;
static struct task_struct* t2;

static int thread1_fn(void* data)
{
	lock_acquire(1, 1);
	msleep(100);
	lock_acquire(1, 2);
	return 0;
}

static int thread2_fn(void* data)
{
	lock_acquire(2, 2);
	msleep(100);
	lock_acquire(2, 1);
	return 0;
}

/* module init */
static int __init mini_lockdep_init(void)
{
	printk(KERN_INFO " module loaded\n");

	t1 = kthread_run(thread1_fn, NULL, "t1");
	t2 = kthread_run(thread2_fn, NULL, "t2");

	return 0;
}

/* module exit */
static void __exit mini_lockdep_exit(void)
{
	printk(KERN_INFO " module removed\n");
}

module_init(mini_lockdep_init);
module_exit(mini_lockdep_exit);

MODULE_LICENSE("GPL");