#include "../CEthreads/CEthreads.h"
#include <stdio.h>
#include <unistd.h>
#include "../config.h"

#define ESPERA_SIN_CARROS_MS 2000  // Espera máxima si no hay carros (en ms)

static LugarInicio lado_actual = LUGAR_IZQUIERDA;
static CEMutex mutex_control;
static CECond cond_izquierda;
static CECond cond_derecha;

static int carros_pasados_turno = 0;
static int esperando_izquierda = 0;
static int esperando_derecha = 0;
static int carros_en_cruce = 0;
static int valor_w_temp;


void iniciar_control_equidad(int valor_w) {
    CEmutex_init(&mutex_control);
    CECond_init(&cond_izquierda);
    CECond_init(&cond_derecha);

    carros_pasados_turno = 0;
    esperando_izquierda = 0;
    esperando_derecha = 0;
}

void* controlador_letrero_equidad(void* arg) {
    valor_w_temp = 0;
    sleep(3);  // Esperar un poco antes de que los carros comiencen

    CEmutex_lock(&mutex_control);
    valor_w_temp = VALOR_W;
    lado_actual = LUGAR_IZQUIERDA;
    carros_pasados_turno = 0;

    printf("\n=== INICIO DE TURNO IZQUIERDA ===\n");
    CECond_broadcast(&cond_izquierda);
    CEmutex_unlock(&mutex_control);

    while (1) {
        CEmutex_lock(&mutex_control);
        int espera_ms = 0;

        while (carros_pasados_turno < valor_w_temp) {
            int quedan_carros = (lado_actual == LUGAR_IZQUIERDA) ? esperando_izquierda : esperando_derecha;

            if (quedan_carros > 0) {
                CEmutex_unlock(&mutex_control);
                usleep(100 * 1000);  // 100 ms
                CEmutex_lock(&mutex_control);
                continue;
            }

            // Si no hay carros esperando, espera un máximo de 2s por si llegan nuevos
            while (espera_ms < ESPERA_SIN_CARROS_MS) {
                quedan_carros = (lado_actual == LUGAR_IZQUIERDA) ? esperando_izquierda : esperando_derecha;
                if (quedan_carros > 0) break;

                CEmutex_unlock(&mutex_control);
                usleep(100 * 1000);
                espera_ms += 100;
                CEmutex_lock(&mutex_control);
            }
            break;
        }

        // Esperar a que todos los carros que comenzaron a cruzar terminen
        while (carros_en_cruce > 0) {
            CEmutex_unlock(&mutex_control);
            usleep(100 * 1000);
            CEmutex_lock(&mutex_control);
        }

        printf("=== FIN DE TURNO: %d carros pasaron ===\n", carros_pasados_turno);
        carros_pasados_turno = 0;

        // Cambiar de lado
        lado_actual = (lado_actual == LUGAR_IZQUIERDA) ? LUGAR_DERECHA : LUGAR_IZQUIERDA;
        printf("\n=== INICIO DE TURNO %s ===\n", lado_actual == LUGAR_IZQUIERDA ? "IZQUIERDA" : "DERECHA");

        if (lado_actual == LUGAR_IZQUIERDA)
            CECond_broadcast(&cond_izquierda);
        else
            CECond_broadcast(&cond_derecha);

        CEmutex_unlock(&mutex_control);
    }

    return NULL;
}


void esperar_turno_equidad(Car* car) {
    float tiempo_cruce = (LONGITUD_CALLE / car->velocidad) * 3600.0f;  // en segundos

    // Registrar carro esperando
    CEmutex_lock(&mutex_control);
    if (car->lugar_inicio == LUGAR_IZQUIERDA)
        esperando_izquierda++;
    else
        esperando_derecha++;
    CEmutex_unlock(&mutex_control);

    while (1) {
        CEmutex_lock(&mutex_control);

        if (car->lugar_inicio == lado_actual && carros_pasados_turno < valor_w_temp) {
            carros_pasados_turno++;

            if (car->lugar_inicio == LUGAR_IZQUIERDA)
                esperando_izquierda--;
            else
                esperando_derecha--;

            carros_en_cruce++;  // Marcar que este carro está cruzando

            printf("Carro puede pasar: %s, tipo=%d, vel=%.2f km/h, toma %.2f s, turno %d/%d\n",
                   car->lugar_inicio == LUGAR_IZQUIERDA ? "IZQ" : "DER",
                   car->tipo, car->velocidad, tiempo_cruce,
                   carros_pasados_turno, valor_w_temp);

            CEmutex_unlock(&mutex_control);
            break;
        }

        if (car->lugar_inicio == LUGAR_IZQUIERDA)
            CECond_wait(&cond_izquierda, &mutex_control);
        else
            CECond_wait(&cond_derecha, &mutex_control);

        CEmutex_unlock(&mutex_control);
    }

    // Simular cruce
    usleep((unsigned int)(tiempo_cruce * 1e6));

    printf("Carro ha cruzado completamente: lugar=%s, vel=%.2f km/h, TID=%d\n",
           car->lugar_inicio == LUGAR_IZQUIERDA ? "IZQ" : "DER",
           car->velocidad, car->tid);

    CEmutex_lock(&mutex_control);
    carros_en_cruce--;  // Marcar que terminó de cruzar

    if (car->lugar_inicio == LUGAR_IZQUIERDA)
        CECond_broadcast(&cond_izquierda);
    else
        CECond_broadcast(&cond_derecha);
    CEmutex_unlock(&mutex_control);
}