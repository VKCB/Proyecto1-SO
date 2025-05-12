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
<<<<<<<< HEAD:CEthreads/CEthreads.c
#include <sys/prctl.h>
#include <string.h>
========
#include <limits.h> // Agregado para INT_MAX
>>>>>>>> calendarizacion:biblioteca/CEthreads.c

#define STACK_SIZE (1024 * 1024) // 1MB

// Internal futex helpers
static int futex_wait(volatile int *addr, int expected) {
    return syscall(SYS_futex, addr, FUTEX_WAIT, expected, NULL, NULL, 0);
}

static int futex_wake(volatile int *addr) {
    return syscall(SYS_futex, addr, FUTEX_WAKE, 1, NULL, NULL, 0);
}

static int futex_wake_all(volatile int *addr) {
    return syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
}

// Internal wrapper to call the thread function
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

        char name[16];
        snprintf(name, sizeof(name), "%s_%s_%d", side, type, (int)car->velocidad);
        prctl(PR_SET_NAME, name, 0, 0, 0);
        printf("✅ Nombre del hilo configurado: %s\n", name); // Depuración
    }

    void* ret = start_routine(arg);
    thread->done = 1;
    free(raw_args);
    return (int)(intptr_t)ret;
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

    printf("✅ Hilo creado: TID=%d\n", tid); // Depuración
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

// Inicializa una condición
void CECond_init(CECond* cond) {
    cond->estado = 0;  // Inicialmente no señalada
    cond->waiting = 0; // No hay hilos esperando
}

// Espera a que la condición sea señalada
void CECond_wait(CECond* cond, CEMutex* mutex) {
    // Incrementa el contador de hilos esperando
    __sync_fetch_and_add(&cond->waiting, 1);

    // Libera el mutex externo
    CEmutex_unlock(mutex);

    // Espera mientras la condición no esté señalada
    while (cond->estado == 0) {
        futex_wait(&cond->estado, 0);
    }

    // Restablece el estado de la condición si no hay más hilos esperando
    if (__sync_sub_and_fetch(&cond->waiting, 1) == 0) {
        cond->estado = 0;
    }

    // Vuelve a bloquear el mutex externo
    CEmutex_lock(mutex);
}

// Señala una condición (despierta un hilo)
void CECond_signal(CECond* cond) {
    if (cond->waiting > 0) {
        cond->estado = 1;  // Señala la condición
        futex_wake(&cond->estado);  // Despierta un hilo
    }
}

// Señala una condición (despierta todos los hilos)
void CECond_broadcast(CECond* cond) {
    if (cond->waiting > 0) {
        cond->estado = 1;  // Señala la condición
        futex_wake_all(&cond->estado);  // Despierta todos los hilos
    }
}
