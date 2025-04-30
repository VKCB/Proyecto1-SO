// CEthreads.c

#define _GNU_SOURCE
#include "CEthreads.h"
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>

#define STACK_SIZE (1024 * 1024) // 1MB

// Internal futex helpers
static int futex_wait(volatile int *addr, int expected) {
    return syscall(SYS_futex, addr, FUTEX_WAIT, expected, NULL, NULL, 0);
}

static int futex_wake(volatile int *addr) {
    return syscall(SYS_futex, addr, FUTEX_WAKE, 1, NULL, NULL, 0);
}

// Internal wrapper to call the thread function
static int CEthread_start(void* arg) {
    void** args = (void**)arg;
    void *(*start_routine)(void*) = args[0];
    void* real_arg = args[1];
    Car* thread = args[2];

    free(arg); // Free the wrapper memory

    start_routine(real_arg);

    // Mark as done
    thread->done = 1;
    futex_wake(&thread->done);

    return 0;
}

// Create a thread
int CEthread_create(Car* thread, void *(*start_routine)(void*), void* arg) {
    void* stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("malloc");
        return -1;
    }

    void** args = malloc(3 * sizeof(void*));
    if (!args) {
        perror("malloc");
        free(stack);
        return -1;
    }
    args[0] = start_routine;
    args[1] = arg;
    args[2] = thread;

    thread->done = 0;

    pid_t tid = clone(CEthread_start, stack + STACK_SIZE,
                      CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD,
                      args);

    if (tid == -1) {
        perror("clone");
        free(stack);
        free(args);
        return -1;
    }

    thread->tid = tid;
    thread->stack = stack;

    return 0;
}

// Wait for a thread to complete
int CEthread_join(Car* thread) {
    while (thread->done == 0) {
        futex_wait(&thread->done, 0);
    }
    free(thread->stack);
    return 0;
}

int CEmutex_init(CEMutex* mutex) {
    if (mutex == NULL) return -1;
    mutex->locked = 0; // Initially unlocked
    return 0;
}

int CEmutex_destroy(CEMutex* mutex) {
    if (mutex == NULL) return -1;
    // Nothing special to do, just set it to 0
    mutex->locked = 0;
    return 0;
}

int CEmutex_lock(CEMutex* mutex) {
    if (mutex == NULL) return -1;

    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        // Mutex already locked, wait
        futex_wait(&mutex->locked, 1);
    }
    return 0;
}


int CEmutex_unlock(CEMutex* mutex) {
    if (mutex == NULL) return -1;

    mutex->locked = 0;       // Unlock
    futex_wake(&mutex->locked);  // Wake up one waiting thread (if any)
    
    return 0;
}


// Exit thread
void CEthread_exit(void) {
    _exit(0);
}
