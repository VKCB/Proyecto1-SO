import pygame
import threading
import ctypes
import time

# Dimensiones de la ventana
ANCHO, ALTO = 800, 400

# Diccionario que define los colores de los carros según su tipo
COLOR_CARROS = {
    "N": (0, 255, 0),      # verde para carros normales
    "D": (0, 0, 255),      # azul para carros deportivos
    "E": (255, 0, 0),      # rojo para carros de emergencia
}

# Clase CarroC que representa la estructura en C
class CarroC(ctypes.Structure):
    _fields_ = [
        ("id", ctypes.c_int),
        ("prioridad", ctypes.c_int),
        ("tiempo", ctypes.c_int),
    ]

# Definición del enum Algoritmo
class Algoritmo:
    PRIORIDAD = 1
    TIEMPO_REAL = 4
    RR = 2
    SJF = 3
    FCFS = 5

# Carga la biblioteca compartida
libcalendarizacion = ctypes.CDLL("../calendarizacion/libcalendarizacion.so")

# Define las funciones de la biblioteca compartida
configurar_algoritmo = libcalendarizacion.configurar_algoritmo
configurar_algoritmo.argtypes = [ctypes.c_int]
configurar_algoritmo.restype = None

ingresar_carro = libcalendarizacion.ingresar_carro
ingresar_carro.argtypes = [ctypes.POINTER(CarroC)]
ingresar_carro.restype = None

salir_carro = libcalendarizacion.salir_carro
salir_carro.argtypes = [ctypes.POINTER(CarroC)]
salir_carro.restype = None

# Clase CarroHilo
class CarroHilo(threading.Thread):
    def __init__(self, lado, tipo, y_pos, carros, lock):
        super().__init__()
        self.lado = lado
        self.tipo = tipo
        self.color = COLOR_CARROS[tipo]  # Asignar color según el tipo
        self.y = y_pos
        self.x = 0 if lado == "I" else ANCHO
        self.velocidad = 1 if tipo == "N" else 2 if tipo == "D" else 3  # Velocidad según el tipo
        self.carros = carros
        self.lock = lock
        self.running = True

    def run(self):
        while self.running:
            with self.lock:
                # Evitar colisiones
                for otro_carro in self.carros:
                    if otro_carro != self:
                        if self.lado == "I" and self.x + self.velocidad + 40 > otro_carro.x and self.y == otro_carro.y:
                            return
                        if self.lado == "D" and self.x - self.velocidad < otro_carro.x + 40 and self.y == otro_carro.y:
                            return

                # Mover el carro
                if self.lado == "I":
                    self.x += self.velocidad
                else:
                    self.x -= self.velocidad

                # Verificar si el carro salió de la pantalla
                if self.x > ANCHO or self.x < 0:
                    carro_c = CarroC(id=self.carros.index(self), prioridad=0, tiempo=0)
                    salir_carro(ctypes.byref(carro_c))
                    self.carros.remove(self)
                    self.running = False

            time.sleep(0.05)  # Controlar la velocidad del hilo

    def dibujar(self, ventana):
        pygame.draw.rect(ventana, self.color, (self.x, self.y, 40, 20))

# Dibujar línea discontinua
def dibujar_linea_discontinua(ventana, color, x_inicio, x_fin, y, ancho_segmento, espacio):
    x = x_inicio
    while x < x_fin:
        pygame.draw.line(ventana, color, (x, y), (x + ancho_segmento, y), 5)
        x += ancho_segmento + espacio

def mostrar_ventana_inicial():
    pygame.init()
    ventana = pygame.display.set_mode((ANCHO, ALTO))
    pygame.display.set_caption("Seleccionar Algoritmo de Calendarización")

    fuente = pygame.font.Font(None, 36)
    opciones = [
        "1. Prioridad",
        "2. Tiempo Real",
        "3. Round Robin",
        "4. SJF (Shortest Job First)",
        "5. FCFS (First Come First Serve)"
    ]
    seleccion = None

    corriendo = True
    while corriendo:
        ventana.fill((200, 200, 200))  # Fondo gris claro

        # Mostrar título
        titulo = fuente.render("Seleccione un algoritmo de calendarización:", True, (0, 0, 0))
        ventana.blit(titulo, (ANCHO // 2 - titulo.get_width() // 2, 50))

        # Mostrar opciones
        for i, opcion in enumerate(opciones):
            texto = fuente.render(opcion, True, (0, 0, 0))
            ventana.blit(texto, (ANCHO // 2 - texto.get_width() // 2, 150 + i * 40))

        pygame.display.update()

        for evento in pygame.event.get():
            if evento.type == pygame.QUIT:
                pygame.quit()
                exit()
            if evento.type == pygame.KEYDOWN:
                if evento.key == pygame.K_1:
                    seleccion = Algoritmo.PRIORIDAD
                    corriendo = False
                elif evento.key == pygame.K_2:
                    seleccion = Algoritmo.TIEMPO_REAL
                    corriendo = False
                elif evento.key == pygame.K_3:
                    seleccion = Algoritmo.RR
                    corriendo = False
                elif evento.key == pygame.K_4:
                    seleccion = Algoritmo.SJF
                    corriendo = False
                elif evento.key == pygame.K_5:
                    seleccion = Algoritmo.FCFS
                    corriendo = False

    return seleccion

# Función principal
def main():
    # Mostrar ventana inicial y obtener la selección del usuario
    algoritmo_seleccionado = mostrar_ventana_inicial()

    # Configurar el algoritmo seleccionado en la biblioteca compartida
    configurar_algoritmo(algoritmo_seleccionado)

    pygame.init()
    ventana = pygame.display.set_mode((ANCHO, ALTO))
    pygame.display.set_caption("Scheduling Cars")

    carros = []
    lock = threading.Lock()
    reloj = pygame.time.Clock()

    corriendo = True
    while corriendo:
        for evento in pygame.event.get():
            if evento.type == pygame.QUIT:
                corriendo = False
            if evento.type == pygame.KEYDOWN:
                if evento.key == pygame.K_w:
                    corriendo = False
                elif evento.key == pygame.K_i:  # Generar carro normal desde la izquierda (carril superior)
                    nuevo_carro = CarroHilo("I", "N", ALTO // 2 - 60, carros, lock)
                    with lock:
                        carros.append(nuevo_carro)
                    nuevo_carro.start()
                elif evento.key == pygame.K_d:  # Generar carro normal desde la derecha (carril inferior)
                    nuevo_carro = CarroHilo("D", "N", ALTO // 2 + 40, carros, lock)
                    with lock:
                        carros.append(nuevo_carro)
                    nuevo_carro.start()
                elif evento.key == pygame.K_e:  # Generar carro de emergencia desde la izquierda (carril superior)
                    nuevo_carro = CarroHilo("I", "E", ALTO // 2 - 60, carros, lock)
                    with lock:
                        carros.append(nuevo_carro)
                    nuevo_carro.start()
                elif evento.key == pygame.K_s:  # Generar carro deportivo desde la derecha (carril inferior)
                    nuevo_carro = CarroHilo("D", "D", ALTO // 2 + 40, carros, lock)
                    with lock:
                        carros.append(nuevo_carro)
                    nuevo_carro.start()
                elif evento.key == pygame.K_r:  # Generar carro de emergencia desde la derecha (carril inferior)
                    nuevo_carro = CarroHilo("D", "E", ALTO // 2 + 40, carros, lock)
                    with lock:
                        carros.append(nuevo_carro)
                    nuevo_carro.start()
                elif evento.key == pygame.K_a:  # Generar carro deportivo desde la izquierda (carril superior)
                    nuevo_carro = CarroHilo("I", "D", ALTO // 2 - 60, carros, lock)
                    with lock:
                        carros.append(nuevo_carro)
                    nuevo_carro.start()

        ventana.fill((245, 245, 220))
        pygame.draw.rect(ventana, (180, 180, 180), (0, ALTO // 4, ANCHO, ALTO // 2))
        dibujar_linea_discontinua(ventana, (255, 255, 255), 0, ANCHO, ALTO // 2, 20, 10)

        with lock:
            for carro in carros:
                carro.dibujar(ventana)

        pygame.display.update()
        reloj.tick(60)

    # Detener todos los hilos
    with lock:
        for carro in carros:
            carro.running = False
    for carro in carros:
        carro.join()

    pygame.quit()

if __name__ == "__main__":
    main()
