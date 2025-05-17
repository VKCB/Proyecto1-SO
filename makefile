CC = gcc
CFLAGS = -Wall -fPIC -I. -Icalendarizacion -Icontrol_flujo -ICEthreads
LDFLAGS = -shared

LIB_NAME = libcalendarizador.so
TEST_SRC = test.c
TEST_EXE = test
PY_MAIN = interfaz/read-threads.py

SRC = \
    CEthreads/CEthreads.c \
    calendarizacion/calendarizador.c \
    calendarizacion/c_prioridad.c \
    calendarizacion/c_tiempo_real.c \
    calendarizacion/RR.c \
    calendarizacion/SJF.c \
    calendarizacion/FCFS.c \
    control_flujo/Equidad.c \
    control_flujo/Letrero.c \
    control_flujo/FIFO.c \
    config.c

# Objetivo principal: compilar la librería y el ejecutable test
all: $(LIB_NAME) $(TEST_EXE)

# Compilar biblioteca compartida
$(LIB_NAME): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilar el ejecutable 'test' de forma directa como indicaste
$(TEST_EXE): $(TEST_SRC)
	$(CC) $(TEST_SRC) \
		calendarizacion/c_prioridad.c \
		calendarizacion/FCFS.c \
		calendarizacion/c_tiempo_real.c \
		calendarizacion/RR.c \
		calendarizacion/SJF.c \
		control_flujo/Equidad.c \
		control_flujo/Letrero.c \
		CEthreads/CEthreads.c \
		control_flujo/FIFO.c \
		config.c \
		-o $(TEST_EXE)

# Limpiar archivos generados
clean:
	rm -f *.so *.o $(TEST_EXE)

# Ejecutar el proyecto desde Python
run: all
	@echo "---------- Checking Python dependencies... ----------"
	@python3 -c "import tkinter" 2>/dev/null || (echo '⚠️ tkinter not found. Run: sudo apt install python3-tk' && exit 1)
	@python3 -m pip show pygame >/dev/null 2>&1 || (echo '📦 Installing pygame...' && python3 -m pip install pygame)
	@echo "---------- Running project... ----------"
	@python3 $(PY_MAIN)
