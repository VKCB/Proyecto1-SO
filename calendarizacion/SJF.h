#ifndef SJF_H
#define SJF_H

#include "../CEthreads/CEthreads.h"

#define MAX_CARROS 100

// Declaraciones de funciones
void sjf_agregar_carro(Car* carro);
void sjf_exec();
void sjf_salir(Car* carro);

// Ordena una fila de carros por tiempo (SJF)
void ordenar_por_sjf(Car* fila, int count);

#endif // SJF_H