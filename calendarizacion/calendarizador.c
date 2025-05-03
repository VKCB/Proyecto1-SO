#include "calendarizador.h"
#include "c_prioridad.h"
#include "CEthreads.h"
#include "c_tiempo_real.h"
#include "RR.h"
#include "FCFS.h"
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
            printf("Usando el algoritmo de Round Robin para ingresar el carro %d\n", carro->id);
            rr_agregar_carro(carro); // Agregar el carro a la cola de Round Robin
            break;

        case FCFS:
            printf("Usando el algoritmo de First Come First Serve para ingresar el carro %d\n", carro->id);
            fcfs_agregar_carro(carro); // Agregar el carro a la cola de FCFS
            break;
        case SJF:
            printf("SJF no implementado aún.\n");
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
            printf("Usando el algoritmo de Round Robin para sacar el carro %d\n", carro->id);
            rr_salir(carro); // Llama a la funcion de salida de RR
            break;
        case FCFS:
            printf("Usando el algoritmo de First Come First Serve para sacar el carro %d\n", carro->id);
            fcfs_salir(carro); // Llamar a la función salir FCFS
            break;

        case SJF:
            printf("SJF no implementado aún.\n");
            break;
        default:
            printf("Algoritmo desconocido.\n");
            break;
    }
}