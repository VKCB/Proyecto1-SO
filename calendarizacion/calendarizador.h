#ifndef CALENDARIZADOR_H
#define CALENDARIZADOR_H

#include "CEthreads.h"

typedef struct {
    int id;
    int prioridad;
    int tiempo;
} Carro;

typedef enum {
    PRIORIDAD,
    TIEMPO_REAL,
    RR,
    SJF,
    FCFS
} Algoritmo;

void configurar_algoritmo(Algoritmo algoritmo);

// Funciones para el algoritmo de Prioridad
void prioridad_ingresar(Car* carro);
void prioridad_salir(Car* carro);

// Funciones para el algoritmo de Tiempo Real
void tiempo_real_ingresar(Car* carro, int tiempo_maximo);
void tiempo_real_salir(Car* carro);

// Funciones generales
void ingresar_carro(Car* carro);
void salir_carro(Car* carro);

#endif // CALENDARIZADOR_H