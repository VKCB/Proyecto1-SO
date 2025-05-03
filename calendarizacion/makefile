CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared -lpthread
TARGET = libcalendarizador.so

SOURCES = calendarizador.c c_prioridad.c c_tiempo_real.c RR.c CEthreads.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
