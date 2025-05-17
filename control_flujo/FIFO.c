#include "../CEthreads/CEthreads.h"
#include <stdio.h>
#include <unistd.h>
#include "../config.h"

#define ESPERA_SIN_CARROS_MS 2000  // Espera máxima si no hay carros (en ms)

static LugarInicio lado_actual = LUGAR_IZQUIERDA; // empieza del lado izquierdo
static CEMutex mutex_control; // Mutex controla lado_actual
static CECond cond_izquierda;
static CECond cond_derecha;

//contadores de los carros que esperan
static int esperando_izquierda = 0;
static int esperando_derecha = 0;
//contador de los carros que cruzan
static int carros_en_cruce = 0;
static int orden_llegada_izq = 0;
static int orden_llegada_der = 0;

void iniciar_control_FIFO() {
    CEmutex_init(&mutex_control);
    CECond_init(&cond_izquierda);
    CECond_init(&cond_derecha);
}

void* controlador_letrero_FIFO(void* arg) {
    sleep(3);  // Esperar un poco antes de que los carros comiencen

    CEmutex_lock(&mutex_control);
    lado_actual = LUGAR_IZQUIERDA;
    printf("\n=== INICIO DE TURNO IZQUIERDA ===\n");
    CEmutex_unlock(&mutex_control);

    while (1) {
        CEmutex_lock(&mutex_control);
        int espera_ms = 0;

        // Si no hay carros esperando en este lado, espera un máximo de 2s por si llegan nuevos
        while (esperando_izquierda == 0 && esperando_derecha == 0) {
            if (espera_ms >= ESPERA_SIN_CARROS_MS) {
                printf("No hay carros esperando, esperando más tiempo...\n");
                break;
            }

            CEmutex_unlock(&mutex_control);
            usleep(100 * 1000);  // 100 ms
            espera_ms += 100;
            CEmutex_lock(&mutex_control);
        }

        // Procesar los carros que están en espera
        while (esperando_izquierda > 0 || esperando_derecha > 0) {
            int quedan_carros = (lado_actual == LUGAR_IZQUIERDA) ? esperando_izquierda : esperando_derecha;

            if (quedan_carros > 0) {
                CEmutex_unlock(&mutex_control);
                usleep(100 * 1000);  // 100 ms
                CEmutex_lock(&mutex_control);
                continue;
            }

            // Si no hay carros esperando, espera un máximo de 2s por si llegan nuevos
            while (espera_ms < ESPERA_SIN_CARROS_MS) {
                if ((lado_actual == LUGAR_IZQUIERDA && esperando_izquierda > 0) ||
                    (lado_actual == LUGAR_DERECHA && esperando_derecha > 0)) {
                    break;
                }

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

        printf("=== FIN DE TURNO: %d carros pasaron ===\n", esperando_izquierda + esperando_derecha);

        // Cambiar de lado
        lado_actual = (lado_actual == LUGAR_IZQUIERDA) ? LUGAR_DERECHA : LUGAR_IZQUIERDA;
        printf("\n=== INICIO DE TURNO %s ===\n", lado_actual == LUGAR_IZQUIERDA ? "IZQUIERDA" : "DERECHA");

        // Señalizar a los carros del nuevo lado que pueden avanzar
        if (lado_actual == LUGAR_IZQUIERDA) {
            CECond_broadcast(&cond_izquierda);
        } else {
            CECond_broadcast(&cond_derecha);
        }

        CEmutex_unlock(&mutex_control);
    }

    return NULL;
}

void esperar_turno_FIFO(Car* car) {
    float tiempo_cruce = (LONGITUD_CALLE / car->velocidad) * 3600.0f;  // en segundos

    // Verifica si el carro ya esta marcado como terminado
    CEmutex_lock(&car->mutex);
    if (car->terminado) {
        printf("[FIFO] Carro TID=%d ya ha terminado, ignorando.\n", car->tid);
        CEmutex_unlock(&car->mutex);
        return;  // El carro ya cruzó, no hacer nada
    }
    CEmutex_unlock(&car->mutex);

    // Registrar carro esperando
    CEmutex_lock(&mutex_control);
    int orden;
    if (car->lugar_inicio == LUGAR_IZQUIERDA) {
        esperando_izquierda++;
        orden = ++orden_llegada_izq;
        printf("[FIFO] Carro TID=%d registrado como #%d en IZQUIERDA\n", 
               car->tid, orden);
    } else {
        esperando_derecha++;
        orden = ++orden_llegada_der;
        printf("[FIFO] Carro TID=%d registrado como #%d en DERECHA\n", 
               car->tid, orden);
    }
    CEmutex_unlock(&mutex_control);

    while (1) {
        CEmutex_lock(&mutex_control);

        if (car->lugar_inicio == lado_actual) {
            if (car->lugar_inicio == LUGAR_IZQUIERDA) {
                esperando_izquierda--;
            } else {
                esperando_derecha--;
            }

            carros_en_cruce++;  // Marcar que este carro está cruzando
            printf("Carro puede pasar: %s, tipo=%d, vel=%.2f km/h, toma %.2f s\n",
                   car->lugar_inicio == LUGAR_IZQUIERDA ? "IZQ" : "DER",
                   car->tipo, car->velocidad, tiempo_cruce);

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

    // Marcar el carro como terminado
    CEmutex_lock(&car->mutex);
    car->terminado = 1;
    CEmutex_unlock(&car->mutex);

    printf("Carro ha cruzado completamente: lugar=%s, vel=%.2f km/h, orden=#%d, TID=%d\n",
           car->lugar_inicio == LUGAR_IZQUIERDA ? "IZQ" : "DER",
           car->velocidad, orden, car->tid);

    CEmutex_lock(&mutex_control);
    carros_en_cruce--;  // Marcar que terminó de cruzar

    if (car->lugar_inicio == LUGAR_IZQUIERDA)
        CECond_broadcast(&cond_izquierda);
    else
        CECond_broadcast(&cond_derecha);
    CEmutex_unlock(&mutex_control);
}
