#include "calendarizador.h"
#include "c_prioridad.h"
#include "CEthreads.h"
#include "c_tiempo_real.h"
#include <stdio.h>

// Variable global para almacenar el algoritmo configurado
static Algoritmo algoritmo_actual = PRIORIDAD;

// Configura el algoritmo de calendarización
void configurar_algoritmo(Algoritmo algoritmo) {
    algoritmo_actual = algoritmo;
    printf("Algoritmo configurado: %d\n", algoritmo_actual);
}

// Ingresa un carro a la cola según el algoritmo configurado
void ingresar_carro(Carro* carro) {
    switch (algoritmo_actual) {
        case PRIORIDAD:
            printf("Usando el algoritmo de Prioridad para ingresar el carro %d\n", carro->id);
            prioridad_ingresar(carro);
            break;
        case TIEMPO_REAL:
            printf("Usando el algoritmo de Tiempo Real para ingresar el carro %d\n", carro->id);
            tiempo_real_ingresar(carro, 5);  // Tiempo máximo de 5 segundos
            break;
        case RR:
            printf("Round Robin no implementado aún.\n");
            break;
        case SJF:
            printf("SJF no implementado aún.\n");
            break;
        case FCFS:
            printf("FCFS no implementado aún.\n");
            break;
        default:
            printf("Algoritmo desconocido.\n");
            break;
    }
}

// Marca que un carro ha salido de la carretera
void salir_carro(Carro* carro) {
    switch (algoritmo_actual) {
        case PRIORIDAD:
            printf("Usando el algoritmo de Prioridad para sacar el carro %d\n", carro->id);
            prioridad_salir(carro);
            break;
        case TIEMPO_REAL:
            printf("Usando el algoritmo de Tiempo Real para sacar el carro %d\n", carro->id);
            tiempo_real_salir(carro);
            break;
        case RR:
        case SJF:
        case FCFS:
            printf("Salir no implementado para este algoritmo.\n");
            break;
        default:
            printf("Algoritmo desconocido.\n");
            break;
    }
}