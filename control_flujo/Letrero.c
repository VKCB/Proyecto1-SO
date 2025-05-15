#include "../CEthreads/CEthreads.h"
#include <stdio.h>
#include <unistd.h>

#define INTERVALO_TURNO 60  // en segundos
#define LONGITUD_CALLE 0.010  // en km

static LugarInicio lado_actual = LUGAR_INICIO;
static CEMutex mutex_control;
static CECond cond_izquierda;
static CECond cond_derecha;

static int carros_en_turno = 0;
static float tiempo_restante_turno = 0.0;

void iniciar_control() {
    CEmutex_init(&mutex_control);
    CECond_init(&cond_izquierda);
    CECond_init(&cond_derecha);
}


void* controlador_letrero(void* arg) {
    sleep(3);  // Esperar fase INICIO
    CEmutex_lock(&mutex_control);
    lado_actual = LUGAR_IZQUIERDA;
    tiempo_restante_turno = INTERVALO_TURNO;
    printf("\n=== INICIO DE TURNO IZQUIERDA ===\n");
    CECond_broadcast(&cond_izquierda);
    CEmutex_unlock(&mutex_control);

    while (1) {
        sleep(INTERVALO_TURNO);

        CEmutex_lock(&mutex_control);
        printf("=== FIN DE TURNO: %d carros pasaron ===\n", carros_en_turno);
        carros_en_turno = 0;

        lado_actual = (lado_actual == LUGAR_IZQUIERDA) ? LUGAR_DERECHA : LUGAR_IZQUIERDA;
        tiempo_restante_turno = INTERVALO_TURNO;
        printf("\n=== INICIO DE TURNO %s ===\n",
               lado_actual == LUGAR_IZQUIERDA ? "IZQUIERDA" : "DERECHA");

        if (lado_actual == LUGAR_IZQUIERDA)
            CECond_broadcast(&cond_izquierda);
        else
            CECond_broadcast(&cond_derecha);

        CEmutex_unlock(&mutex_control);
    }
    return NULL;
}

void esperar_turno(Car* car) {
    float tiempo_cruce = (LONGITUD_CALLE / car->velocidad) * 3600.0;  // en segundos

    while (1) {
        CEmutex_lock(&mutex_control);

        // Verificamos si puede pasar
        if (car->lugar_inicio == lado_actual && tiempo_restante_turno >= tiempo_cruce) {
            tiempo_restante_turno -= tiempo_cruce;
            carros_en_turno++;

            printf("🚗 Carro puede pasar: %s, tipo=%d, vel=%.2f km/h, toma %.2f s, tiempo restante: %.2f s\n",
                   car->lugar_inicio == LUGAR_IZQUIERDA ? "IZQ" : "DER",
                   car->tipo, car->velocidad, tiempo_cruce, tiempo_restante_turno);

            CEmutex_unlock(&mutex_control);
            break;  // Sale del bucle para cruzar
        }

        // Espera hasta que cambie la condición
        if (car->lugar_inicio == LUGAR_IZQUIERDA)
            CECond_wait(&cond_izquierda, &mutex_control);
        else
            CECond_wait(&cond_derecha, &mutex_control);

        CEmutex_unlock(&mutex_control);
        // El hilo volverá a entrar al while a verificar si puede cruzar
    }

    // Ya fuera del mutex: simula el cruce
    usleep((unsigned int)(tiempo_cruce * 1e6));

    printf("✅ Carro ha cruzado completamente: lugar=%s, vel=%.2f km/h, TID=%d\n",
       car->lugar_inicio == LUGAR_IZQUIERDA ? "IZQ" : "DER",
       car->velocidad,
       car->tid);

    // Opcional: volver a despertar a los carros del mismo lado
    CEmutex_lock(&mutex_control);
    if (car->lugar_inicio == LUGAR_IZQUIERDA)
        CECond_broadcast(&cond_izquierda);
    else
        CECond_broadcast(&cond_derecha);
    CEmutex_unlock(&mutex_control);

}
