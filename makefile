CC = gcc
CFLAGS = -Wall -fPIC -I./CEthreads -I./control_flujo -I./calendarizacion
LDFLAGS = -shared -lpthread

# Targets de las bibliotecas
CALENDARIZADOR_TARGET = calendarizacion/libcalendarizador.so
SIMULACION_TARGET = libsimulacion.so

# Archivos fuente
SOURCES = CEthreads/CEthreads.c \
          CEthreads/test.c \
          calendarizacion/calendarizador.c \
          calendarizacion/c_prioridad.c \
          calendarizacion/c_tiempo_real.c \
          calendarizacion/RR.c \
          calendarizacion/SJF.c \
          calendarizacion/FCFS.c \
          control_flujo/Equidad.c \
          control_flujo/Letrero.c

OBJECTS = $(SOURCES:.c=.o)

# Archivos de prueba opcional
TEST_EXEC = test
TEST_SRC = CEthreads/test.c
TEST_DEPS = CEthreads/CEthreads.c control_flujo/Equidad.c control_flujo/Letrero.c

# Regla principal
all: $(CALENDARIZADOR_TARGET) $(SIMULACION_TARGET)

# Generar libcalendarizador.so
$(CALENDARIZADOR_TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Generar libsimulacion.so
$(SIMULACION_TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^ -lpthread

# Compilar objetos individuales
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Ejecutar interfaz principal en Python
run: $(SIMULACION_TARGET)
	python3 interfaz/interfaz.py

# Ejecutar prueba (test)
run-test: $(TEST_EXEC)
	./$(TEST_EXEC) Prioridad 3 3

$(TEST_EXEC): $(TEST_SRC) $(TEST_DEPS)
	$(CC) -Wall -I./CEthreads -I./control_flujo -I./calendarizacion \
	      $(TEST_SRC) $(TEST_DEPS) -o $(TEST_EXEC) -lpthread

# Limpieza
clean:
	rm -f $(OBJECTS) $(CALENDARIZADOR_TARGET) $(SIMULACION_TARGET) $(TEST_EXEC)
