// filepath: /home/vale/Escritorio/ProyectoSO/Proyecto1-SO/calendarizacion/c_prioridad.h
#ifndef C_PRIORIDAD_H
#define C_PRIORIDAD_H

#include "calendarizador.h"

void inicializar_prioridad();
void insertar_por_prioridad(Carro* nuevo);
Carro* primero();
void eliminar_primero();
void prioridad_ingresar(Carro* carro);
void prioridad_salir(Carro* carro);

#endif // C_PRIORIDAD_H