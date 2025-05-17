#include "calendarizador.h"
#include "c_tiempo_real.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../CEthreads/CEthreads.h"

#define MAX_COLA 100

static Car* cola_espera[MAX_COLA];
static int tamaño = 0;

static CEMutex mutex;
static CECond cond;

static int inicializado = 0;
static int carro_en_carretera = 0;

// Inicializa el algoritmo de Tiempo Real
void inicializar_tiempo_real() {
    if (!inicializado) {
        CEmutex_init(&mutex);
        CECond_init(&cond);
        inicializado = 1;
    }
}

void insertar_tiempo_real(Car* nuevo) {
    cola_espera[tamaño++] = nuevo;
}

Car* primero_tiempo_real() {
    return (tamaño > 0) ? cola_espera[0] : NULL;
}

void eliminar_primero_tiempo_real() {
    for (int i = 1; i < tamaño; i++) {
        cola_espera[i - 1] = cola_espera[i];
    }
    tamaño--;
}

void tiempo_real_ingresar(Car* carro, int tiempo_maximo) {
    inicializar_tiempo_real();
    CEmutex_lock(&mutex);

    printf("Carro %d ingresado al algoritmo de Tiempo Real\n", carro->tid);
    insertar_tiempo_real(carro);

    while (carro != cola_espera[0] || carro_en_carretera) {
        CECond_wait(&cond, &mutex);
    }

    printf("Carro %d está cruzando la carretera (Tiempo Real)\n", carro->tid);
    carro_en_carretera = 1;

    CEmutex_unlock(&mutex);
}

void tiempo_real_salir(Car* carro) {
    CEmutex_lock(&mutex);

    printf("Carro %d ha salido del algoritmo de Tiempo Real\n", carro->tid);
    carro_en_carretera = 0;
    CECond_broadcast(&cond);

    CEmutex_unlock(&mutex);
}

// Ordena una fila de carros por tiempo (menor tiempo primero)
void ordenar_por_tiempo_real(Car* fila, int count) {
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