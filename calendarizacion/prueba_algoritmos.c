// filepath: /home/vale/Escritorio/ProyectoSO/Proyecto1-SO/calendarizacion/prueba_algoritmos.c
#include "calendarizador.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    Carro carro1 = {1, 3, 0};  // ID: 1, Prioridad: 3
    Carro carro2 = {2, 1, 0};  // ID: 2, Prioridad: 1
    Carro carro3 = {3, 2, 0};  // ID: 3, Prioridad: 2

    printf("Prueba del algoritmo de Prioridad:\n");
    configurar_algoritmo(PRIORIDAD);

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