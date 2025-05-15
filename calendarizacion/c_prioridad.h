// filepath: /home/vale/Escritorio/ProyectoSO/Proyecto1-SO/calendarizacion/c_prioridad.h
#ifndef C_PRIORIDAD_H
#define C_PRIORIDAD_H

#include "calendarizador.h" // Si calendarizador.h está en el mismo directorio
// #include "../calendarizacion/calendarizador.h" // Si está en un subdirectorio

void inicializar_prioridad();
void insertar_por_prioridad(Car* nuevo);
Car* primero_prioridad();
void eliminar_primero_prioridad();
void prioridad_ingresar(Car* carro);
void prioridad_salir(Car* carro);
void ordenar_por_prioridad(Car* fila, int count);

#endif // C_PRIORIDAD_H