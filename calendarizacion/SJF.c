#include "SJF.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Variables globales para SJF
static Car* cola_listos[MAX_CARROS]; // Cola de carros listos (máximo 100 carros)
static int num_listos = 0;           // Número de carros en la cola

// Mutex para proteger las variables compartidas
static CEMutex mutex;
static int mutex_inicializado = 0;

// Inicializa el mutex
void init_mutex_sjf() {
    if (!mutex_inicializado) {
        CEmutex_init(&mutex);
        mutex_inicializado = 1;
    }
}

// Ordena una fila de carros por tiempo (SJF) - para test.c (arreglo de estructuras)
void ordenar_por_sjf(Car* fila, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (fila[j].tiempo > fila[j + 1].tiempo) {
                Car temp = fila[j];
                fila[j] = fila[j + 1];
                fila[j + 1] = temp;
            }
        }
    }
}



// Ordena la cola interna de punteros (para uso interno de SJF)
void ordenar_cola_listos() {
    for (int i = 0; i < num_listos - 1; i++) {
        for (int j = 0; j < num_listos - i - 1; j++) {
            if (cola_listos[j]->tiempo > cola_listos[j + 1]->tiempo) {
                Car* temp = cola_listos[j];
                cola_listos[j] = cola_listos[j + 1];
                cola_listos[j + 1] = temp;
            }
        }
    }
}

// Función para agregar un carro a la cola de SJF
void sjf_agregar_carro(Car* carro) {
    init_mutex_sjf();
    CEmutex_lock(&mutex);

    if (num_listos >= MAX_CARROS) {
        printf("[SJF] La cola de SJF está llena.\n");
        CEmutex_unlock(&mutex);
        return;
    }

    cola_listos[num_listos] = carro;
    num_listos++;

    printf("[SJF] Carro con tid %d agregado a la cola de SJF.\n", carro->tid);

    // Ordenar la cola interna por tiempo (SJF)
    ordenar_cola_listos();

    CEmutex_unlock(&mutex);

    sjf_exec();
}

// Función que ejecuta el algoritmo SJF
void sjf_exec() {
    init_mutex_sjf();
    printf("[SJF] Iniciando ejecución de SJF...\n");

    while (num_listos > 0) {
        CEmutex_lock(&mutex);

        if (num_listos == 0) {
            CEmutex_unlock(&mutex);
            break;
        }

        Car* carro = cola_listos[0];

        CEmutex_unlock(&mutex);

        printf("[SJF] Ejecutando el carro con tid %d durante %d us...\n", carro->tid, carro->tiempo);
        usleep(carro->tiempo);
        printf("[SJF] Carro con tid %d ha terminado.\n", carro->tid);

        CEmutex_lock(&mutex);

        for (int i = 0; i < num_listos - 1; i++) {
            cola_listos[i] = cola_listos[i + 1];
        }
        num_listos--;

        CEmutex_unlock(&mutex);
    }

    printf("[SJF] Todos los carros han sido procesados.\n");
}

// Función para manejar la salida de un carro (opcional)
void sjf_salir(Car* carro) {
    init_mutex_sjf();
    CEmutex_lock(&mutex);

    for (int i = 0; i < num_listos; i++) {
        if (cola_listos[i] == carro) {
            for (int j = i; j < num_listos - 1; j++) {
                cola_listos[j] = cola_listos[j + 1];
            }
            num_listos--;
            printf("[SJF] Carro con tid %d ha salido de la cola de SJF.\n", carro->tid);
            break;
        }
    }

    CEmutex_unlock(&mutex);
}