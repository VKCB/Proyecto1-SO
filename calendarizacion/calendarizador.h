#ifndef CALENDARIZADOR_H
#define CALENDARIZADOR_H

typedef struct {
    int id;
    int prioridad;
    int tiempo;
} Carro;

typedef enum {
    PRIORIDAD,
    TIEMPO_REAL,
    RR,
    SJF,
    FCFS
} Algoritmo;

void configurar_algoritmo(Algoritmo algoritmo);

// Funciones para el algoritmo de Prioridad
void prioridad_ingresar(Carro* carro);
void prioridad_salir(Carro* carro);

// Funciones para el algoritmo de Tiempo Real
void tiempo_real_ingresar(Carro* carro, int tiempo_maximo);
void tiempo_real_salir(Carro* carro);

// Funciones generales
void ingresar_carro(Carro* carro);
void salir_carro(Carro* carro);

#endif // CALENDARIZADOR_H