#ifndef EQUIDAD_H
#define EQUIDAD_H

#include "../CEthreads/CEthreads.h"

// Inicializa el controlador de tráfico
void iniciar_control_equidad(int w);

// Hilo que alterna el paso entre lados con base en valor_W carros
void* controlador_letrero_equidad(void* arg);

// Función que debe ser llamada por cada carro antes de cruzar
void esperar_turno_equidad(Car* car);

#endif // EQUIDAD_H
