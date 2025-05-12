// filepath: /home/vale/Escritorio/ProyectoSO/Proyecto1-SO/calendarizacion/prueba_algoritmos.c
#include "calendarizador.h"
#include "CEthreads.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    Car carro1 = {
        .tid = 1,               // ID
        .prioridad = 3,         // Priority
        .stack = NULL,          // Default value
        .done = 0,              // Default value
        .lugar_inicio = LUGAR_IZQUIERDA, // Default side
        .tipo = TIPO_NORMAL,    // Default car type
        .velocidad = 0.0f,      // Default speed
        .tiempo = 0             // Default time
    };
    
    Car carro2 = {
        .tid = 2,               // ID
        .prioridad = 1,         // Priority
        .stack = NULL,          // Default value
        .done = 0,              // Default value
        .lugar_inicio = LUGAR_IZQUIERDA, // Default side
        .tipo = TIPO_NORMAL,    // Default car type
        .velocidad = 0.0f,      // Default speed
        .tiempo = 0             // Default time
    };
    
    Car carro3 = {
        .tid = 3,               // ID
        .prioridad = 2,         // Priority
        .stack = NULL,          // Default value
        .done = 0,              // Default value
        .lugar_inicio = LUGAR_IZQUIERDA, // Default side
        .tipo = TIPO_NORMAL,    // Default car type
        .velocidad = 0.0f,      // Default speed
        .tiempo = 0             // Default time
    };

    printf("Prueba del algoritmo de Prioridad:\n");
    configurar_algoritmo(RR);

    ingresar_carro(&carro1);
    ingresar_carro(&carro2);
    ingresar_carro(&carro3);

    salir_carro(&carro1);
    salir_carro(&carro2);
    salir_carro(&carro3);

    printf("Prueba del algoritmo de Tiempo Real:\n");
    configurar_algoritmo(TIEMPO_REAL);

    ingresar_carro(&carro1);
    ingresar_carro(&carro2);
    ingresar_carro(&carro3);

    salir_carro(&carro1);
    salir_carro(&carro2);
    salir_carro(&carro3);

    return 0;
}