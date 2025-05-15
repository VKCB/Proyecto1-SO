#ifndef RR_H
#define RR_H

#include "../CEthreads/CEthreads.h"

// Quantum fijo en microsegundos
#define QUANTUM_USEC 500000
#define MAX_CARROS 100

//-----Funciones de RR.c-----//
void* funcion_carro_rr(void* arg);
void rr_agregar_carro(Car* carro);
void rr_exec();
void rr_salir(Car* carro);

// NUEVA: Ordena una fila de carros para RR (no hace nada, pero mantiene la interfaz)
void ordenar_por_rr(Car* fila, int count);

// calendarizacion/RR.h
void procesar_rr(Car* fila, int* count, int quantum);

#endif // RR_H
