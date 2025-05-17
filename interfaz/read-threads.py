import pygame
import sys
import random
import time
import threading
from enum import Enum
from collections import deque

# This module integrates your thread monitoring code with the traffic simulation

class ThreadMonitor:
    def __init__(self):
        self.all_threads = []
        self.monitor_active = False
        self.monitor_thread = None
    
    def get_test_pid(self):
        """Busca el PID del proceso llamado 'test'."""
        import os
        for pid in os.listdir("/proc"):
            if pid.isdigit():
                try:
                    with open(f"/proc/{pid}/comm") as f:
                        if f.read().strip() == "test":
                            return int(pid)
                except FileNotFoundError:
                    continue
        return None

    def list_threads(self, pid):
        """Devuelve una lista de TIDs y nombres de hilos del proceso con PID dado."""
        import os
        task_dir = f"/proc/{pid}/task"
        threads = []
        try:
            for tid in os.listdir(task_dir):
                comm_path = f"{task_dir}/{tid}/comm"
                try:
                    with open(comm_path) as f:
                        name = f.read().strip()
                        threads.append((tid, name))
                except FileNotFoundError:
                    continue
        except FileNotFoundError:
            pass
        return threads

    def monitor_thread_func(self):
        """Function that runs in a separate thread to monitor test process threads."""
        import os
        seen_threads_global = set()
        print("🔄 Iniciando monitor de proceso 'test'...\n")
        
        try:
            while self.monitor_active:
                pid = self.get_test_pid()
                if pid is None:
                    print("⏳ Esperando a que se inicie el proceso 'test'...")
                    time.sleep(2)
                    continue
                
                print(f"✅ Proceso 'test' activo con PID {pid}. Monitoreando hilos...\n")
                seen_threads = set()
                
                while self.monitor_active and os.path.exists(f"/proc/{pid}/task"):
                    threads = self.list_threads(pid)
                    new_threads = [(tid, name) for tid, name in threads if tid not in seen_threads_global]
                    
                    for tid, name in new_threads:
                        try:
                            side, tipo, vel, tiempo = name.split('_')
                            print(f"Hilo detectado - TID: {tid} | Lado: {side} | Tipo: {tipo} | Velocidad: {vel} | Tiempo: {tiempo}")
                            
                            # Add thread to the list with the current time
                            thread_info = {
                                "tid": tid,
                                "lado": side,
                                "tipo": tipo,
                                "velocidad": int(vel),
                                "tiempo": int(tiempo),
                                "detected_time": time.time()
                            }
                            self.all_threads.append(thread_info)
                            
                        except ValueError:
                            print(f"Hilo erróneo - TID: {tid}, Nombre: {name}")
                        
                        seen_threads.add(tid)
                        seen_threads_global.add(tid)
                    
                    time.sleep(1)
                
                print("Proceso 'test' terminó. Reintentando en 2 segundos...\n")
                time.sleep(2)
                
        except Exception as e:
            print(f"Error en el monitor: {e}")
    
    def start_monitoring(self):
        """Start the monitoring thread."""
        if not self.monitor_active:
            self.monitor_active = True
            self.monitor_thread = threading.Thread(target=self.monitor_thread_func)
            self.monitor_thread.daemon = True
            self.monitor_thread.start()
    
    def stop_monitoring(self):
        """Stop the monitoring thread."""
        self.monitor_active = False
        if self.monitor_thread:
            self.monitor_thread.join(timeout=2)
    
    def get_threads(self):
        """Return the current list of threads."""
        return self.all_threads.copy()

# Main program to integrate the thread monitor with the traffic simulation
if __name__ == "__main__":
    # Import the traffic simulation code
    from traffic_simulation import TrafficSimulation, Car, Algorithm, Direction
    
    # Initialize pygame
    pygame.init()
    
    # Constants
    WIDTH, HEIGHT = 800, 600
    FPS = 60
    
    # Setup screen
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Traffic Simulation with Live Thread Monitoring")
    clock = pygame.time.Clock()
    
    # Start thread monitoring
    monitor = ThreadMonitor()
    monitor.start_monitoring()
    
    # Create a simulation with initial empty data
    simulation = TrafficSimulation([])
    
    # Main loop
    running = True
    last_thread_check = time.time()
    
    try:
        while running:
            # Handle events
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_1:
                        simulation.algorithm = Algorithm.LETRERO
                        print("Algorithm switched to LETRERO")
                    elif event.key == pygame.K_2:
                        simulation.algorithm = Algorithm.EQUIDAD
                        print("Algorithm switched to EQUIDAD")
                    elif event.key == pygame.K_3:
                        simulation.algorithm = Algorithm.FIFO
                        print("Algorithm switched to FIFO")
            
            # Check for new threads every second
            current_time = time.time()
            if current_time - last_thread_check >= 1:
                new_threads = monitor.get_threads()
                
                # Add any new threads as cars
                for thread in new_threads:
                    # Check if this thread is already a car
                    if not any(car.tid == thread["tid"] for car in simulation.cars):
                        car = Car(
                            thread['tid'],
                            thread['lado'],
                            thread['tipo'],
                            thread['velocidad'],
                            thread['tiempo']
                        )
                        car.detection_time = thread['detected_time']  # Set detection time from thread
                        simulation.cars.append(car)
                        
                        # If using FIFO algorithm, add to queue
                        if simulation.algorithm == Algorithm.FIFO:
                            simulation.car_queue.append(car)
                
                last_thread_check = current_time
            
            # Update and draw the simulation
            simulation.update()
            simulation.draw()
            
            # Limit the frame rate
            clock.tick(FPS)
    
    finally:
        # Clean up
        monitor.stop_monitoring()
        pygame.quit()
        print("Simulation terminated")
