// Letrero.h

#ifndef LETRERO_H
#define LETRERO_H

#include "../CEthreads/CEthreads.h"

// Inicializa mutexes y condiciones
void iniciar_control();

// Hilo que alterna el lado cada INTERVALO_TURNO segundos
void* controlador_letrero(void* arg);

// Lógica de espera por turno del lado correcto
void esperar_turno(Car* car);

#endif // LETRERO_H
