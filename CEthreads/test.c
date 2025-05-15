#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> 
#include "CEthreads.h"
#include <time.h> 

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


//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////


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

Car crear_carro(const char* lado, const char* tipo, float velocidad) {
    Car carro;
    carro.lugar_inicio = strcmp(lado, "izquierda") == 0 ? LUGAR_IZQUIERDA : LUGAR_DERECHA;

    if (strcmp(tipo, "prioritario") == 0) {
        carro.tipo = TIPO_PRIORITARIO;
    } else if (strcmp(tipo, "sport") == 0) {
        carro.tipo = TIPO_SPORT;
    } else {
        carro.tipo = TIPO_NORMAL;
    }

    carro.velocidad = velocidad;
    return carro;
    
}


void crear_hilo_carro(Car* car) {
    CEthread_create(car, rutina_carro, car);
}

void esperar_carro(Car* car) {
    CEthread_join(car);
}

// interfaz para Python
#include "CEthreads.h"

void crear_carro_desde_python(int lado, int tipo, float velocidad) {
    Car carro;
    carro.lugar_inicio = lado == 0 ? LUGAR_IZQUIERDA : LUGAR_DERECHA;
    carro.tipo = tipo;
    carro.velocidad = velocidad;

    CEthread_create(&carro, rutina_carro, &carro);
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


    srand(time(NULL));
    iniciar_control_equidad(VALOR_W);  // Inicializa mutexes, condiciones y valor_W

    Car tid_letrero;
    CEthread_create(&tid_letrero, controlador_letrero_equidad, NULL);



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

        Car* car_ptr = &fila_izquierda[i];
        CEthread_create(car_ptr, rutina_carro, car_ptr);  // car_ptr ya es Car*, guarda tid internamente
    }

    // Crear carros desde la derecha
    for (int i = 0; i < carros_derecha; i++) {
        Car carro;
        carro.lugar_inicio = LUGAR_DERECHA;
        carro.tipo = (i % 3 == 0) ? TIPO_PRIORITARIO : (i % 2 == 0) ? TIPO_SPORT : TIPO_NORMAL;
        carro.velocidad = 30.0f + (i % 3) * 10.0f;
        agregar_a_fila(carro);

        Car* car_ptr = &fila_derecha[i];
        CEthread_create(car_ptr, rutina_carro, car_ptr);
    }

    // Esperar que todos los carros terminen con CEthread_join
    for (int i = 0; i < count_izquierda; i++) {
        CEthread_join(&fila_izquierda[i]);
    }
    for (int i = 0; i < count_derecha; i++) {
        CEthread_join(&fila_derecha[i]);
    }

    printf("✅ Todos los carros han cruzado la carretera.\n");
    return 0;
}

// Función simple que lanza el sistema con carros aleatorios
void iniciar_simulacion(int cantidad_izq, int cantidad_der) {
    iniciar_control_equidad(VALOR_W);  // Inicia el sistema de equidad

    Car tid_letrero;
    CEthread_create(&tid_letrero, controlador_letrero_equidad, NULL);

    srand(time(NULL));

    // Crear carros desde izquierda
    for (int i = 0; i < cantidad_izq; i++) {
        Car* car = malloc(sizeof(Car));
        car->lugar_inicio = LUGAR_IZQUIERDA;
        car->tipo = (i % 3 == 0) ? TIPO_PRIORITARIO : (i % 2 == 0) ? TIPO_SPORT : TIPO_NORMAL;
        car->velocidad = 30.0f + (i % 3) * 10.0f;
        CEthread_create(car, rutina_carro, car);
    }

    // Crear carros desde derecha
    for (int i = 0; i < cantidad_der; i++) {
        Car* car = malloc(sizeof(Car));
        car->lugar_inicio = LUGAR_DERECHA;
        car->tipo = (i % 3 == 0) ? TIPO_PRIORITARIO : (i % 2 == 0) ? TIPO_SPORT : TIPO_NORMAL;
        car->velocidad = 30.0f + (i % 3) * 10.0f;
        CEthread_create(car, rutina_carro, car);
    }

    // ⚠️ Nota: no hay join, ya que no conservamos punteros en este ejemplo.
}
