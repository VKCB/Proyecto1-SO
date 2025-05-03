#ifndef RR_H
#define RR_H

#include "CEthreads.h"

// Quantum fijo en microsegundos
#define QUANTUM_USEC 500000
#define MAX_CARROS 100
// Arreglo de punteros a hilos listos (carros)
extern Car* cola_listos[MAX_CARROS];

// Cantidad de hilos en la cola de listos
extern int num_listos;

// Tiempos restantes por carro en microsegundos
extern int tiempos_restantes[MAX_CARROS]; //se utiliza para rastrear cuánto tiempo le queda a cada carro para completar su tarea en el algoritmo de Round Robin

//-----FUnciones de RR.c-----//
// Funcion que ejecuta cada hilo
void* funcion_carro_rr(void* arg);

void rr_agregar_carro(Car* carro);
void rr_exec();
void rr_salir(Car* carro);

#endif // RR_H
