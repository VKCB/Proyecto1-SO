#ifndef FCFS_H
#define FCFS_H

#include "CEthreads.h"

// Funciones públicas para FCFS
void init_fcfs();
void fcfs_agregar_carro(Car* carro);
void fcfs_exec();
void fcfs_salir(Car* carro);

#endif // FCFS_H