#ifndef FCFS_H
#define FCFS_H

#include "../CEthreads/CEthreads.h"

// Funciones públicas para FCFS
void init_fcfs();
void fcfs_agregar_carro(Car* carro);
void fcfs_exec();
void fcfs_salir(Car* carro);

// NUEVA: Ordena una fila de carros por orden de llegada (no cambia nada, pero mantiene la interfaz)
void ordenar_por_fcfs(Car* fila, int count);

#endif // FCFS_H