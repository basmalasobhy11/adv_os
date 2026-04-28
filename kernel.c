#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/string.h>

#define MAX_LOCKS 10 
#define MAX_THREADS 10

/* graph[i][j] = 1 -> dependency edge i -> j */
static int graph[MAX_LOCKS][MAX_LOCKS] = { 0 };

/* each thread has id, list of locks it holds, and count */
struct thread {
	int id;
	int locks[MAX_LOCKS];
	int lock_count;
};

/* threads storage */
static struct thread threads[MAX_THREADS];

/* get or create thread */
static struct thread* get_thread(int id) {
	int i;

	for (i = 0; i < MAX_THREADS; i++) { // find existing thread	
		if (threads[i].id == id)
			return &threads[i];
	}

	for (i = 0; i < MAX_THREADS; i++) { // create new thread if not found
		if (threads[i].id == 0) {
			threads[i].id = id;
			threads[i].lock_count = 0;
			return &threads[i];
		}
	}
	return NULL;
}

/* DFS for cycle detection */
static bool dfs(int v, bool visited[], bool recStack[]) {
	int i;

	visited[v] = true;		// mark current node as visited
	recStack[v] = true;		// mark current node in recursion stack

	for (i = 0; i < MAX_LOCKS; i++) { // check all adjacent nodes
		if (graph[v][i]) {

			if (!visited[i] && dfs(i, visited, recStack)) // if not visited, visit it
				return true;

			else if (recStack[i]) // if in recursion stack, cycle detected
				return true; 
		}
	}

	recStack[v] = false;
	return false;
}

/* detect deadlock */
static bool detect_deadlock(void) { 
	bool visited[MAX_LOCKS] = { 0 };
	bool recStack[MAX_LOCKS] = { 0 };
	int i;

	for (i = 0; i < MAX_LOCKS; i++) { // check all nodes
		if (!visited[i]) {
			if (dfs(i, visited, recStack)) 
				return true;
		}
	}
	return false;
}

/* acquire lock */
static void mini_lock_acquire(int thread_id, int lock_id) { 
	struct thread* t;
	int i;

	t = get_thread(thread_id); 
	if (!t)
		return;

	/* first lock ( dependency yet) */
	if (t->lock_count == 0) {
		printk(KERN_INFO "MiniLockdep: Thread %d acquired Lock %d\n",thread_id, lock_id);
	}

	/* add dependency edges */
	for (i = 0; i < t->lock_count; i++) {
		int prev = t->locks[i]; // prev is the lock already held by the thread

		graph[prev][lock_id] = 1; // add edge prev -> lock_id

		printk(KERN_INFO "MiniLockdep: Thread %d acquired Lock %d (edge %d -> %d)\n",
			thread_id, lock_id, prev, lock_id);
	}

	/* detect state */
	if (detect_deadlock()) {
		printk(KERN_WARNING "MiniLockdep WARNING: DEADLOCK DETECTED!\n");
	}
	else {
		printk(KERN_INFO "MiniLockdep: SAFE (No cycle detected)\n");
	}

	t->locks[t->lock_count++] = lock_id; // store lock in thread list and increment lock count
}

/* release lock */
static void mini_lock_release(int thread_id, int lock_id) {
	struct thread* t;
	int i, j;

	t = get_thread(thread_id);
	if (!t)
		return;

	for (i = 0; i < t->lock_count; i++) { // remove dependency edges
		int prev = t->locks[i];
		graph[prev][lock_id] = 0;
	}

	for (i = 0; i < t->lock_count; i++) {
		if (t->locks[i] == lock_id) { // find lock in thread's lock list

			for (j = i; j < t->lock_count - 1; j++) // shift left to remove lock_id from list
				t->locks[j] = t->locks[j + 1]; 

			t->lock_count--; // decrement lock count
			break;
		}
	}

	printk(KERN_INFO "MiniLockdep: Thread %d released Lock %d\n",
		thread_id, lock_id);
}

/* ---------------- INIT ---------------- */
static int __init mini_lockdep_init(void) {

	printk(KERN_INFO "\n--- Test1 ---\n\n");

	/* Deadlock case */
	mini_lock_acquire(1, 1); 
	mini_lock_acquire(2, 2);
	mini_lock_acquire(1, 2);
	mini_lock_acquire(2, 1);

	/* reset system */
	memset(graph, 0, sizeof(graph)); 
	memset(threads, 0, sizeof(threads));

	printk(KERN_INFO "\n--- Test2 ---\n\n");

	/* Safe case */
	mini_lock_acquire(3, 1);
	mini_lock_acquire(3, 2);

	return 0;
}

/* ---------------- EXIT ---------------- */
static void __exit mini_lockdep_exit(void) {
	printk(KERN_INFO "MiniLockdep Module Removed\n"); #include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/string.h>

#define MAX_LOCKS 10 
#define MAX_THREADS 10

		/* graph[i][j] = 1 -> dependency edge i -> j */
		static int graph[MAX_LOCKS][MAX_LOCKS] = { 0 };

	/* each thread has id, list of locks it holds, and count */
	struct thread {
		int id;
		int locks[MAX_LOCKS];
		int lock_count;
	};

	/* threads storage */
	static struct thread threads[MAX_THREADS];

	/* get or create thread */
	static struct thread* get_thread(int id) {
		int i;

		for (i = 0; i < MAX_THREADS; i++) {
			if (threads[i].id == id)
				return &threads[i];
		}

		for (i = 0; i < MAX_THREADS; i++) {
			if (threads[i].id == 0) {
				threads[i].id = id;
				threads[i].lock_count = 0;
				return &threads[i];
			}
		}
		return NULL;
	}

	/* DFS for cycle detection */
	static bool dfs(int v, bool visited[], bool recStack[]) {
		int i;

		visited[v] = true;
		recStack[v] = true;

		for (i = 0; i < MAX_LOCKS; i++) {
			if (graph[v][i]) {

				if (!visited[i] && dfs(i, visited, recStack))
					return true;

				else if (recStack[i])
					return true; // cycle detected
			}
		}

		recStack[v] = false;
		return false;
	}

	/* detect deadlock */
	static bool detect_deadlock(void) {
		bool visited[MAX_LOCKS] = { 0 };
		bool recStack[MAX_LOCKS] = { 0 };
		int i;

		for (i = 0; i < MAX_LOCKS; i++) {
			if (!visited[i]) {
				if (dfs(i, visited, recStack))
					return true;
			}
		}
		return false;
	}

	/* acquire lock */
	static void mini_lock_acquire(int thread_id, int lock_id) {
		struct thread* t;
		int i;

		t = get_thread(thread_id);
		if (!t)
			return;

		/* first lock (no dependency yet) */
		if (t->lock_count == 0) {
			printk(KERN_INFO "MiniLockdep: Thread %d acquired Lock %d\n",
				thread_id, lock_id);
		}

		/* add dependency edges */
		for (i = 0; i < t->lock_count; i++) {
			int prev = t->locks[i];

			graph[prev][lock_id] = 1;

			printk(KERN_INFO "MiniLockdep: Thread %d acquired Lock %d (edge %d -> %d)\n",
				thread_id, lock_id, prev, lock_id);
		}

		/* detect state */
		if (detect_deadlock()) {
			printk(KERN_WARNING "⚠️ MiniLockdep WARNING: DEADLOCK DETECTED!\n");
		}
		else {
			printk(KERN_INFO "MiniLockdep: SAFE (No cycle detected)\n");
		}

		t->locks[t->lock_count++] = lock_id;
	}

	/* release lock */
	static void mini_lock_release(int thread_id, int lock_id) {
		struct thread* t;
		int i, j;

		t = get_thread(thread_id);
		if (!t)
			return;

		for (i = 0; i < t->lock_count; i++) {
			int prev = t->locks[i];
			graph[prev][lock_id] = 0;
		}

		for (i = 0; i < t->lock_count; i++) {
			if (t->locks[i] == lock_id) {

				for (j = i; j < t->lock_count - 1; j++)
					t->locks[j] = t->locks[j + 1];

				t->lock_count--;
				break;
			}
		}

		printk(KERN_INFO "MiniLockdep: Thread %d released Lock %d\n",
			thread_id, lock_id);
	}

	/* ---------------- INIT ---------------- */
	static int __init mini_lockdep_init(void) {

		printk(KERN_INFO "\n--- Running Deadlock Scenario ---\n\n");

		/* Deadlock case */
		mini_lock_acquire(1, 1);
		mini_lock_acquire(2, 2);
		mini_lock_acquire(1, 2);
		mini_lock_acquire(2, 1);

		/* reset system */
		memset(graph, 0, sizeof(graph));
		memset(threads, 0, sizeof(threads));

		printk(KERN_INFO "\n--- Running Safe Scenario ---\n\n");

		/* Safe case */
		mini_lock_acquire(3, 1);
		mini_lock_acquire(3, 2);

		return 0;
	}

	/* ---------------- EXIT ---------------- */
	static void __exit mini_lockdep_exit(void) {
		printk(KERN_INFO "MiniLockdep Module Removed\n");
	}

	module_init(mini_lockdep_init);
	module_exit(mini_lockdep_exit);

	MODULE_LICENSE("GPL");
}

module_init(mini_lockdep_init);
module_exit(mini_lockdep_exit);

MODULE_LICENSE("GPL")
