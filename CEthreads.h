#ifndef CETHREADS_H
#define CETHREADS_H

#include <unistd.h>
#include <sys/types.h>

// Start side
typedef enum {
    LUGAR_IZQUIERDA,
    LUGAR_DERECHA
} LugarInicio;

// Car type
typedef enum {
    TIPO_NORMAL,
    TIPO_SPORT,
    TIPO_PRIORITARIO
} TipoCarro;

typedef struct {
    volatile int locked;
} CEMutex;


typedef struct {
    pid_t tid;           // Thread ID
    void* stack;         // Stack memory
    volatile int done;   // For futex
    LugarInicio lugar_inicio; // Side (left/right)
    TipoCarro tipo;           // Car type (normal/sport/prioritario)
    float velocidad;          // Speed
} Car;

// Thread API
int CEthread_create(Car* thread, void *(*start_routine)(void*), void* arg);
int CEthread_join(Car* thread);
void CEthread_exit(void);
int CEmutex_init(CEMutex* mutex);
int CEmutex_destroy(CEMutex* mutex);
int CEmutex_lock(CEMutex* mutex);
int CEmutex_unlock(CEMutex* mutex);

#endif // CETHREADS_H
