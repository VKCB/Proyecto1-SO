// Letrero.h

#ifndef FIFO_H
#define FIFO_H

#include "../CEthreads/CEthreads.h"

// Inicializa mutexes y condiciones
void iniciar_control_FIFO();

// Hilo que alterna el lado cada INTERVALO_TURNO segundos
void* controlador_letrero_FIFO(void* arg);

// Lógica de espera por turno del lado correcto
void esperar_turno_FIFO(Car* car);

#endif // LETRERO_H
