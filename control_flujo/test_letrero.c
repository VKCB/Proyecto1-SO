#include "../CEthreads/CEthreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "Equidad.h"

#define NUM_CARROS 10
#define VALOR_W 3  // Número de carros que pueden pasar por lado antes de alternar

// Funciones definidas por el sistema de control EQUIDAD
extern void iniciar_control_equidad(int);
extern void* controlador_letrero_equidad(void*);
extern void esperar_turno_equidad(Car*);

// Funciones definidas por el sistema de control LETRERO
extern void iniciar_control();
extern void* controlador_letrero(void*);
extern void esperar_turno(Car*);



// Rutina que sigue cada carro al ser creado
void* rutina_carro(void* arg) {
    Car* car = (Car*)arg;
    // esperar_turno(car);  // Para Algoritmo LETRERO
    esperar_turno_equidad(car);  // Espera y cruza el puente (incluye el usleep)
    return NULL;
}

int main() {
    srand(time(NULL));

    
    // EJEMPLO DE USO DE ALGORITMO LETRERO 
    /*
    iniciar_control();  // Inicializa mutexes, condiciones y variables globales

    // Creamos el hilo del letrero (NO es un carro, así que no uses `Car`)
    Car hilo_letrero_dummy;
    CEthread_create(&hilo_letrero_dummy, controlador_letrero, NULL);
    */


    // EJEMPLO DE USO DE ALGORITMO EQUIDAD 
    iniciar_control_equidad(VALOR_W);  // Inicializa mutexes, condiciones y valor_W

    // Creamos el hilo del letrero
    Car tid_letrero;
    CEthread_create(&tid_letrero, controlador_letrero_equidad, NULL);


    // Creamos los carros e iniciamos sus hilos
    Car carros[NUM_CARROS];
    for (int i = 0; i < NUM_CARROS; i++) {
        carros[i].lugar_inicio = (rand() % 2 == 0) ? LUGAR_IZQUIERDA : LUGAR_DERECHA;
        carros[i].tipo = TIPO_NORMAL;
        carros[i].velocidad = 5.0 + (rand() % 100) / 50.0;  // Vel entre 1.0 y 3.0 km/h
        carros[i].prioridad = 0;
        carros[i].tiempo = 0;

        printf("🛠️ Creando carro %d: Lugar = %s, Velocidad = %.2f km/h\n", i,
               carros[i].lugar_inicio == LUGAR_IZQUIERDA ? "IZQ" : "DER",
               carros[i].velocidad);

        CEthread_create(&carros[i], rutina_carro, &carros[i]);
    }

    // Esperamos a que todos los carros terminen
    for (int i = 0; i < NUM_CARROS; i++) {
        CEthread_join(&carros[i]);
    }

    return 0;
}
