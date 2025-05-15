#include "c_prioridad.h"
#include <stdio.h>
#include <stdlib.h>
#include "../CEthreads/CEthreads.h"

#define MAX_COLA 100

static Car* cola_espera[MAX_COLA];
static int tamaño = 0;

static CEMutex mutex;
static CECond cond;

static int inicializado = 0;
static int carro_en_carretera = 0;

void inicializar_prioridad() {
    if (!inicializado) {
        CEmutex_init(&mutex);
        CECond_init(&cond);
        inicializado = 1;
    }
}

void insertar_por_prioridad(Car* nuevo) {
    int i = tamaño - 1;
    while (i >= 0 && cola_espera[i]->prioridad > nuevo->prioridad) {
        cola_espera[i + 1] = cola_espera[i];
        i--;
    }
    cola_espera[i + 1] = nuevo;
    tamaño++;
}

Car* primero_prioridad() {
    return (tamaño > 0) ? cola_espera[0] : NULL;
}

void eliminar_primero_prioridad() {
    for (int i = 1; i < tamaño; i++) {
        cola_espera[i - 1] = cola_espera[i];
    }
    tamaño--;
}

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

void prioridad_salir(Car* carro) {
    CEmutex_lock(&mutex);

    printf("Carro %d ha salido del algoritmo de Prioridad\n", carro->tid);
    carro_en_carretera = 0;
    CECond_broadcast(&cond);

    CEmutex_unlock(&mutex);
}

// Ordena una fila de carros por prioridad (mayor prioridad primero)
void ordenar_por_prioridad(Car* fila, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (fila[j].prioridad < fila[j + 1].prioridad) {
                Car temp = fila[j];
                fila[j] = fila[j + 1];
                fila[j + 1] = temp;
            }
        }
    }
}
