import pygame  
import time   

# Dimensiones de la ventana
ANCHO, ALTO = 800, 400

# Diccionario que define los colores de los carros según su tipo
COLOR_CARROS = {
    "N": (70, 70, 200),      # azul
    "D": (255, 140, 0),      # anaranjado
    "E": (200, 0, 0),        # ojo
}

class Carro:
    def __init__(self, lado, tipo, y_pos):
        # Inicializamos las propiedades del carro
        self.lado = lado  # Lado de inicio ("I" = izquierda, "D" = derecha)
        self.tipo = tipo  # Tipo de carro ("N", "D", "E")
        self.color = COLOR_CARROS[tipo]  # Color según el tipo
        self.y = y_pos  # Posición vertical (carril)
        # Posición inicial en x depende del lado
        self.x = 0 if lado == "I" else ANCHO
        # Velocidad según el tipo de carro
        self.velocidad = 1 if tipo == "N" else 2 if tipo == "D" else 3

    # Método mover el carro
    def mover(self):
        if self.lado == "I":  
            self.x += self.velocidad  
        else:  
            self.x -= self.velocidad  

    # Método para dibujar el carro en la ventana
    def dibujar(self, ventana):
        # Dibujamos un rectángulo que representa el carro
        pygame.draw.rect(ventana, self.color, (self.x, self.y, 40, 20))

# Dibujamos la línea divisoria blanca discontinua
def dibujar_linea_discontinua(ventana, color, x_inicio, x_fin, y, ancho_segmento, espacio):
    x = x_inicio
    while x < x_fin:
        # Dibujamos un segmento de la línea
        pygame.draw.line(ventana, color, (x, y), (x + ancho_segmento, y), 5)
        # Dejamos un espacio entre segmentos
        x += ancho_segmento + espacio

# Función para leer los datos de los carros desde un archivo
def leer_carros():
    carros = []  # Lista para almacenar los carros
    # Abrir informaciòn de los carros
    with open("./entradas/carros.txt", "r") as f:
        lineas = f.readlines()  # Leemos todas las líneas
        for i, linea in enumerate(lineas):
    
            if linea.strip() and not linea.startswith("#"):
                lado, tipo = linea.strip().split()  # Obtenemos lado y tipo
                # Carril superior para carros que van hacia la derecha
                if lado == "I":
                    y_pos = ALTO // 2 - 30  # Ajustamos para el carril superior
                # Carril inferior para carros que van hacia la izquierda
                else:
                    y_pos = ALTO // 2 + 10  # Ajustamos para el carril inferior
                # Creamos un carro y lo añadimos a la lista
                carros.append(Carro(lado, tipo, y_pos))
    print(f"Carros cargados: {len(carros)}")  # Mensaje de depuración
    return carros  # Devolvemos la lista de carros

def main():
    pygame.init()  
    
    ventana = pygame.display.set_mode((ANCHO, ALTO))
    pygame.display.set_caption("Scheduling Cars")

    # Cargamos los carros desde el archivo
    carros = leer_carros()
    reloj = pygame.time.Clock()  # Reloj para controlar los FPS

    corriendo = True  
    while corriendo:
        
        for evento in pygame.event.get():
            if evento.type == pygame.QUIT:  
                corriendo = False  

        # Fondo de la ventana 
        ventana.fill((245, 245, 220))

        # Dibujamos la calle 
        pygame.draw.rect(ventana, (180, 180, 180), (0, ALTO // 4, ANCHO, ALTO // 2))  # Calle más ancha

        # Dibujamos la línea divisoria blanca discontinua
        dibujar_linea_discontinua(ventana, (255, 255, 255), 0, ANCHO, ALTO // 2, 20, 10)

        # Dibujamos y movemos los carros
        for carro in carros:
            carro.mover()  # Actualizamos la posición del carro
            carro.dibujar(ventana)  # Dibujamos el carro en la ventana

        pygame.display.update()  # Actualizamos la pantalla
        reloj.tick(60)  # Limitamos a 60 FPS

    pygame.quit()  


if __name__ == "__main__":
    main()
