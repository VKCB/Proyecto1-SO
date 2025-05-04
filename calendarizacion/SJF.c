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

// Función para agregar un carro a la cola de SJF
void sjf_agregar_carro(Car* carro) {
    init_mutex_sjf();
    CEmutex_lock(&mutex); // Bloquear el acceso a las variables compartidas

    if (num_listos >= MAX_CARROS) {
        printf("[SJF] La cola de SJF está llena.\n");
        CEmutex_unlock(&mutex);
        return;
    }

    cola_listos[num_listos] = carro;
    num_listos++;

    printf("[SJF] Carro con tid %d agregado a la cola de SJF.\n", carro->tid);

    // Ordenar la cola por tiempo (SJF)
    for (int i = 0; i < num_listos - 1; i++) {
        for (int j = 0; j < num_listos - i - 1; j++) {
            if (cola_listos[j]->tiempo > cola_listos[j + 1]->tiempo) {
                Car* temp = cola_listos[j];
                cola_listos[j] = cola_listos[j + 1];
                cola_listos[j + 1] = temp;
            }
        }
    }

    CEmutex_unlock(&mutex); // Desbloquear después de actualizar

    sjf_exec(); // Ejecutar el algoritmo SJF
    
}

// Función que ejecuta el algoritmo SJF
void sjf_exec() {
    init_mutex_sjf();
    printf("[SJF] Iniciando ejecución de SJF...\n");

    while (num_listos > 0) {
        CEmutex_lock(&mutex); // Bloquear para acceder a la cola

        // Verificar si hay carros en la cola
        if (num_listos == 0) {
            CEmutex_unlock(&mutex);
            break; // Salir del bucle si no hay más carros
        }

        // Tomar el primer carro de la cola
        Car* carro = cola_listos[0];

        // Desbloquear antes de ejecutar
        CEmutex_unlock(&mutex);

        // Ejecutar el carro
        printf("[SJF] Ejecutando el carro con tid %d durante %d us...\n", carro->tid, carro->tiempo);
        usleep(carro->tiempo); // Simula el tiempo que tarda en cruzar
        printf("[SJF] Carro con tid %d ha terminado.\n", carro->tid);

        CEmutex_lock(&mutex); // Volver a bloquear para actualizar la cola

        // Eliminar el carro de la cola
        for (int i = 0; i < num_listos - 1; i++) {
            cola_listos[i] = cola_listos[i + 1];
        }
        num_listos--;

        CEmutex_unlock(&mutex); // Desbloquear después de actualizar
    }

    printf("[SJF] Todos los carros han sido procesados.\n");
}

// Función para manejar la salida de un carro (opcional)
void sjf_salir(Car* carro) {
    init_mutex_sjf();
    CEmutex_lock(&mutex);

    // Buscar el carro en la cola y eliminarlo
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