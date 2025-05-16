#include "RR.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Variables globales para Round Robin
static Car* cola_listos[100]; // Cola de carros listos (máximo 100 carros)
static int num_listos = 0;    // Número de carros en la cola
static int tiempos_restantes[100]; // Tiempos restantes para cada carro (en microsegundos)

// Mutex para proteger las variables compartidas
static CEMutex mutex;
static int mutex_inicializado = 0;

// Inicializa el mutex 
void init_mutex() {
    if (!mutex_inicializado) {
        CEmutex_init(&mutex);
        mutex_inicializado = 1;
    }
}

// Función que ejecuta cada carro durante su quantum
void* funcion_carro_rr(void* arg) {
    Car* carro = (Car*)arg;

    printf("[RR] Carro con tid %d cruzando...\n", carro->tid);
    usleep(QUANTUM_USEC); // Simula el tiempo que tarda en cruzar
    printf("[RR] Carro con tid %d ha cruzado.\n", carro->tid);

    CEthread_exit();
    return NULL;
}

// Agrega un carro a la cola de Round Robin
void rr_agregar_carro(Car* carro) {
    init_mutex();
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

    rr_exec(); // Ejecutar el algoritmo de Round Robin
}

// Ejecuta el algoritmo de Round Robin
void rr_exec() {
    init_mutex();
    printf("[RR] Iniciando ejecución de Round Robin...\n");

    while (num_listos > 0) {
        CEmutex_lock(&mutex); // Bloquear para acceder a la cola

        // Verificar si hay carros en la cola
        if (num_listos == 0) {
            CEmutex_unlock(&mutex);
            break; // Salir del bucle si no hay más carros
        }

        // Tomar el primer carro de la cola
        Car* carro = cola_listos[0];
        int id = carro->tid;

        // Desbloquear antes de ejecutar el quantum
        CEmutex_unlock(&mutex);

        // Ejecutar el carro durante su quantum
        printf("[RR] Ejecutando el carro con tid %d durante %d us...\n", id, QUANTUM_USEC);
        CEthread_create(carro, funcion_carro_rr, carro);
        CEthread_join(carro);

        CEmutex_lock(&mutex); // Volver a bloquear para actualizar la cola

        

        // Reducir el tiempo restante del carro
        tiempos_restantes[0] -= QUANTUM_USEC;

        if (tiempos_restantes[0] > 0) {
            // Si el carro no ha terminado, moverlo al final de la cola
            printf("[RR] Carro con tid %d no terminó, moviéndolo al final de la cola.\n", id);
            Car* temp_carro = cola_listos[0];
            int temp_tiempo = tiempos_restantes[0];

            // Mover todos los elementos hacia adelante
            for (int i = 0; i < num_listos - 1; i++) {
                cola_listos[i] = cola_listos[i + 1];
                tiempos_restantes[i] = tiempos_restantes[i + 1];
            }

            // Colocar el carro al final de la cola
            cola_listos[num_listos - 1] = temp_carro;
            tiempos_restantes[num_listos - 1] = temp_tiempo;
        } else {
            // Si el carro ha terminado, eliminarlo de la cola
            printf("[RR] Carro con tid %d ha terminado.\n", id);
            for (int i = 0; i < num_listos - 1; i++) {
                cola_listos[i] = cola_listos[i + 1];
                tiempos_restantes[i] = tiempos_restantes[i + 1];
            }
            num_listos--; // Reducir el número de carros en la cola
        }

        CEmutex_unlock(&mutex);
    }
    printf("[RR] Todos los carros han terminado.\n");
}

// Maneja la salida de un carro
void rr_salir(Car* carro) {
    init_mutex();
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

// Ordena una fila de carros para RR (no hace nada, pero mantiene la interfaz uniforme)
void ordenar_por_rr(Car* fila, int count) {
    // RR no requiere ordenamiento, pero la función existe para mantener la interfaz uniforme
}

void procesar_rr(Car* fila, int* count, int quantum) {
    for (int i = 0; i < *count; i++) {
        rr_agregar_carro(&fila[i]); // Agregar cada carro a la cola de RR
    }
    rr_exec(); // Ejecutar el algoritmo de Round Robin
    *count = 0; // Después de procesar, la fila queda vacía
}
