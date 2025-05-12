#define _GNU_SOURCE
#include "CEthreads.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

// Función para convertir `LugarInicio` a cadena
const char* lugar_to_string(LugarInicio lugar) {
    return lugar == LUGAR_IZQUIERDA ? "Izquierda" : "Derecha";
}

// Función para convertir `TipoCarro` a cadena
const char* tipo_to_string(TipoCarro tipo) {
    switch (tipo) {
        case TIPO_NORMAL: return "Normal";
        case TIPO_SPORT: return "Sport";
        case TIPO_PRIORITARIO: return "Prioritario";
        default: return "Desconocido";
    }
}

// Función que ejecutará cada hilo de carro
void* car_thread(void* arg) {
    Car* car = (Car*)arg;

    // Configurar el nombre del hilo usando `prctl` (ya implementado en CEthreads.c)
    printf("Carro %s de tipo %s a velocidad %.1f km/h esperando cruzar\n",
           lugar_to_string(car->lugar_inicio),
           tipo_to_string(car->tipo),
           car->velocidad);

    // Simula trabajo largo (mantiene el hilo vivo)
    for (int i = 0; i < 5; ++i) {  // Reducido a 5 iteraciones
        printf("🕒 Carro %s de tipo %s sigue esperando...\n",
               lugar_to_string(car->lugar_inicio),
               tipo_to_string(car->tipo));
        sleep(1);  // Reducido a 1 segundo por iteración para pruebas rápidas
    }

    printf("✅ Carro %s de tipo %s ha cruzado\n",
           lugar_to_string(car->lugar_inicio),
           tipo_to_string(car->tipo));

    return NULL;
}

int main() {
    // Crear tres carros de prueba
    Car car1 = { .lugar_inicio = LUGAR_IZQUIERDA, .tipo = TIPO_NORMAL, .velocidad = 30.0f };
    Car car2 = { .lugar_inicio = LUGAR_DERECHA, .tipo = TIPO_SPORT, .velocidad = 50.0f };
    Car car3 = { .lugar_inicio = LUGAR_IZQUIERDA, .tipo = TIPO_PRIORITARIO, .velocidad = 40.0f };

    // Crear los carros pero NO hacer join inmediatamente
    printf("Creando carros (los hilos vivirán más tiempo)\n");

    CEthread_create(&car1, car_thread, &car1);
    CEthread_create(&car2, car_thread, &car2);
    CEthread_create(&car3, car_thread, &car3);

    // Espera para que el monitor pueda ver los hilos
    sleep(2);

    // Ahora sí esperamos que terminen
    CEthread_join(&car1);
    CEthread_join(&car2);
    CEthread_join(&car3);

    printf("🏆 Todos los carros han cruzado\n");

    return 0;
}