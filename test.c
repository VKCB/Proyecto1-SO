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

    printf("🚗 Carro %s de tipo %s a velocidad %.1f km/h esperando cruzar\n",
           lugar_to_string(car->lugar_inicio),
           tipo_to_string(car->tipo),
           car->velocidad);

    // Simula avanzar
    sleep(1);

    printf("✅ Carro %s de tipo %s ha cruzado\n",
           lugar_to_string(car->lugar_inicio),
           tipo_to_string(car->tipo));

    return NULL;
}

void* mutex_thread(void* arg) {
    CEMutex* mutex = (CEMutex*)arg;
    
    printf("🔒 Intentando bloquear mutex\n");
    CEmutex_lock(mutex);
    printf("🔓 Mutex bloqueado\n");
    
    // Simulate some work with the mutex locked
    sleep(1);
    
    printf("🔒 Desbloqueando mutex\n");
    CEmutex_unlock(mutex);
    
    return NULL;
}

int main() {
    Car car1 = { .lugar_inicio = LUGAR_IZQUIERDA, .tipo = TIPO_NORMAL, .velocidad = 30.0f };
    Car car2 = { .lugar_inicio = LUGAR_DERECHA, .tipo = TIPO_SPORT, .velocidad = 50.0f };
    Car car3 = { .lugar_inicio = LUGAR_IZQUIERDA, .tipo = TIPO_PRIORITARIO, .velocidad = 40.0f };

    // Test thread creation and joining
    printf("🚗 Creando y esperando que los carros crucen\n");

    CEthread_create(&car1, car_thread, &car1);
    CEthread_join(&car1);

    CEthread_create(&car2, car_thread, &car2);
    CEthread_join(&car2);

    CEthread_create(&car3, car_thread, &car3);
    CEthread_join(&car3);

    printf("🏆 Todos los carros han cruzado\n");

    // Test mutex functionality
    CEMutex mutex;
    CEmutex_init(&mutex);

    pthread_t t1, t2;

    // Create two threads that will test the mutex
    pthread_create(&t1, NULL, mutex_thread, &mutex);
    pthread_create(&t2, NULL, mutex_thread, &mutex);

    // Wait for the mutex threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    CEmutex_destroy(&mutex);

    return 0;
}
