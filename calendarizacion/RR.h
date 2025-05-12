#ifndef RR_H
#define RR_H

#include "../biblioteca/CEthreads.h"

// Quantum fijo en microsegundos
#define QUANTUM_USEC 500000
#define MAX_CARROS 100

//-----FUnciones de RR.c-----//
// Funcion que ejecuta cada hilo
void* funcion_carro_rr(void* arg);

void rr_agregar_carro(Car* carro);
void rr_exec();
void rr_salir(Car* carro);

#endif // RR_H
