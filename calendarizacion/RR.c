#include "RR.h"
#include "CEthreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define QUANTUM_USEC 500000 // Quantum de 0.5 segundos

// Variables globales para Round Robin
static Car* cola_listos[100]; // Cola de carros listos (máximo 100 carros)
static int num_listos = 0;    // Número de carros en la cola
static int tiempos_restantes[100]; // Tiempos restantes para cada carro (en microsegundos)

// Mutex para proteger las variables compartidas
static CEMutex mutex;
static int mutex_inicializado = 0;

// Inicializa el mutex si no está inicializado
void inicializar_mutex() {
    if (!mutex_inicializado) {
        CEmutex_init(&mutex);
        mutex_inicializado = 1;
    }
}

// Función que ejecuta cada carro durante su quantum
void* funcion_carro(void* arg) {
    Car* carro = (Car*)arg;
    int id = carro->tid;

    CEmutex_lock(&mutex); // Bloquear el acceso a las variables compartidas
    int trabajo = tiempos_restantes[id];
    int trabajo_actual = (trabajo >= QUANTUM_USEC) ? QUANTUM_USEC : trabajo;
    CEmutex_unlock(&mutex); // Desbloquear después de leer

    printf("[RR] Carro %d trabajando %d us\n", id, trabajo_actual);
    usleep(trabajo_actual);

    CEmutex_lock(&mutex); // Bloquear para actualizar
    tiempos_restantes[id] -= trabajo_actual;

    if (tiempos_restantes[id] <= 0) {
        printf("[RR] Carro %d ha terminado\n", id);
    }
    CEmutex_unlock(&mutex); // Desbloquear después de actualizar

    CEthread_exit();
    return NULL;
}

// Agrega un carro a la cola de Round Robin
void agregar_carro(Car* carro) {
    inicializar_mutex();
    CEmutex_lock(&mutex); // Bloquear el acceso a las variables compartidas

    if (num_listos >= 100) {
        printf("[RR] La cola de Round Robin está llena.\n");
        CEmutex_unlock(&mutex);
        return;
    }

    cola_listos[num_listos] = carro;
    tiempos_restantes[num_listos] = 1500000; // Ejemplo: tiempo inicial de 1.5 segundos
    carro->tid = num_listos; // Asignar un ID basado en la posición en la cola
    num_listos++;

    printf("[RR] Carro %d ingresado a la cola de Round Robin.\n", carro->tid);

    CEmutex_unlock(&mutex); // Desbloquear después de actualizar
}

// Ejecuta el algoritmo de Round Robin
void rr(Car* carro) {
    inicializar_mutex();
    printf("[RR] Iniciando ejecución de Round Robin...\n");

    while (num_listos > 0) {
        CEmutex_lock(&mutex); // Bloquear para acceder a la cola

        // Tomar el primer carro de la cola
        Car* carro = cola_listos[0];
        int id = carro->tid;

        // Desbloquear antes de ejecutar el quantum
        CEmutex_unlock(&mutex);

        // Ejecutar el carro durante su quantum
        CEthread_create(carro, funcion_carro, carro);
        CEthread_join(carro);

        CEmutex_lock(&mutex); // Volver a bloquear para actualizar la cola

        // Verificar si el carro ha terminado
        if (tiempos_restantes[id] <= 0) {
            printf("[RR] Carro %d ha terminado y será eliminado de la cola.\n", id);
            salir(carro); // Eliminar el carro de la cola
        } else {
            // Mover el carro al final de la cola usando agregar_carro
            printf("[RR] Carro %d no terminó, moviéndolo al final de la cola.\n", id);

            // Eliminar el carro de la cola actual
            for (int i = 0; i < num_listos - 1; i++) {
                cola_listos[i] = cola_listos[i + 1];
                tiempos_restantes[i] = tiempos_restantes[i + 1];
                cola_listos[i]->tid = i; // Actualizar el ID del carro
            }
            num_listos--; // Reducir el número de carros en la cola

            // Añadir el carro al final de la cola
            carro->tid = num_listos; // Actualizar el ID del carro
            agregar_carro(carro);
        }

        CEmutex_unlock(&mutex); // Desbloquear después de actualizar la cola
    }

    printf("[RR] Todos los carros han terminado.\n");
}

// Maneja la salida de un carro
void salir(Car* carro) {
    inicializar_mutex();
    CEmutex_lock(&mutex); // Bloquear el acceso a las variables compartidas

    int id = carro->tid;

    // Verificar si el carro está en la cola
    if (id < 0 || id >= num_listos || cola_listos[id] != carro) {
        printf("[RR] Carro %d no encontrado en la cola.\n", id);
        CEmutex_unlock(&mutex);
        return;
    }

    printf("[RR] Carro %d saliendo de la cola de Round Robin.\n", id);

    // Eliminar el carro de la cola
    for (int i = id; i < num_listos - 1; i++) {
        cola_listos[i] = cola_listos[i + 1];
        tiempos_restantes[i] = tiempos_restantes[i + 1];
        cola_listos[i]->tid = i; // Actualizar el ID del carro
    }

    // Reducir el número de carros en la cola
    num_listos--;

    // Limpiar la última posición
    cola_listos[num_listos] = NULL;
    tiempos_restantes[num_listos] = 0;

    CEmutex_unlock(&mutex); // Desbloquear después de actualizar

    printf("[RR] Carro %d eliminado de la cola.\n", id);
}
