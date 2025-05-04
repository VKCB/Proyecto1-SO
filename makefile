CC = gcc
CFLAGS = -Wall -fPIC -I./biblioteca -I./calendarizacion
LDFLAGS = -shared -lpthread
TARGET = calendarizacion/libcalendarizador.so

SOURCES = biblioteca/CEthreads.c calendarizacion/calendarizador.c calendarizacion/c_prioridad.c calendarizacion/c_tiempo_real.c calendarizacion/RR.c calendarizacion/SJF.c calendarizacion/FCFS.c 
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run:
	python3 interfaz/interfaz.py
