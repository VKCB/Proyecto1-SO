// config.h
#ifndef CONFIG_H
#define CONFIG_H
#include "CEthreads/CEthreads.h"
#define MAX_CARROS 100

extern Car carros_izquierda[];  // arreglo, no int
extern int cantidad_izquierda;

extern Car carros_derecha[];
extern int cantidad_derecha;

extern double LONGITUD_CALLE;
extern int INTERVALO_TURNO;
extern int VALOR_W;


#endif
