import os
import time
import threading
import tkinter as tk
from tkinter import ttk
import subprocess

# === Pantalla de selección de algoritmo ===

class SeleccionAlgoritmoApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Seleccionar Algoritmo de Calendarización")

        # Configurar tamaño y centrar la ventana
        self.root.geometry("800x400")
        self.root.resizable(False, False)  # Evitar redimensionar
        self.center_window(self.root)

        # Cambiar el color de fondo
        self.root.configure(bg="#f0f0f0")

        # Etiqueta de instrucción
        tk.Label(
            root,
            text="Seleccione el algoritmo de calendarización:",
            font=("Arial", 16, "bold"),
            bg="#f0f0f0"
        ).pack(pady=20)

        # Lista de algoritmos
        self.algoritmo = tk.StringVar(value="RR")
        opciones = ["RR", "Prioridad", "SJF", "FCFS", "Tiempo real"]
        self.combo = ttk.Combobox(
            root,
            textvariable=self.algoritmo,
            values=opciones,
            state="readonly",
            font=("Arial", 14)
        )
        self.combo.pack(pady=20)

        # Botón para continuar
        tk.Button(
            root,
            text="Continuar",
            font=("Arial", 14, "bold"),
            bg="#4caf50",
            fg="white",
            command=self.continuar
        ).pack(pady=20)

        # Vincular la tecla 'w' para cerrar la ventana
        self.root.bind("<w>", self.close_program)

        # Configurar el cierre al presionar 'x'
        self.root.protocol("WM_DELETE_WINDOW", self.close_program)

    def center_window(self, window):
        """Centrar la ventana en la pantalla."""
        window.update_idletasks()
        width = window.winfo_width()
        height = window.winfo_height()
        x = (window.winfo_screenwidth() // 2) - (width // 2)
        y = (window.winfo_screenheight() // 2) - (height // 2)
        window.geometry(f"{width}x{height}+{x}+{y}")

    def continuar(self):
        algoritmo = self.algoritmo.get()
        self.root.destroy()  # Cerrar la ventana actual
        if algoritmo == "SJF":
            self.solicitar_parametro(algoritmo, "Tiempo de cada carro:")
        elif algoritmo == "Prioridad":
            self.solicitar_parametro(algoritmo, "Prioridad de los carros:")
        else:
            iniciar_calle(algoritmo, None)

    def solicitar_parametro(self, algoritmo, mensaje):
        # Crear una nueva ventana para solicitar el parámetro adicional
        parametro_root = tk.Tk()
        parametro_root.title(f"Parámetro para {algoritmo}")
        parametro_root.geometry("800x400")
        parametro_root.resizable(False, False)
        self.center_window(parametro_root)

        tk.Label(parametro_root, text=mensaje, font=("Arial", 14)).pack(pady=20)
        parametro_entry = tk.Entry(parametro_root, font=("Arial", 14))
        parametro_entry.pack(pady=20)

        def continuar_con_parametro():
            parametro = parametro_entry.get()
            parametro_root.destroy()
            iniciar_calle(algoritmo, parametro)

        tk.Button(
            parametro_root,
            text="Continuar",
            font=("Arial", 14, "bold"),
            bg="#4caf50",
            fg="white",
            command=continuar_con_parametro
        ).pack(pady=20)

        # Configurar el cierre al presionar 'x'
        parametro_root.protocol("WM_DELETE_WINDOW", parametro_root.destroy)
        parametro_root.mainloop()

    def close_program(self, event=None):
        """Cerrar la ventana principal."""
        print("Cerrando la ventana principal...")
        self.root.destroy()


# === Clase principal de la interfaz ===

class CalleApp:
    def __init__(self, root, algoritmo, parametro):
        self.root = root
        self.root.title(f"Calle de 1 carril - Algoritmo: {algoritmo}")

        # Configurar tamaño y centrar la ventana
        self.root.geometry("800x400")
        self.root.resizable(False, False)  
        self.center_window(self.root)

        # Configuración del lienzo
        self.canvas = tk.Canvas(root, width=800, height=400, bg="white")
        self.canvas.pack()

        # Dibujar la calle (ajustada al tamaño de la ventana)
        self.canvas.create_rectangle(0, 150, 800, 250, fill="gray", outline="gray")

        self.carros = {}
        self.lock = threading.Lock()
        self.running = True
        self.algoritmo = algoritmo
        self.parametro = parametro

        # Iniciar el programa 'test' con el algoritmo y parámetro seleccionados
        self.test_process = None
        self.test_pid = None

        # Vincular la tecla 'w' para cerrar el programa
        self.root.bind("<w>", self.close_program)

        # Configurar el cierre al presionar 'x'
        self.root.protocol("WM_DELETE_WINDOW", self.close_program)

        # Iniciar el bucle de actualización
        threading.Thread(target=self.update_loop, daemon=True).start()
        self.animate()

    def center_window(self, window):
        """Centrar la ventana en la pantalla."""
        window.update_idletasks()
        width = window.winfo_width()
        height = window.winfo_height()
        x = (window.winfo_screenwidth() // 2) - (width // 2)
        y = (window.winfo_screenheight() // 2) - (height // 2)
        window.geometry(f"{width}x{height}+{x}+{y}")

    def close_program(self, event=None):
        """Cerrar el programa al presionar la tecla 'w' o el botón 'x'."""
        print("Cerrando el programa...")
        self.stop()
        self.root.destroy()

    def update_loop(self):
        while self.running:
            threads = self.list_threads()
            print(f"Hilos detectados: {threads}")  # Depuración
            for tid, name in threads:
                print(f"Procesando hilo: TID={tid}, Nombre={name}")  # Depuración
                if tid not in self.carros:
                    try:
                        lado, tipo, velocidad = name.split('_')
                        velocidad = int(velocidad)
                        print(f"Creando carro: Lado={lado}, Tipo={tipo}, Velocidad={velocidad}")  # Depuración
                        x = 0 if lado == "IZQ" else 800
                        y = 180  # Ajustar la posición vertical de los carros
                        dx = velocidad if lado == "IZQ" else -velocidad

                        carro = {
                            "x": x,
                            "y": y,
                            "dx": dx,
                            "rect": self.canvas.create_rectangle(x, y, x+40, y+20, fill="blue"),
                            "text": self.canvas.create_text(x+20, y+10, text=tipo, fill="white")
                        }
                        self.carros[tid] = carro
                    except ValueError:
                        print(f"⚠️ Nombre de hilo inválido: {name}")
            time.sleep(1)

    def animate(self):
        with self.lock:
            to_delete = []
            for tid, carro in self.carros.items():
                print(f"🔄 Actualizando carro: TID={tid}, X={carro['x']}, DX={carro['dx']}")
                carro["x"] += carro["dx"]
                self.canvas.coords(carro["rect"], carro["x"], carro["y"], carro["x"]+40, carro["y"]+20)
                self.canvas.coords(carro["text"], carro["x"]+20, carro["y"]+10)
                if carro["x"] > 850 or carro["x"] < -50:  # Ajustar límites para el nuevo tamaño
                    self.canvas.delete(carro["rect"])
                    self.canvas.delete(carro["text"])
                    to_delete.append(tid)
            for tid in to_delete:
                del self.carros[tid]
        self.root.after(50, self.animate)

    def list_threads(self):
        threads = []
        try:
            task_dir = f"/proc/{self.test_pid}/task"
            for tid in os.listdir(task_dir):
                with open(f"{task_dir}/{tid}/comm", "r") as f:
                    name = f.read().strip()
                    threads.append((int(tid), name))
        except Exception as e:
            print(f"⚠️ Error al listar hilos: {e}")
        return threads

    def stop(self):
        self.running = False
        if self.test_process:
            self.test_process.terminate()
            self.test_process.wait()


# === Función para iniciar la calle ===

def iniciar_calle(algoritmo, parametro):
    # Obtener la ruta base del proyecto
    base_dir = os.path.dirname(os.path.abspath(__file__))
    ejecutable_path = os.path.join(base_dir, "../CEthreads/test")
    ruta_ce = os.path.join(base_dir, "../CEthreads")

    # Verificar si el ejecutable `test` existe
    if not os.path.exists(ejecutable_path):
        print(f"⚠️ El ejecutable 'test' no existe. Asegúrate de compilarlo.")
        return

    # Crear la ventana de la calle
    root = tk.Tk()
    app = CalleApp(root, algoritmo, parametro)

    # Ejecutar el programa `test` como un proceso externo
    try:
        # Construir el comando con el algoritmo y parámetro (si aplica)
        comando = [ejecutable_path, algoritmo]
        if parametro:
            comando.append(parametro)

        app.test_process = subprocess.Popen(
            comando,
            cwd=ruta_ce  # Usar la ruta relativa al directorio base
        )
        app.test_pid = app.test_process.pid
        root.mainloop()
    except KeyboardInterrupt:
        app.stop()


# === Función para compilar los algoritmos ===

def compilar_algoritmos():
    """Compilar el programa de prueba y los algoritmos en la carpeta calendarizacion."""
    base_dir = os.path.dirname(os.path.abspath(__file__))
    carpeta_calendarizacion = os.path.join(base_dir, "../calendarizacion")
    carpeta_ce = os.path.join(base_dir, "../CEthreads")

    # Compilar el programa `test`
    ruta_test = os.path.join(carpeta_ce, "test.c")
    ruta_ejecutable_test = os.path.join(carpeta_ce, "test")

    print("🔨 Compilando el programa de prueba...")
    subprocess.run([
        "gcc", "-o", ruta_ejecutable_test, ruta_test,
        os.path.join(carpeta_calendarizacion, "c_prioridad.c"),
        os.path.join(carpeta_calendarizacion, "c_tiempo_real.c"),
        os.path.join(carpeta_calendarizacion, "FCFS.c"),
        os.path.join(carpeta_calendarizacion, "RR.c"),
        os.path.join(carpeta_calendarizacion, "SJF.c"),
        os.path.join(carpeta_ce, "CEthreads.c"),
        "-lpthread"
    ], check=True)
    print("✅ Compilación completada.")


# === Ejecutar interfaz de selección ===

if __name__ == "__main__":
    compilar_algoritmos()
    root = tk.Tk()
    app = SeleccionAlgoritmoApp(root)
    root.mainloop()
