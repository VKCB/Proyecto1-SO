#include "FCFS.h"
#include "CEthreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Variables globales para FCFS
static Car* cola_fcfs[100]; // Cola de carros (máximo 100 carros)
static int num_fcfs = 0;    // Número de carros en la cola
static CEMutex mutex;       // Mutex para proteger las variables compartidas
static int inicializado = 0;

// Inicializa el algoritmo FCFS
void init_fcfs() {
    if (!inicializado) {
        CEmutex_init(&mutex);
        inicializado = 1;
    }
}

// Función que ejecuta cada carro (hilo)
void* funcion_carro_fcfs(void* arg) {
    Car* carro = (Car*)arg;

    printf("[FCFS] Carro %d está cruzando...\n", carro->tid);
    sleep(1); // Simula el tiempo que tarda en cruzar
    printf("[FCFS] Carro %d ha cruzado.\n", carro->tid);

    CEthread_exit();
    return NULL;
}

// Agrega un carro a la cola de FCFS
void fcfs_agregar_carro(Car* carro) {
    init_fcfs();
    CEmutex_lock(&mutex);

    if (num_fcfs >= 100) {
        printf("[FCFS] La cola está llena. No se puede agregar el carro %d.\n", carro->tid);
        CEmutex_unlock(&mutex);
        return;
    }

    cola_fcfs[num_fcfs] = carro;
    num_fcfs++;
    printf("[FCFS] Carro %d ingresado a la cola.\n", carro->tid);

    CEmutex_unlock(&mutex);

	fcfs_exec(); // Ejecutar el algoritmo FCFS
}

// Ejecuta el algoritmo FCFS
void fcfs_exec() {
    init_fcfs();
    printf("[FCFS] Iniciando ejecución del algoritmo First Come First Serve...\n");

    while (1) {
        CEmutex_lock(&mutex);

        // Verificar si hay carros en la cola
        if (num_fcfs == 0) {
            CEmutex_unlock(&mutex);
            break; // Salir del bucle si no hay más carros
        }

        // Tomar el primer carro de la cola
        Car* carro = cola_fcfs[0];

        // Desplazar la cola hacia la izquierda
        for (int i = 0; i < num_fcfs - 1; i++) {
            cola_fcfs[i] = cola_fcfs[i + 1];
        }
        num_fcfs--;

        CEmutex_unlock(&mutex);

        // Ejecutar el carro
        printf("[FCFS] Ejecutando el carro %d...\n", carro->tid);
        CEthread_create(carro, funcion_carro_fcfs, carro);
        CEthread_join(carro);
        printf("[FCFS] Carro %d ha terminado.\n", carro->tid);
    }

    printf("[FCFS] Todos los carros han sido procesados.\n");
}



void fcfs_salir(Car* carro) {
    init_fcfs();
    CEmutex_lock(&mutex);

    int encontrado = -1;

    // Buscar el carro en la cola
    for (int i = 0; i < num_fcfs; i++) {
        if (cola_fcfs[i] == carro) {
            encontrado = i;
            break;
        }
    }

    if (encontrado == -1) {
        printf("[FCFS] Carro con tid %d no encontrado en la cola.\n", carro->tid);
        CEmutex_unlock(&mutex);
        return;
    }

    printf("[FCFS] Carro con tid %d saliendo de la cola de FCFS.\n", carro->tid);

    // Eliminar el carro de la cola desplazando los elementos hacia la izquierda
    for (int i = encontrado; i < num_fcfs - 1; i++) {
        cola_fcfs[i] = cola_fcfs[i + 1];
    }

    // Reducir el número de carros en la cola
    num_fcfs--;

    printf("[FCFS] Carro con tid %d eliminado de la cola.\n", carro->tid);

    CEmutex_unlock(&mutex);
}