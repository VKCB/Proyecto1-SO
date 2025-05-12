#include "calendarizador.h"
#include <stdio.h>
#include <stdlib.h>
#include "../CEthreads/CEthreads.h"

#define MAX_COLA 100

static Car* cola_espera[MAX_COLA];
static int tamaño = 0;

static CEMutex mutex;  // Mutex
static CECond cond;    // Condición

static int inicializado = 0;
static int carro_en_carretera = 0;

// Inicializa el algoritmo de Prioridad
void inicializar_prioridad() {
    if (!inicializado) {
        CEmutex_init(&mutex);
        CECond_init(&cond);
        inicializado = 1;
    }
}

// Inserta un carro en la cola según su prioridad
void insertar_por_prioridad(Car* nuevo) {
    int i = tamaño - 1;
    while (i >= 0 && cola_espera[i]->prioridad > nuevo->prioridad) {
        cola_espera[i + 1] = cola_espera[i];
        i--;
    }
    cola_espera[i + 1] = nuevo;
    tamaño++;
}

// Devuelve el primer carro en la cola
Car* primero_prioridad() {
    return (tamaño > 0) ? cola_espera[0] : NULL;
}

// Elimina el primer carro de la cola
void eliminar_primero_prioridad() {
    for (int i = 1; i < tamaño; i++) {
        cola_espera[i - 1] = cola_espera[i];
    }
    tamaño--;
}

// Ingresa un carro al algoritmo de Prioridad
void prioridad_ingresar(Car* carro) {
    inicializar_prioridad();
    CEmutex_lock(&mutex);

    printf("Carro %d ingresado al algoritmo de Prioridad con prioridad %d\n", carro->tid, carro->prioridad);
    insertar_por_prioridad(carro);

    while (carro != cola_espera[0] || carro_en_carretera) {
        CECond_wait(&cond, &mutex);
    }

    printf("Carro %d está cruzando la carretera (Prioridad)\n", carro->tid);
    carro_en_carretera = 1;

    CEmutex_unlock(&mutex);
}

// Marca que un carro ha salido de la carretera
void prioridad_salir(Car* carro) {
    CEmutex_lock(&mutex);

    printf("Carro %d ha salido del algoritmo de Prioridad\n", carro->tid);
    carro_en_carretera = 0;
    CECond_broadcast(&cond);

    CEmutex_unlock(&mutex);
}
