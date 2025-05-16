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

static int quantum_usec = 500000; // Por defecto: 0.5 segundos por quantum

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
        while (!carro->listo) {
            CECond_wait(&carro->cond, &carro->mutex);
        }
        carro->listo = 0;
        CEmutex_unlock(&carro->mutex);

        int ejecutar = quantum_usec;
        if (carro->tiempo_restante < quantum_usec)
            ejecutar = carro->tiempo_restante;

        esperar_turno_equidad(carro);

        printf("[RR] Carro con tid %d cruzando %d us...\n", carro->tid, ejecutar);
        usleep(ejecutar);
        carro->tiempo_restante -= ejecutar;

        if (carro->tiempo_restante <= 0) {
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
    CEmutex_init(&carro->mutex);
    CECond_init(&carro->cond);
    carro->tid = num_listos;

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

        CEmutex_lock(&carro->mutex);
        carro->listo = 1;
        CECond_signal(&carro->cond);
        CEmutex_unlock(&carro->mutex);

        usleep(quantum_usec + 1000); // tiempo para que el carro cruce (1ms extra)

        if (carro->tiempo_restante > 0) {
            // Reubicar el carro al final
            for (int i = 0; i < num_listos - 1; i++) {
                cola_listos[i] = cola_listos[i + 1];
            }
            cola_listos[num_listos - 1] = carro;
        } else {
            // Eliminar el carro de la cola
            for (int i = 0; i < num_listos - 1; i++) {
                cola_listos[i] = cola_listos[i + 1];
            }
            num_listos--;
        }
    }

    printf("[RR] Todos los carros han terminado.\n");
}

void procesar_rr(Car* fila, int count, int quantum) {
    quantum_usec = quantum; // <-- Aquí aplicamos el quantum recibido
    for (int i = 0; i < count; i++) {
        rr_agregar_carro(&fila[i]);
    }
    rr_exec();
}
