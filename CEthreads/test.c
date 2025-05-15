#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> 
#include "CEthreads.h" 

#include "../calendarizacion/c_prioridad.h"
#include "../calendarizacion/c_tiempo_real.h"
#include "../calendarizacion/FCFS.h"
#include "../calendarizacion/SJF.h"
#include "../calendarizacion/RR.h"


// Variables globales para las filas de carros
#define MAX_CARROS 100
Car fila_izquierda[MAX_CARROS];
Car fila_derecha[MAX_CARROS];
int count_izquierda = 0;
int count_derecha = 0;

// Función para convertir LugarInicio a cadena
const char* lugar_to_string(LugarInicio lugar) {
    return lugar == LUGAR_IZQUIERDA ? "Izquierda" : "Derecha";
}

// Función para convertir tipoCarro a cadena
const char* tipo_to_string(TipoCarro tipo) {
    switch (tipo) {
        case TIPO_NORMAL: return "Normal";
        case TIPO_SPORT: return "Sport";
        case TIPO_PRIORITARIO: return "Prioritario";
        default: return "Desconocido";
    }
}

// Función para agregar un carro a una fila
void agregar_a_fila(Car carro) {
    if (carro.lugar_inicio == LUGAR_IZQUIERDA) {
        if (count_izquierda < MAX_CARROS) {
            fila_izquierda[count_izquierda++] = carro;
        } else {
            printf("La fila de la izquierda está llena.\n");
        }
    } else if (carro.lugar_inicio == LUGAR_DERECHA) {
        if (count_derecha < MAX_CARROS) {
            fila_derecha[count_derecha++] = carro;
        } else {
            printf("La fila de la derecha está llena.\n");
        }
    }
}

// Función para procesar las filas de carros
void procesar_filas(const char* algoritmo) {
    while (count_izquierda > 0 || count_derecha > 0) {
        if (strcmp(algoritmo, "Prioridad") == 0) {
            ordenar_por_prioridad(fila_izquierda, count_izquierda);
            ordenar_por_prioridad(fila_derecha, count_derecha);
        } else if (strcmp(algoritmo, "TiempoReal") == 0) {
            ordenar_por_tiempo_real(fila_izquierda, count_izquierda);
            ordenar_por_tiempo_real(fila_derecha, count_derecha);
        } else if (strcmp(algoritmo, "FCFS") == 0) {
            ordenar_por_fcfs(fila_izquierda, count_izquierda);
            ordenar_por_fcfs(fila_derecha, count_derecha);
        } else if (strcmp(algoritmo, "SJF") == 0) {
            ordenar_por_sjf(fila_izquierda, count_izquierda);
            ordenar_por_sjf(fila_derecha, count_derecha);
        } else if (strcmp(algoritmo, "RR") == 0) {
            int quantum = 2; // ejemplo
            procesar_rr(fila_izquierda, &count_izquierda, quantum);
            procesar_rr(fila_derecha, &count_derecha, quantum);
            continue; // RR ya procesa la fila, así que salta el resto del ciclo
        }

        // Procesar un carro de la izquierda si hay carros
        if (count_izquierda > 0) {
            Car carro = fila_izquierda[0];
            printf("Procesando carro de la izquierda: Tipo=%s, Velocidad=%.1f km/h\n",
                   tipo_to_string(carro.tipo), carro.velocidad);
            sleep(3); 
            // Mover los carros restantes hacia adelante
            for (int i = 1; i < count_izquierda; i++) {
                fila_izquierda[i - 1] = fila_izquierda[i];
            }
            count_izquierda--;
        }

        // Procesar un carro de la derecha si hay carros
        if (count_derecha > 0) {
            Car carro = fila_derecha[0];
            printf("Procesando carro de la derecha: Tipo=%s, Velocidad=%.1f km/h\n",
                   tipo_to_string(carro.tipo), carro.velocidad);
            sleep(3); 
            // Mover los carros restantes hacia adelante
            for (int i = 1; i < count_derecha; i++) {
                fila_derecha[i - 1] = fila_derecha[i];
            }
            count_derecha--;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Uso: %s <algoritmo> <carros_izquierda> <carros_derecha>\n", argv[0]);
        return 1;
    }

    const char* algoritmo = argv[1];
    int carros_izquierda = atoi(argv[2]);
    int carros_derecha = atoi(argv[3]);

    printf("Algoritmo seleccionado: %s\n", algoritmo);
    printf("Creando %d carros desde la izquierda y %d desde la derecha...\n", carros_izquierda, carros_derecha);

    // Crear carros desde la izquierda
    for (int i = 0; i < carros_izquierda; i++) {
        Car carro;
        carro.lugar_inicio = LUGAR_IZQUIERDA;
        carro.tipo = (i % 3 == 0) ? TIPO_PRIORITARIO : (i % 2 == 0) ? TIPO_SPORT : TIPO_NORMAL;
        carro.velocidad = 30.0f + (i % 3) * 10.0f;
        agregar_a_fila(carro);
    }

    // Crear carros desde la derecha
    for (int i = 0; i < carros_derecha; i++) {
        Car carro;
        carro.lugar_inicio = LUGAR_DERECHA;
        carro.tipo = (i % 3 == 0) ? TIPO_PRIORITARIO : (i % 2 == 0) ? TIPO_SPORT : TIPO_NORMAL;
        carro.velocidad = 30.0f + (i % 3) * 10.0f;
        agregar_a_fila(carro);
    }

    printf("🏁 Todos los carros han sido creados. Procesando filas...\n");

    // Procesar las filas de carros
    procesar_filas(algoritmo);

    printf("✅ Todos los carros han cruzado la carretera.\n");
    return 0;
}