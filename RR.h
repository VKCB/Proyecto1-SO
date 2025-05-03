#ifndef RR_H
#define RR_H

#include "CEthreads.h"

// Quantum fijo en microsegundos
#define QUANTUM_USEC 500000

// Arreglo de punteros a hilos listos (carros)
extern Car* cola_listos[];

// Cantidad de hilos en la cola de listos
extern int num_listos;

// Tiempos restantes por carro en microsegundos
extern int tiempos_restantes[];

// Funcion que ejecuta cada hilo
void* funcion_carro(void* arg);

// Ejecicion de Round Robin 
void RR(void);

#endif // RR_H
