import pygame
import time
import threading
from menu_window import MenuWindow
import subprocess
import os
import signal

# This module integrates your thread monitoring code with the traffic simulation

class ThreadMonitor:
    def __init__(self):
        self.all_threads = []
        self.processed_threads = set()    
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
    
    def get_threads(self):
        """Return only newly detected threads that haven't been processed yet."""
        new_threads = []
        for thread in self.all_threads:
            if thread["tid"] not in self.processed_threads:
                new_threads.append(thread)
                self.processed_threads.add(thread["tid"])
        return new_threads

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
        print("Iniciando monitor de proceso 'test'...\n")
        
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
    
    
    # Function to find and kill the test process if it's running
def kill_test_process():
    try:
        # First try using ps command to find the process
        ps_output = subprocess.check_output(["ps", "-ef"], universal_newlines=True)
        for line in ps_output.split('\n'):
            if "./test" in line:
                parts = line.split()
                if len(parts) > 1:
                    pid = int(parts[1])
                    print(f"Killing test process with PID {pid}")
                    os.kill(pid, signal.SIGTERM)
                    time.sleep(0.5)  # Give it time to terminate
                    try:
                        # Check if process still exists
                        os.kill(pid, 0)
                        # If we get here, process still exists, use SIGKILL
                        print(f"Process {pid} still exists, using SIGKILL")
                        os.kill(pid, signal.SIGKILL)
                    except OSError:
                        # Process is already gone
                        pass
        
        # Alternative method using pkill
        try:
            subprocess.run(["pkill", "-f", "./test"], check=False)
        except Exception:
            pass
            
    except Exception as e:
        print(f"Error trying to kill test process: {e}")


# Main program to integrate the thread monitor with the traffic simulation
if __name__ == "__main__":

    kill_test_process()
    # Show menu window first
    menu = MenuWindow()
    config = menu.show()
    
    if config:

        calendarizador = config["scheduling"]
        flujo = config["control"]

        # Run the C thread creation binary in background
        print(f"Starting thread simulation: ./test {calendarizador} {flujo}")
        process = subprocess.Popen(["./test", calendarizador, flujo])

        # Wait briefly to allow C simulation to start producing threads
        time.sleep(1)
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
        
        # Set initial algorithm based on menu selection
        if config['control'] == 'Letrero':
            simulation.algorithm = Algorithm.LETRERO
        elif config['control'] == 'Equidad':
            simulation.algorithm = Algorithm.EQUIDAD
        elif config['control'] == 'FIFO':
            simulation.algorithm = Algorithm.FIFO
        
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
                        if event.key == pygame.K_w:
                            running = False
                            print("Closing simulation...")
                
                # Check for new threads every second
                current_time = time.time()
                if current_time - last_thread_check >= 1:
                    new_threads = monitor.get_threads()
                    
                    # Add any new threads as cars
                    for thread in new_threads:
                        # No need to check if already exists since get_threads now only returns new ones
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
            # Make sure to terminate the test process if it was started
            if process is not None:
                print("Terminating test process...")
                process.terminate()
                try:
                    # Wait for process to terminate with timeout
                    process.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    # Kill the process if it doesn't terminate
                    print("Process didn't terminate, killing it...")
                    process.kill()
            
            # Make sure no test processes are left running
            kill_test_process()
            pygame.quit()
            print("Simulation terminated")
