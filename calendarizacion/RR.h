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

//-----FUnciones de RR.c-----//
// Funcion que ejecuta cada hilo
void* funcion_carro(void* arg);

void agregar_carro(Car* carro);
void rr();
void salir(Car* carro);

#endif // RR_H
