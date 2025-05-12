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
        parametro_root.geometry("400x200")
        parametro_root.resizable(False, False)

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

        # Dibujar la calle
        self.canvas.create_rectangle(0, 150, 800, 250, fill="gray", outline="gray")

        self.carros = {}
        self.lock = threading.Lock()
        self.running = True
        self.algoritmo = algoritmo
        self.parametro = parametro
        self.tipo_carro_actual = 0  # Para intercalar tipos de carros
        self.test_process = None  # Inicializar el atributo test_process

        # Tipos de carros y sus propiedades
        self.tipos_carros = [
            {"tipo": "Normal", "velocidad": 5, "color": "blue"},
            {"tipo": "Deportivo", "velocidad": 10, "color": "red"},
            {"tipo": "Emergencia", "velocidad": 15, "color": "yellow"}
        ]

        # Vincular teclas para generar carros
        self.root.bind("<k>", lambda e: self.generar_carro("IZQ"))  # Tecla 'K' para la izquierda
        self.root.bind("<l>", lambda e: self.generar_carro("DER"))  # Tecla 'L' para la derecha

        # Vincular la tecla 'w' para cerrar la ventana
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

    def generar_carro(self, lado):
        """Generar un nuevo carro desde el lado especificado."""
        with self.lock:
            tipo_carro = self.tipos_carros[self.tipo_carro_actual]
            self.tipo_carro_actual = (self.tipo_carro_actual + 1) % len(self.tipos_carros)

            x = 0 if lado == "IZQ" else 800
            y = 180
            dx = tipo_carro["velocidad"] if lado == "IZQ" else -tipo_carro["velocidad"]

            carro = {
                "x": x,
                "y": y,
                "dx": dx,
                "rect": self.canvas.create_rectangle(x, y, x+40, y+20, fill=tipo_carro["color"]),
                "text": self.canvas.create_text(x+20, y+10, text=tipo_carro["tipo"], fill="white"),
                "tipo": tipo_carro["tipo"]
            }

            # Generar un identificador único para el carro
            tid = threading.get_ident() + len(self.carros)
            self.carros[tid] = carro

            # Si es un carro de emergencia, iniciar un temporizador para su prioridad
            if tipo_carro["tipo"] == "Emergencia":
                threading.Thread(target=self.gestionar_prioridad_emergencia, args=(tid,), daemon=True).start()

    def gestionar_prioridad_emergencia(self, tid):
        """Gestionar la prioridad de un carro de emergencia."""
        time.sleep(5)  # Tiempo máximo permitido para cruzar
        with self.lock:
            if tid in self.carros:
                print(f"⚠️ Carro de emergencia {tid} no cruzó a tiempo. Eliminando...")
                self.canvas.delete(self.carros[tid]["rect"])
                self.canvas.delete(self.carros[tid]["text"])
                del self.carros[tid]

    def animate(self):
        with self.lock:
            to_delete = []
            for tid, carro in self.carros.items():
                carro["x"] += carro["dx"]
                self.canvas.coords(carro["rect"], carro["x"], carro["y"], carro["x"]+40, carro["y"]+20)
                self.canvas.coords(carro["text"], carro["x"]+20, carro["y"]+10)
                if carro["x"] > 850 or carro["x"] < -50:
                    self.canvas.delete(carro["rect"])
                    self.canvas.delete(carro["text"])
                    to_delete.append(tid)
            for tid in to_delete:
                del self.carros[tid]
        self.root.after(50, self.animate)

    def update_loop(self):
        """Monitorear los hilos creados por el programa `test`."""
        while self.running:
            threads = self.list_threads()
            for tid, name in threads:
                if tid not in self.carros:
                    try:
                        lado, tipo, velocidad = name.split('_')
                        velocidad = int(velocidad)
                        x = 0 if lado == "IZQ" else 800
                        y = 180
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
