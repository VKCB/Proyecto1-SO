#include "CEthreads.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

const char* lugar_to_string(LugarInicio lugar) {
    return lugar == LUGAR_IZQUIERDA ? "Izquierda" : "Derecha";
}

const char* tipo_to_string(TipoCarro tipo) {
    switch(tipo) {
        case TIPO_NORMAL: return "Normal";
        case TIPO_SPORT: return "Sport";
        case TIPO_PRIORITARIO: return "Prioritario";
        default: return "Desconocido";
    }
}

void* car_thread(void* arg) {
    Car* car = (Car*)arg;

    // Nombrar el hilo para que se pueda ver en /proc
    char thread_name[50];
    snprintf(thread_name, sizeof(thread_name), "%s_%s_%d", 
             lugar_to_string(car->lugar_inicio), 
             tipo_to_string(car->tipo), 
             (int)car->velocidad);
    pthread_setname_np(pthread_self(), thread_name);

    printf(" Carro %s de tipo %s a velocidad %.1f km/h esperando cruzar\n",
           lugar_to_string(car->lugar_inicio),
           tipo_to_string(car->tipo),
           car->velocidad);

    // Simula trabajo largo (mantiene el thread vivo)
    for (int i = 0; i < 5; ++i) {  // Reducido a 5 iteraciones
        printf("🕒 Carro %s de tipo %s sigue esperando...\n",
               lugar_to_string(car->lugar_inicio),
               tipo_to_string(car->tipo));
        sleep(5);  // Reducido a 5 segundos por iteración
    }

    printf("✅ Carro %s de tipo %s ha cruzado\n",
           lugar_to_string(car->lugar_inicio),
           tipo_to_string(car->tipo));

    return NULL;
}

int main() {
    Car car1 = { .lugar_inicio = LUGAR_IZQUIERDA, .tipo = TIPO_NORMAL, .velocidad = 30.0f };
    Car car2 = { .lugar_inicio = LUGAR_DERECHA, .tipo = TIPO_SPORT, .velocidad = 50.0f };
    Car car3 = { .lugar_inicio = LUGAR_IZQUIERDA, .tipo = TIPO_PRIORITARIO, .velocidad = 40.0f };

    // Crear los carros pero NO hacer join inmediatamente
    printf("Creando carros (los hilos vivirán más tiempo)\n");

    CEthread_create(&car1, car_thread, &car1);
    CEthread_create(&car2, car_thread, &car2);
    CEthread_create(&car3, car_thread, &car3);

    // Espera para que el monitor pueda ver los hilos
    sleep(1);

    // Ahora sí esperamos que terminen
    CEthread_join(&car1);
    CEthread_join(&car2);
    CEthread_join(&car3);

    printf("🏆 Todos los carros han cruzado\n");

    return 0;
}
