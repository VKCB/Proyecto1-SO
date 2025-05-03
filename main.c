#include "CEthreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Declarar las variables globales necesarias
Car* cola_listos[3]; // Cambia el tamaño según el número de carros
int num_listos = 3;
int tiempos_restantes[3]; // Tiempos de trabajo restantes para cada carro

extern void RR(); // Declarar la función RR

int main() {
    // Inicializar los carros
    Car car1 = { .tid = 0, .lugar_inicio = LUGAR_IZQUIERDA, .tipo = TIPO_NORMAL, .velocidad = 30.0f };
    Car car2 = { .tid = 1, .lugar_inicio = LUGAR_DERECHA, .tipo = TIPO_SPORT, .velocidad = 50.0f };
    Car car3 = { .tid = 2, .lugar_inicio = LUGAR_IZQUIERDA, .tipo = TIPO_PRIORITARIO, .velocidad = 40.0f };

    // Agregar los carros a la cola de listos
    cola_listos[0] = &car1;
    cola_listos[1] = &car2;
    cola_listos[2] = &car3;

    // Inicializar los tiempos restantes
    tiempos_restantes[0] = 1500000; // 1.5 segundos
    tiempos_restantes[1] = 1000000; // 1.0 segundo
    tiempos_restantes[2] = 2000000; // 2.0 segundos

    // Llamar a la función Round Robin
    printf("Iniciando Round Robin...\n");
    RR();

    printf("Prueba completada.\n");
    return 0;
}