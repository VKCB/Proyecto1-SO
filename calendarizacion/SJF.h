#ifndef SJF_H
#define SJF_H

#include "../biblioteca/CEthreads.h"

#define MAX_CARROS 100

// Declaraciones de funciones
void sjf_agregar_carro(Car* carro);
void sjf_exec();
void sjf_salir(Car* carro);

#endif // SJF_H