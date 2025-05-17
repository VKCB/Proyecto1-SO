# Makefile

CC = gcc
CFLAGS = -Wall -fPIC -I. -Icalendarizacion -Icontrol_flujo -ICEthreads
LDFLAGS = -shared

LIB_NAME = libcalendarizador.so

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

PY_MAIN = interfaz/read-threads.py

all: $(LIB_NAME)

$(LIB_NAME): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.so *.o

# Optional: check Python dependencies and run main Python script
run: $(LIB_NAME)
	@echo "---------- Checking Python dependencies... ----------"
	@python3 -c "import tkinter" 2>/dev/null || (echo '⚠️ tkinter not found. Run: sudo apt install python3-tk' && exit 1)
	@python3 -m pip show pygame >/dev/null 2>&1 || (echo '📦 Installing pygame...' && python3 -m pip install pygame)
	@echo "---------- Running project... ----------"
	@python3 $(PY_MAIN)
