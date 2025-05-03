#include "CEthreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define QUANTUM_USEC 500000 // Quantum de 0.5 segundos

extern Car* cola_listos[];   // cola de hilos (carros) listos para ejecutar
extern int num_listos;       // Cantidad actual de carros en cola
extern int tiempos_restantes[]; // Tiempos de trabajo restantes por carro microsegundos

// Funcion carro ejecuta cada hilo durante su quantum, cada carro es un hilo
void* funcion_carro(void* arg) {
    Car* carro = (Car*)arg;
    int id = carro->tid;
    int trabajo = tiempos_restantes[id];

    // Ejecuta solo el quantum o lo que quede
    int trabajo_actual = (trabajo >= QUANTUM_USEC) ? QUANTUM_USEC : trabajo;

    printf("[RR] Carro %d trabajando %d us\n", id, trabajo_actual);
    usleep(trabajo_actual);

    // Actualiza el tiempo restante
    tiempos_restantes[id] -= trabajo_actual;

    if (tiempos_restantes[id] <= 0) {
        printf("[RR] Carro %d ha terminado\n", id);
    }

    CEthread_exit();
    return NULL;
}

// Round Robin 
void RR() {
    int terminado = 0;

    while (!terminado) {
        terminado = 1;

        for (int i = 0; i < num_listos; i++) {
            int id = cola_listos[i]->tid;

            if (tiempos_restantes[id] > 0) {
                terminado = 0;

                CEthread_create(cola_listos[i], funcion_carro, cola_listos[i]);
                CEthread_join(cola_listos[i]);
            }
        }
    }

    printf("[RR] Todos los carros han cruzado.\n");
}
