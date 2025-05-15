#include "../CEthreads/CEthreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_CARROS 10

// Funciones definidas por el sistema de control
extern void iniciar_control();
extern void* controlador_letrero(void*);
extern void esperar_turno(Car*);

// Rutina que sigue cada carro al ser creado
void* rutina_carro(void* arg) {
    Car* car = (Car*)arg;
    esperar_turno(car);  // Espera y cruza el puente (incluye el usleep)
    return NULL;
}

int main() {
    srand(time(NULL));
    iniciar_control();  // Inicializa mutexes, condiciones y variables globales

    // Creamos el hilo del letrero (NO es un carro, así que no uses `Car`)
    Car hilo_letrero_dummy;
    CEthread_create(&hilo_letrero_dummy, controlador_letrero, NULL);

    // Creamos los carros e iniciamos sus hilos
    Car hilos[NUM_CARROS];
    for (int i = 0; i < NUM_CARROS; i++) {
        hilos[i].lugar_inicio = (rand() % 2 == 0) ? LUGAR_IZQUIERDA : LUGAR_DERECHA;
        hilos[i].tipo = TIPO_NORMAL;
        hilos[i].velocidad = 1.0 + (rand() % 100) / 50.0;  // Velocidad entre 1.0 y 3.0 km/h
        hilos[i].prioridad = 0;
        hilos[i].tiempo = 0;

        printf("🛠️ Creando carro %d: Lugar = %s, Velocidad = %.2f km/h\n", i,
               hilos[i].lugar_inicio == LUGAR_IZQUIERDA ? "IZQ" : "DER",
               hilos[i].velocidad);

        CEthread_create(&hilos[i], rutina_carro, &hilos[i]);
    }

    // Esperamos a que todos los carros terminen de cruzar
    for (int i = 0; i < NUM_CARROS; i++) {
        CEthread_join(&hilos[i]);
    }

    return 0;
}
