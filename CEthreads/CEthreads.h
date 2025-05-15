#ifndef CETHREADS_H
#define CETHREADS_H

#include <unistd.h>
#include <sys/types.h>

// Start side
typedef enum {
    LUGAR_IZQUIERDA,
    LUGAR_DERECHA,
    LUGAR_INICIO
} LugarInicio;

// Car type
typedef enum {
    TIPO_NORMAL,
    TIPO_SPORT,
    TIPO_PRIORITARIO
} TipoCarro;

// Mutex personalizado
typedef struct {
    volatile int locked;  // Estado del mutex (0 = desbloqueado, 1 = bloqueado)
} CEMutex;

// Condición personalizada
typedef struct {
    volatile int estado;  // Estado de la condición (0 = no señalada, 1 = señalada)
    volatile int waiting; // Número de hilos esperando
} CECond;

// Car structure
typedef struct {
    pid_t tid;           // Thread ID
    int activo; // 1 = activo, 0 = terminado
    int id;
    void* stack;         // Stack memory
    volatile int done;   // For futex
    LugarInicio lugar_inicio; // Side (left/right)
    TipoCarro tipo;           // Car type (normal/sport/prioritario)
    float velocidad;          // Speed
    int prioridad;            // Priority
    int tiempo;               // Time
} Car;

// Thread API
int CEthread_create(Car* thread, void *(*start_routine)(void*), void* arg);
int CEthread_join(Car* thread);
void CEthread_exit(void);

// Mutex API
int CEmutex_init(CEMutex* mutex);
int CEmutex_destroy(CEMutex* mutex);
int CEmutex_lock(CEMutex* mutex);
int CEmutex_unlock(CEMutex* mutex);

// Condición API
void CECond_init(CECond* cond);
void CECond_wait(CECond* cond, CEMutex* mutex);
void CECond_signal(CECond* cond);
void CECond_broadcast(CECond* cond);

#endif // CETHREADS_H
