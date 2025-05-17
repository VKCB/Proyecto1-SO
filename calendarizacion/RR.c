#include "RR.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../control_flujo/Equidad.h"

// Variables globales
static Car* cola_listos[100];
static int num_listos = 0;
static int tiempos_restantes[100]; // microsegundos

static CEMutex mutex;
static int mutex_inicializado = 0;

static int quantum_usec = 1000000; // Por defecto: 10 segundos por quantum

void init_mutex() {
    if (!mutex_inicializado) {
        CEmutex_init(&mutex);
        mutex_inicializado = 1;
    }
}

void* funcion_carro_rr(void* arg) {
    Car* carro = (Car*)arg;

    while (1) {
        CEmutex_lock(&carro->mutex);
        // Si el carro ya está marcado como terminado, salir
        if (carro->terminado) {
            CEmutex_unlock(&carro->mutex);
            printf("[RR] Carro con tid %d ya ha terminado, saliendo.\n", carro->tid);
            break;
        }

        while (!carro->listo) {
            CECond_wait(&carro->cond, &carro->mutex);
        }
        
        // Obtener tiempo a ejecutar (manteniendo el mutex bloqueado)
        int ejecutar = quantum_usec;
        if (carro->tiempo_restante < quantum_usec)
            ejecutar = carro->tiempo_restante;
            
        carro->listo = 0;
        CEmutex_unlock(&carro->mutex);

        // Llamar a la función de control de flujo solo si el carro no ha terminado
        CEmutex_lock(&carro->mutex);
        if (!carro->terminado) {
            CEmutex_unlock(&carro->mutex);
            
            esperar_turno_equidad(carro); // Llamar a la función de control de flujo con Equidad
            //extern void esperar_turno_FIFO(Car*); // Declara la función externa
            //esperar_turno_FIFO(carro);  // Usa FIFO para controlar el turno
            
            printf("[RR] Carro con tid %d cruzando %d us...\n", carro->tid, ejecutar);
            usleep(ejecutar);
        } else {
            CEmutex_unlock(&carro->mutex);
        }
        
        // Bloquear nuevamente para actualizar tiempo restante
        CEmutex_lock(&carro->mutex);
        carro->tiempo_restante -= ejecutar;
        int terminado = (carro->tiempo_restante <= 0);
        if (terminado) {
            carro->terminado = 1;  // MARCAR EXPLÍCITAMENTE COMO TERMINADO
        }
        CEmutex_unlock(&carro->mutex);

        if (terminado) {
            printf("[RR] Carro con tid %d ha terminado.\n", carro->tid);
            break;
        }
    }

    CEthread_exit();
    return NULL;
}

void rr_agregar_carro(Car* carro) {
    init_mutex();
    CEmutex_lock(&mutex);

    if (num_listos >= 100) {
        printf("[RR] Cola llena.\n");
        CEmutex_unlock(&mutex);
        return;
    }

    carro->tiempo_restante = 1500000; // 1.5 segundos
    carro->listo = 0;
    carro->terminado = 0;  // Inicializar como no terminado
    CEmutex_init(&carro->mutex);
    CECond_init(&carro->cond);
    carro->tid = 100000 + num_listos;

    cola_listos[num_listos] = carro;
    tiempos_restantes[num_listos] = carro->tiempo_restante;

    CEthread_create(carro, funcion_carro_rr, carro);
    printf("[RR] Carro %d ingresado a la cola de Round Robin.\n", carro->tid);
    num_listos++;

    CEmutex_unlock(&mutex);
}

void rr_exec() {
    init_mutex();
    printf("[RR] Iniciando ejecución de Round Robin con quantum %d us...\n", quantum_usec);

    while (num_listos > 0) {
        Car* carro = cola_listos[0];
        
        // Verificar primero si el carro ya está marcado como termiestá marcado comodo
        CEmutex_lock(&carro->mutex);
        int esta_terminado = carro->terminado;
        CEmutex_unlock(&carro->mutex);

        if (esta_terminado) {
            // El carro ya terminó, eliminarlo de la cola
            printf("[RR] Eliminando carro %d de la cola (ya terminado).\n", carro->tid);
            CEmutex_lock(&mutex);
            for (int i = 0; i < num_listos - 1; i++) {
                cola_listos[i] = cola_listos[i + 1];
            }
            num_listos--;
            CEmutex_unlock(&mutex);
            continue;  // Ir al siguiente carro
        }

        // Activar el carro
        CEmutex_lock(&carro->mutex);
        carro->listo = 1;
        CECond_signal(&carro->cond);
        CEmutex_unlock(&carro->mutex);

        // Esperar un poco más que el quantum para dar tiempo al carro
        usleep(quantum_usec + 10000); // 10ms extra para seguridad
        
        // Verificar de nuevo el estado del carro
        CEmutex_lock(&carro->mutex);
        esta_terminado = carro->terminado;
        int tiempo_restante = carro->tiempo_restante;
        CEmutex_unlock(&carro->mutex);
        
        // Mostrar tiempo restante para depuración
        //printf("/[RR] Carro %d tiene %d us restantes. Terminado: %s\n", 
        //       carro->tid, tiempo_restante, esta_terminado ? "Sí" : "No");

        if (esta_terminado || tiempo_restante <= 0) {
            // Eliminar el carro de la cola
            CEmutex_lock(&mutex);
            printf("[RR] Eliminando carro %d de la cola.\n", carro->tid);
            for (int i = 0; i < num_listos - 1; i++) {
                cola_listos[i] = cola_listos[i + 1];
            }
            num_listos--;
            CEmutex_unlock(&mutex);
        } else {
            // Reubicar el carro al final
            CEmutex_lock(&mutex);
            Car* temp = cola_listos[0];
            for (int i = 0; i < num_listos - 1; i++) {
                cola_listos[i] = cola_listos[i + 1];
            }
            cola_listos[num_listos - 1] = temp;
            CEmutex_unlock(&mutex);
        }
    }

    printf("[RR] Todos los carros han terminado.\n");
}

void procesar_rr(Car* fila, int count, int quantum) {
    quantum_usec = quantum;
    for (int i = 0; i < count; i++) {
        rr_agregar_carro(&fila[i]);
    }
    rr_exec();
}
