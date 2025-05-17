#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> 
#include "CEthreads.h"
#include <time.h> 
#include "../config.h"

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
#define LONGITUD_CALLE 0.100  // en km

// Funciones definidas por el sistema de control EQUIDAD
extern void iniciar_control_equidad(int);
extern void* controlador_letrero_equidad(void*);
extern void esperar_turno_equidad(Car*);

// Funciones definidas por el sistema de control LETRERO
extern void iniciar_control();
extern void* controlador_letrero(void*);
extern void esperar_turno(Car*);

// Funciones definidas por el sistema de control FIFO
extern void iniciar_control_FIFO();
extern void* controlador_letrero_FIFO(void*);
extern void esperar_turno_FIFO(Car*);

// Rutina que sigue cada carro al ser creado
void* rutina_carro_equidad(void* arg) {
    Car* car = (Car*)arg;
    esperar_turno_equidad(car);  // Espera y cruza el puente (incluye el usleep)
    return NULL;
}

void* rutina_carro_letrero(void* arg) {
    Car* car = (Car*)arg;
    esperar_turno(car);  // Espera y cruza el puente (incluye el usleep)
    return NULL;
}

void* rutina_carro_fifo(void* arg) {
    Car* car = (Car*)arg;
    esperar_turno_FIFO(car);
    return NULL;
}

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


// Función para agregar un carro a una fila
void agregar_a_fila(Car carro) {
    if (carro.lugar_inicio == LUGAR_IZQUIERDA) {
        if (count_izquierda < MAX_CARROS) {
            fila_izquierda[count_izquierda++] = carro;
            //printf("se agrega a fila de la izquierda.\n");
        } else {
            printf("La fila de la izquierda está llena.\n");
        }
    } else if (carro.lugar_inicio == LUGAR_DERECHA) {
        if (count_derecha < MAX_CARROS) {
            fila_derecha[count_derecha++] = carro;
            //printf("se agrega a fila de la derecha.\n");
        } else {
            printf("La fila de la derecha está llena.\n");
        }
    }
    else {
       printf("Error: Lugar de inicio desconocido para el carro.\n");
    }
}

// Función para procesar las filas de carros
void procesar_filas(const char* algoritmo) {
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
        int quantum = 2;
    
        // Crear lista unificada
        Car todos_los_carros[MAX_CARROS];
        int total_carros = 0;
    
        // Copiar carros de la izquierda
        for (int i = 0; i < count_izquierda; i++) {
            todos_los_carros[total_carros++] = fila_izquierda[i];
        }
    
        // Copiar carros de la derecha
        for (int i = 0; i < count_derecha; i++) {
            todos_los_carros[total_carros++] = fila_derecha[i];
        }
    
        procesar_rr(todos_los_carros, total_carros, quantum);
    }
    
}

// Función para obtener la fila izquierda
Car* obtener_fila_izquierda() {
    return fila_izquierda;  
}

// Función para obtener la fila derecha
Car* obtener_fila_derecha() {
    return fila_derecha;  
}

// Función para obtener el número de carros en la fila izquierda
int obtener_count_izquierda() {
    printf("Count izquierda desde C: %d\n", count_izquierda);
    return count_izquierda;
}

// Función para obtener el número de carros en la fila derecha
int obtener_count_derecha() {

    printf("Count derecha desde C: %d\n", count_derecha);
    return count_derecha;
}


int main(int argc, char* argv[]) {
    srand(time(NULL));

    if (argc < 2) {
        printf("Uso: %s <algoritmo> <control>\n", argv[0]);
        return 1;
    }

    const char* algoritmo = argv[1];
    const char* control = argv[2];  // Elección de control: "Equidad" o "Letrero"
    
    printf("Algoritmo seleccionado: %s\n", algoritmo);
    printf("Control seleccionado: %s\n", control);


    if (strcmp(control, "Equidad") == 0) {
        iniciar_control_equidad(VALOR_W);
        Car tid_equidad;
        CEthread_create(&tid_equidad, controlador_letrero_equidad, NULL);
    } else if (strcmp(control, "Letrero") == 0) {
        iniciar_control();
        Car tid_letrero;
        CEthread_create(&tid_letrero, controlador_letrero, NULL);
    } else if (strcmp(control, "FIFO") == 0) {
        iniciar_control_FIFO();
        Car tid_fifo;
        CEthread_create(&tid_fifo, controlador_letrero_FIFO, NULL);
    } else {
        printf("Control no reconocido. Debe ser 'Equidad', 'Letrero' o 'FIFO'.\n");
        return 1;
    }
    

    // Lógica de creación de carros desde la izquierda y la derecha
    for (int i = 0; i < cantidad_izquierda; i++) {
        Car carro = carros_izquierda[i];
        carro.tiempo = (int)((LONGITUD_CALLE / carro.velocidad) * 3600.0);
        carro.tiempo_restante = carro.tiempo;  // Inicializa el tiempo restante
        agregar_a_fila(carro);
    }

    // Desde la derecha
    for (int i = 0; i < cantidad_derecha; i++) {
        Car carro = carros_derecha[i];
        carro.tiempo = (int)((LONGITUD_CALLE / carro.velocidad) * 3600.0);
        carro.tiempo_restante = carro.tiempo;  // Inicializa el tiempo restante
        agregar_a_fila(carro);
    }

    // Procesar las filas de carros según el algoritmo
    procesar_filas(algoritmo);

    if (strcmp(algoritmo, "RR") == 0) {
        // Si el algoritmo es Round Robin, manejar el procesamiento aquí
        rr_exec();  // Esta función maneja el procesamiento de los carros con RR
    } else {
        // Seleccionar la función de rutina adecuada según el tipo de control
        void* (*rutina_seleccionada)(void*);
        if (strcmp(control, "Letrero") == 0) {
            rutina_seleccionada = rutina_carro_letrero;
        } else if (strcmp(control, "Equidad") == 0) {
            rutina_seleccionada = rutina_carro_equidad;
        } else if (strcmp(control, "FIFO") == 0) {
            rutina_seleccionada = rutina_carro_fifo;
        } else {
            fprintf(stderr, "Control desconocido: %s\n", control);
            exit(EXIT_FAILURE);
        }
        
    
        // Crear hilos ya ordenados para los demás algoritmos
        for (int i = 0; i < count_izquierda; i++) {
            Car* car_ptr = &fila_izquierda[i];
            CEthread_create(car_ptr, rutina_seleccionada, car_ptr);
        }
    
        for (int i = 0; i < count_derecha; i++) {
            Car* car_ptr = &fila_derecha[i];
            CEthread_create(car_ptr, rutina_seleccionada, car_ptr);
        }
    
        // Esperar que todos los carros terminen
        for (int i = 0; i < count_izquierda; i++) {
            CEthread_join(&fila_izquierda[i]);
        }
        for (int i = 0; i < count_derecha; i++) {
            CEthread_join(&fila_derecha[i]);
        }
    }
    
    CEthread_exit();
    printf("Todos los carros cruzaron la carretera \n");
    return 0;
}


