import pygame
import sys
import random
import time
from enum import Enum
from collections import deque
import ctypes

# Load the shared library
lib = ctypes.CDLL("./libcalendarizador.so")

# Initialize pygame
pygame.init()

# Access the global variables
valor_w = ctypes.c_int.in_dll(lib, "VALOR_W").value
intervalo_turno = ctypes.c_int.in_dll(lib, "INTERVALO_TURNO").value

# Constants
WIDTH, HEIGHT = 800, 600
ROAD_WIDTH = 200
CAR_WIDTH, CAR_HEIGHT = 60, 30
STREET_COLOR = (100, 100, 100)
BACKGROUND_COLOR = (50, 200, 50)
FPS = 60

# Colors
RED = (255, 0, 0)
BLUE = (0, 0, 255)
GREEN = (0, 255, 0)
YELLOW = (255, 255, 0)
PURPLE = (128, 0, 128)
GREY = (150, 150, 150)
WHITE = (255, 255, 255)

# Algorithms Enum
class Algorithm(Enum):
    LETRERO = 0
    EQUIDAD = 1
    FIFO = 2

# Direction Enum
class Direction(Enum):
    LEFT = 0
    RIGHT = 1

class Car:
    def __init__(self, tid, lado, tipo, velocidad, tiempo):
        self.tid = tid
        self.lado = lado
        self.tipo = tipo
        self.velocidad = velocidad
        self.tiempo = tiempo
        self.creation_time = time.time()  # When the car was created
        self.start_crossing_time = None   # When the car started crossing
        self.detection_time = time.time()  # When the thread was detected
        
        # Set position based on side
        if lado == "IZQ":
            self.x = 0 - CAR_WIDTH
            self.direction = Direction.RIGHT
        else:  # "DER"
            self.x = WIDTH
            self.direction = Direction.LEFT
        
        # Set color based on tipo
        if tipo == "NORM":
            self.color = GREY
        elif tipo == "SPRT":
            self.color = BLUE
        elif tipo == "PRIO":
            self.color = RED
            self.has_stripe = True  # For red and white pattern
        else:
            self.color = YELLOW  # Default color for unknown types
            self.has_stripe = False
            
        self.y = HEIGHT // 2 - CAR_HEIGHT // 2
        self.in_transit = False
        self.passed = False
        self.ready_to_cross = False  # Will be set to True after 3-second delay
        
    def move(self):
        current_time = time.time()
        
        # Set car ready after 3-second delay from detection
        if not self.ready_to_cross and current_time - self.detection_time >= 0:
            self.ready_to_cross = True
        
        # If car is in transit, calculate progress based on tiempo attribute
        if self.in_transit:
            if self.start_crossing_time is None:
                self.start_crossing_time = current_time
            
            # Calculate progress as a percentage of total crossing time
            elapsed = current_time - self.start_crossing_time
            progress_percent = min(elapsed / self.tiempo, 1.0)
            
            if self.direction == Direction.RIGHT:
                # Calculate position based on elapsed time and total time
                total_distance = WIDTH + CAR_WIDTH
                self.x = -CAR_WIDTH + (progress_percent * total_distance)
                if progress_percent >= 1.0:
                    self.passed = True
            else:
                # For cars coming from right
                total_distance = WIDTH + CAR_WIDTH
                self.x = WIDTH - (progress_percent * total_distance)
                if progress_percent >= 1.0:
                    self.passed = True
    
    def draw(self, screen):
        # Draw basic car rectangle
        pygame.draw.rect(screen, self.color, (self.x, self.y, CAR_WIDTH, CAR_HEIGHT))
        
        # Draw white stripe if it's a PRIO car
        if hasattr(self, 'has_stripe') and self.has_stripe:
            stripe_rect = pygame.Rect(self.x + CAR_WIDTH//4, self.y, CAR_WIDTH//2, CAR_HEIGHT)
            pygame.draw.rect(screen, WHITE, stripe_rect)
        
        # Draw headlights
        if self.direction == Direction.RIGHT:
            pygame.draw.circle(screen, YELLOW, (int(self.x + CAR_WIDTH - 5), int(self.y + 10)), 5)
            pygame.draw.circle(screen, YELLOW, (int(self.x + CAR_WIDTH - 5), int(self.y + CAR_HEIGHT - 10)), 5)
        else:
            pygame.draw.circle(screen, YELLOW, (int(self.x + 5), int(self.y + 10)), 5)
            pygame.draw.circle(screen, YELLOW, (int(self.x + 5), int(self.y + CAR_HEIGHT - 10)), 5)

class TrafficSimulation:
    def __init__(self, all_threads):
        self.screen = pygame.display.set_mode((WIDTH, HEIGHT))
        pygame.display.set_caption("Traffic Simulation")
        self.clock = pygame.time.Clock()
        
        # Create cars from threads
        self.cars = []
        for thread in all_threads:
            car = Car(
                thread['tid'],
                thread['lado'],
                thread['tipo'],
                thread['velocidad'],
                thread['tiempo']
            )
            if 'detected_time' in thread:
                car.detection_time = thread['detected_time']
            self.cars.append(car)
        
        # Traffic control variables
        self.algorithm = Algorithm.LETRERO
        self.current_direction = Direction.RIGHT  # Start with right direction
        self.switch_time = intervalo_turno  # seconds for Letrero
        self.cars_to_pass = valor_w  # number of cars for Equidad
        self.cars_passed = 0
        self.last_switch = time.time()
        
        # Queue for FIFO algorithm
        self.car_queue = deque(sorted(self.cars, key=lambda car: car.creation_time))
        
        # Current car in transit
        self.current_car = None
        
        # Font for text display
        self.font = pygame.font.Font(None, 36)
    
    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_1:
                    self.algorithm = Algorithm.LETRERO
                    print("Algorithm switched to LETRERO")
                elif event.key == pygame.K_2:
                    self.algorithm = Algorithm.EQUIDAD
                    print("Algorithm switched to EQUIDAD")
                elif event.key == pygame.K_3:
                    self.algorithm = Algorithm.FIFO
                    print("Algorithm switched to FIFO")
    
    def update(self):
        # Check if current car has passed
        if self.current_car and self.current_car.passed:
            self.current_car = None
        
        # If no car is currently in transit, apply the algorithm to select the next car
        if self.current_car is None:
            # Apply the selected algorithm
            if self.algorithm == Algorithm.LETRERO:
                self.apply_letrero_algorithm()
            elif self.algorithm == Algorithm.EQUIDAD:
                self.apply_equidad_algorithm()
            elif self.algorithm == Algorithm.FIFO:
                self.apply_fifo_algorithm()
        
        # Move the current car if there is one
        if self.current_car:
            self.current_car.move()
            
        # Move other cars but don't allow them to enter transit
        for car in self.cars:
            if car != self.current_car:
                car.move()  # This will only update ready_to_cross status
        
        # Remove passed cars from the lists
        passed_cars = [car for car in self.cars if car.passed]
        for passed_car in passed_cars:
            if passed_car in self.cars:
                self.cars.remove(passed_car)
                try:
                    self.car_queue.remove(passed_car)
                except ValueError:
                    pass  # Car might not be in the queue anymore
        
        # If current car has passed, reset it
        if self.current_car and self.current_car.passed:
            self.current_car = None

            
    
    def apply_letrero_algorithm(self):
        # Check if there are any cars on the current side
        cars_on_current_side = any(car for car in self.cars if 
                                not car.in_transit and not car.passed and
                                ((car.direction == Direction.RIGHT and self.current_direction == Direction.RIGHT) or
                                (car.direction == Direction.LEFT and self.current_direction == Direction.LEFT)))
        
        # Switch direction if there are no cars on current side
        if not cars_on_current_side:
            self.current_direction = Direction.LEFT if self.current_direction == Direction.RIGHT else Direction.RIGHT
            self.last_switch = time.time()
            print(f"No cars on current side. Switching direction to {'RIGHT' if self.current_direction == Direction.RIGHT else 'LEFT'}")
            return  # Return to avoid checking timer right after switching
        
        # Normal time-based direction switching
        current_time = time.time()
        if current_time - self.last_switch > self.switch_time:
            self.current_direction = Direction.LEFT if self.current_direction == Direction.RIGHT else Direction.RIGHT
            self.last_switch = current_time
            print(f"Time expired. Switching direction to {'RIGHT' if self.current_direction == Direction.RIGHT else 'LEFT'}")
        
        # Find a car that's ready to cross in the current direction
        ready_cars = [car for car in self.cars if 
                    car.ready_to_cross and not car.in_transit and not car.passed and
                    ((car.direction == Direction.RIGHT and self.current_direction == Direction.RIGHT) or
                    (car.direction == Direction.LEFT and self.current_direction == Direction.LEFT))]
        
        if ready_cars:
            # First look for PRIO cars (priority vehicles)
            prio_cars = [car for car in ready_cars if car.tipo == "PRIO"]
            if prio_cars:
                self.current_car = prio_cars[0]
            else:
                self.current_car = ready_cars[0]
            
            self.current_car.in_transit = True

    def apply_equidad_algorithm(self):
        # Get cars ready to cross on current side
        ready_cars = [car for car in self.cars if 
                     car.ready_to_cross and not car.in_transit and not car.passed and
                     ((car.direction == Direction.RIGHT and self.current_direction == Direction.RIGHT) or
                      (car.direction == Direction.LEFT and self.current_direction == Direction.LEFT))]
        
        # If we've passed enough cars or there are no ready cars on the current side, switch sides
        if self.cars_passed >= self.cars_to_pass or not ready_cars:
            if not ready_cars or self.cars_passed >= self.cars_to_pass:
                self.current_direction = Direction.LEFT if self.current_direction == Direction.RIGHT else Direction.RIGHT
                self.cars_passed = 0
                print(f"Switching direction to {'RIGHT' if self.current_direction == Direction.RIGHT else 'LEFT'}")
                return  # Wait for next update to select a car
        
        # If we have a car ready to cross in the current direction
        if ready_cars:
            self.current_car = ready_cars[0]
            self.current_car.in_transit = True
            self.cars_passed += 1
    
    def apply_fifo_algorithm(self):
        # Filter out cars that aren't ready yet
        ready_queue = deque([car for car in self.car_queue if car.ready_to_cross and not car.in_transit and not car.passed])
        
        # If there are ready cars in the queue
        if ready_queue:
            oldest_car = ready_queue[0]
            
            # If current direction doesn't match the oldest car, switch direction
            if oldest_car.direction != self.current_direction:
                self.current_direction = oldest_car.direction
                print(f"Cambiando dirección a  {'RIGHT' if self.current_direction == Direction.RIGHT else 'LEFT'}")
            
            # Let the car pass
            self.current_car = oldest_car
            self.current_car.in_transit = True
    
    def draw(self):
        # Background
        self.screen.fill(BACKGROUND_COLOR)
        
        # Draw the road
        road_rect = pygame.Rect(0, HEIGHT // 2 - ROAD_WIDTH // 2, WIDTH, ROAD_WIDTH)
        pygame.draw.rect(self.screen, STREET_COLOR, road_rect)
        
        # Draw center line
        pygame.draw.line(self.screen, (255, 255, 255), (0, HEIGHT // 2), (WIDTH, HEIGHT // 2), 2)
        
        # Draw cars
        for car in self.cars:
            car.draw(self.screen)
        
        # Draw algorithm information
        algorithm_text = f"Control de flujo: {self.algorithm.name}"
        text_surface = self.font.render(algorithm_text, True, (0, 0, 0))
        self.screen.blit(text_surface, (10, 10))
        
        direction_text = f"Dirección: {'RIGHT -->' if self.current_direction == Direction.RIGHT else '<-- LEFT'}"
        text_surface = self.font.render(direction_text, True, (0, 0, 0))
        self.screen.blit(text_surface, (10, 50))
        
        # Draw algorithm-specific information
        if self.algorithm == Algorithm.LETRERO:
            time_text = f"Tiempo hasta el cambio de dirección: {int(self.switch_time - (time.time() - self.last_switch))}"
            text_surface = self.font.render(time_text, True, (0, 0, 0))
            self.screen.blit(text_surface, (10, 90))
        elif self.algorithm == Algorithm.EQUIDAD:
            count_text = f"Carros cruzados: {self.cars_passed}/{self.cars_to_pass}"
            text_surface = self.font.render(count_text, True, (0, 0, 0))
            self.screen.blit(text_surface, (10, 90))
        
        # Draw car waiting counts
        cars_left = sum(1 for car in self.cars if car.direction == Direction.RIGHT and not car.in_transit and not car.passed)
        cars_right = sum(1 for car in self.cars if car.direction == Direction.LEFT and not car.in_transit and not car.passed)
        cars_text = f"Carros IZQ: {cars_left} | Carros DER: {cars_right}"
        text_surface = self.font.render(cars_text, True, (0, 0, 0))
        self.screen.blit(text_surface, (10, 130))
        
        # Draw current car info if available
        if self.current_car:
            car_info = f"CARRO PASANDO: TID {self.current_car.tid}, Type {self.current_car.tipo}, Time {self.current_car.tiempo}s"
            text_surface = self.font.render(car_info, True, (0, 0, 0))
            self.screen.blit(text_surface, (10, 170))
            
            # Show crossing progress
            if self.current_car.start_crossing_time:
                elapsed = time.time() - self.current_car.start_crossing_time
                progress = min(elapsed / self.current_car.tiempo, 1.0) * 100
                progress_text = f"Progreso: {int(progress)}%"
                text_surface = self.font.render(progress_text, True, (0, 0, 0))
                self.screen.blit(text_surface, (10, 210))
        

        # Display 3s waiting indicator for cars not yet ready
        waiting_cars = [car for car in self.cars if not car.ready_to_cross]
        if waiting_cars:
            waiting_text = f"Cars waiting for 3s delay: {len(waiting_cars)}"
            text_surface = self.font.render(waiting_text, True, (0, 0, 0))
            self.screen.blit(text_surface, (WIDTH - text_surface.get_width() - 10, 50))
        
        pygame.display.flip()
    
    def run(self):
        while True:
            self.handle_events()
            self.update()
            self.draw()
            self.clock.tick(FPS)

# Example usage with mock data
if __name__ == "__main__":
    # Mock data to test the simulation
    all_threads = []
    
    sim = TrafficSimulation(all_threads)
    sim.run()
