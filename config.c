// config.c
#include "config.h"
#include "CEthreads/CEthreads.h"

// Puedes modificar estos valores directamente antes de compilar
double LONGITUD_CALLE = 0.100;
int INTERVALO_TURNO = 60;
int VALOR_W = 3;


// Inicialización de los carros en la calle izquierda
Car carros_izquierda[] = {
    {0, 1, 1, 0, 0, LUGAR_IZQUIERDA, TIPO_NORMAL, 40.0f, 1, 0, 0, 0, 0, 0.0f},  // Cambiado NULL a 0
    {0, 1, 2, 0, 0, LUGAR_IZQUIERDA, TIPO_SPORT, 60.0f, 2, 0, 0, 0, 0, 0.0f},    // Cambiado NULL a 0
    {0, 1, 3, 0, 0, LUGAR_IZQUIERDA, TIPO_NORMAL, 40.0f, 1, 0, 0, 0, 0, 0.0f},  // Cambiado NULL a 0
    {0, 1, 4, 0, 0, LUGAR_IZQUIERDA, TIPO_PRIORITARIO, 40.0f, 3, 0, 0, 0, 0, 0.0f}  // Cambiado NULL a 0
};

// Cálculo del número de carros en la fila izquierda
int cantidad_izquierda = sizeof(carros_izquierda) / sizeof(Car);

// Inicialización de los carros en la calle derecha
Car carros_derecha[] = {
    {0, 1, 1, 0, 0, LUGAR_DERECHA, TIPO_NORMAL, 40.0f, 1, 0, 0, 0, 0, 0.0f},  // Cambiado NULL a 0
    {0, 1, 2, 0, 0, LUGAR_DERECHA, TIPO_NORMAL, 40.0f, 1, 0, 0, 0, 0, 0.0f},  // Cambiado NULL a 0
    {0, 1, 3, 0, 0, LUGAR_DERECHA, TIPO_NORMAL, 40.0f, 1, 0, 0, 0, 0, 0.0f}   // Cambiado NULL a 0
};
// Cálculo del número de carros en la fila derecha
int cantidad_derecha = sizeof(carros_derecha) / sizeof(Car);