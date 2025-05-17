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
#include <sys/prctl.h>
#include <string.h>
#include <limits.h>

#define STACK_SIZE (1024 * 1024) // 1MB

static int futex_wait(volatile int *addr, int expected) {
    return syscall(SYS_futex, addr, FUTEX_WAIT, expected, NULL, NULL, 0);
}

static int futex_wake(volatile int *addr) {
    return syscall(SYS_futex, addr, FUTEX_WAKE, 1, NULL, NULL, 0);
}

int CEthread_start(void* raw_args) {
    void** args = (void**)raw_args;
    void *(*start_routine)(void*) = args[0];
    void* arg = args[1];
    Car* thread = (Car*)args[2];

    Car* car = (Car*)arg;

    if (car) {
        const char* side = (car->lugar_inicio == LUGAR_IZQUIERDA) ? "IZQ" : "DER";
        const char* type = "UNK";
        switch (car->tipo) {
            case TIPO_NORMAL: type = "NORM"; break;
            case TIPO_SPORT: type = "SPRT"; break;
            case TIPO_PRIORITARIO: type = "PRIO"; break;
        }

        char name[32]; // Aumentamos el tamaño por seguridad
        snprintf(name, sizeof(name), "%s_%s_%d_%d", side, type, (int)car->velocidad, car->tiempo);
        prctl(PR_SET_NAME, name, 0, 0, 0);

    }

    void* ret = start_routine(arg);
    thread->done = 1;
    futex_wake(&thread->done); // Wake up thread that is waiting in CEthread_join
    free(raw_args);
    return (int)(intptr_t)ret;
}

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

    printf("Hilo creado: TID=%d\n", tid);
    return 0;
}

int CEthread_join(Car* thread) {
    while (thread->done == 0) {
        futex_wait(&thread->done, 0);
    }
    free(thread->stack);
    return 0;
}

int CEmutex_init(CEMutex* mutex) {
    if (mutex == NULL) return -1;
    mutex->locked = 0; 
    return 0;
}

int CEmutex_destroy(CEMutex* mutex) {
    if (mutex == NULL) return -1;
    mutex->locked = 0;
    return 0;
}

int CEmutex_lock(CEMutex* mutex) {
    if (mutex == NULL) return -1;
    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        futex_wait(&mutex->locked, 1);
    }
    return 0;
}

int CEmutex_unlock(CEMutex* mutex) {
    if (mutex == NULL) return -1;
    mutex->locked = 0;       
    futex_wake(&mutex->locked);  
    return 0;
}

void CEthread_exit(void) {
    exit(0);
}

void CECond_init(CECond* cond) {
    if (cond == NULL) return;
    cond->estado = 0;  
    cond->waiting = 0; 
}

void CECond_wait(CECond* cond, CEMutex* mutex) {
    if (cond == NULL || mutex == NULL) return;

    cond->waiting++;
    CEmutex_unlock(mutex);  

    while (cond->estado == 0) {
        futex_wait(&cond->estado, 0);
    }

    cond->waiting--;
    cond->estado = 0; 
    CEmutex_lock(mutex);  
}

void CECond_signal(CECond* cond) {
    if (cond == NULL) return;

    if (cond->waiting > 0) {
        cond->estado = 1;  
        futex_wake(&cond->estado);  
    }
}

void CECond_broadcast(CECond* cond) {
    if (cond == NULL) return;

    if (cond->waiting > 0) {
        cond->estado = 1;  
        futex_wake(&cond->estado);  
    }
}
