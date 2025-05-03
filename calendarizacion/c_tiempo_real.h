#ifndef C_TIEMPO_REAL_H
#define C_TIEMPO_REAL_H

#include "calendarizador.h"


// Inicializa el algoritmo de Tiempo Real
void inicializar_tiempo_real();

// Inserta un carro en la cola de Tiempo Real
void insertar_tiempo_real(Carro* nuevo);

// Devuelve el primer carro en la cola de Tiempo Real
Carro* primero_tiempo_real();

// Elimina el primer carro de la cola de Tiempo Real
void eliminar_primero_tiempo_real();

// Ingresa un carro al algoritmo de Tiempo Real
void tiempo_real_ingresar(Carro* carro, int tiempo_maximo);

// Marca que un carro ha salido de la carretera
void tiempo_real_salir(Carro* carro);

#endif // C_TIEMPO_REAL_H