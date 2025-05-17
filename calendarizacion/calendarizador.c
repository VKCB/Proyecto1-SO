#include "calendarizador.h"
#include "c_prioridad.h"
#include "c_tiempo_real.h"
#include "RR.h"
#include "FCFS.h"
#include "SJF.h"
#include <stdio.h>

// Variable global para almacenar el algoritmo configurado
static Algoritmo algoritmo_actual = PRIORIDAD;



// Configura el algoritmo de calendarización
void configurar_algoritmo(Algoritmo algoritmo) {
    algoritmo_actual = algoritmo;
    printf("Algoritmo configurado: %d\n", algoritmo_actual);
}

// Ingresa un carro a la cola según el algoritmo configurado
void ingresar_carro(Car* carro) {
    switch (algoritmo_actual) {
        case PRIORIDAD:
            printf("Usando el algoritmo de Prioridad para ingresar el carro %d\n", carro->tid);
            prioridad_ingresar(carro);
            break;
        case TIEMPO_REAL:
            printf("Usando el algoritmo de Tiempo Real para ingresar el carro %d\n", carro->tid);
            tiempo_real_ingresar(carro, 5);  // Tiempo máximo de 5 segundos
            break;
        case RR:
            printf("Usando el algoritmo de Round Robin para ingresar el carro %d\n", carro->tid);
            rr_agregar_carro(carro); // Agregar el carro a la cola y ejecutar Round Robin
            break;

        case FCFS:
            printf("Usando el algoritmo de First Come First Serve para ingresar el carro %d\n", carro->tid);
            fcfs_agregar_carro(carro); // Agregar el carro a la cola y ejecutar FCFS
            break;
        case SJF:
            printf("Usando el algoritmo de Shortest Job First para ingresar el carro %d\n", carro->tid);
            sjf_agregar_carro(carro); // Agregar el carro a la cola y ejecutar SJF
            break;
        default:
            printf("Algoritmo desconocido.\n");
            break;
    }
}


// Marca que un carro ha salido de la carretera
void salir_carro(Car* carro) {
    switch (algoritmo_actual) {
        case PRIORIDAD:
            printf("Usando el algoritmo de Prioridad para sacar el carro %d\n", carro->tid);
            prioridad_salir(carro);
            break;
        case TIEMPO_REAL:
            printf("Usando el algoritmo de Tiempo Real para sacar el carro %d\n", carro->tid);
            tiempo_real_salir(carro);
            break;
        case RR:
            printf("Usando el algoritmo de Round Robin para sacar el carro %d\n", carro->tid);
            break;
        case FCFS:
            printf("Usando el algoritmo de First Come First Serve para sacar el carro %d\n", carro->tid);
            break;

        case SJF:
            printf("SJF no implementado aún.\n");
            break;
        default:
            printf("Algoritmo desconocido.\n");
            break;
    }
}