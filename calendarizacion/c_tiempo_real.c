#include "calendarizador.h"
#include "CEthreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_COLA 100

static Car* cola_espera[MAX_COLA];
static int tamaño = 0;

static CEMutex mutex;  // Mutex
static CECond cond;    // Condición

static int inicializado = 0;
static int carro_en_carretera = 0;

// Estructura para medir el tiempo
typedef struct {
    Car* carro;
    time_t tiempo_inicio;
} TiempoRealCarro;

static TiempoRealCarro tiempos[MAX_COLA];

// Inicializa el algoritmo de Tiempo Real
void inicializar_tiempo_real() {
    if (!inicializado) {
        CEmutex_init(&mutex);
        CECond_init(&cond);
        inicializado = 1;
    }
}

// Inserta un carro en la cola
void insertar_tiempo_real(Car* nuevo) {
    cola_espera[tamaño] = nuevo;
    tiempos[tamaño].carro = nuevo;
    tiempos[tamaño].tiempo_inicio = time(NULL);  // Registra el tiempo de entrada
    tamaño++;
}

// Ingresa un carro al algoritmo de Tiempo Real
void tiempo_real_ingresar(Car* carro, int tiempo_maximo) {
    inicializar_tiempo_real();
    CEmutex_lock(&mutex);

    printf("Carro %d ingresado al algoritmo de Tiempo Real con tiempo máximo %d\n", carro->tid, tiempo_maximo);
    insertar_tiempo_real(carro);

    while (carro != cola_espera[0] || carro_en_carretera) {
        time_t tiempo_actual = time(NULL);
        for (int i = 0; i < tamaño; i++) {
            if (difftime(tiempo_actual, tiempos[i].tiempo_inicio) > tiempo_maximo) {
                Car* carro_prioritario = cola_espera[i];
                for (int j = i; j > 0; j--) {
                    cola_espera[j] = cola_espera[j - 1];
                    tiempos[j] = tiempos[j - 1];
                }
                cola_espera[0] = carro_prioritario;
                tiempos[0].carro = carro_prioritario;
                tiempos[0].tiempo_inicio = tiempo_actual;
                break;
            }
        }

        CECond_wait(&cond, &mutex);
    }

    printf("Carro %d está cruzando la carretera (Tiempo Real)\n", carro->tid);
    carro_en_carretera = 1;

    CEmutex_unlock(&mutex);
}

// Marca que un carro ha salido de la carretera
void tiempo_real_salir(Car* carro) {
    CEmutex_lock(&mutex);

    printf("Carro %d ha salido del algoritmo de Tiempo Real\n", carro->tid);
    carro_en_carretera = 0;
    CECond_broadcast(&cond);

    CEmutex_unlock(&mutex);
}