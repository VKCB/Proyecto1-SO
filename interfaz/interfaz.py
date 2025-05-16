import os
import time
import threading
import tkinter as tk
from tkinter import ttk
import subprocess

from ctypes import *
import ctypes 
from ctypes import Structure, c_int, c_float, c_void_p, c_long, c_char_p


lib = CDLL("./libsimulacion.so")
# Define la estructura de Carro mapeada de C a Python
class Car(Structure):
    _fields_ = [
        ("tid", c_long),            # pid_t generalmente es un entero largo
        ("activo", c_int),          # int
        ("id", c_int),              # int
        ("stack", c_void_p),        # void* (puntero genérico)
        ("done", c_int),            # volatile int
        ("lugar_inicio", c_int),    # LugarInicio (enum, representado como int)
        ("tipo", c_int),            # TipoCarro (enum, representado como int)
        ("velocidad", c_float),     # float
        ("prioridad", c_int),       # int
        ("tiempo", c_int)           # int
    ]   
c_lado = "IZQ"  # Lado del carro, puedes modificarlo según sea necesario
tipo = "deportivo"  # Tipo de carro, este es un ejemplo
dx = 10.0  # Por ejemplo, la velocidad
    
lib.crear_carro_desde_python(c_lado.encode('utf-8'), tipo.encode('utf-8'), ctypes.c_float(abs(dx)))
# Define la función
lib.crear_carro.argtypes = [c_char_p, c_char_p, c_float]
lib.crear_carro.restype = Car
lib.controlador_letrero_equidad.argtypes = [c_void_p]
lib.controlador_letrero_equidad.restype = c_void_p

lib.crear_hilo_carro.argtypes = [POINTER(Car)]
lib.esperar_carro.argtypes = [POINTER(Car)]

# Para obtener las filas procesadas por los algoritmos de calendarizacion
lib.obtener_fila_izquierda.restype = ctypes.POINTER(Car)
lib.obtener_fila_derecha.restype = ctypes.POINTER(Car)
lib.obtener_count_izquierda.restype = ctypes.c_int
lib.obtener_count_derecha.restype = ctypes.c_int

# Crear un carro desde Python
#carro = lib.crear_carro(b"izquierda", b"sport", 50.0)

# Pasarlo por referencia
#lib.crear_hilo_carro(byref(carro))
#lib.esperar_carro(byref(carro))

# === Pantalla de selección de algoritmo ===

class SeleccionAlgoritmoApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Seleccionar Algoritmo de Calendarización")
        self.root.geometry("800x400")
        self.root.resizable(False, False)
        self.center_window(self.root)
        self.root.configure(bg="#f0f0f0")

        tk.Label(
            root,
            text="Seleccione el algoritmo de calendarización:",
            font=("Arial", 16, "bold"),
            bg="#f0f0f0"
        ).pack(pady=20)

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

        tk.Button(
            root,
            text="Continuar",
            font=("Arial", 14, "bold"),
            bg="#4caf50",
            fg="white",
            command=self.continuar
        ).pack(pady=20)

        self.root.bind("<w>", self.close_program)
        self.root.protocol("WM_DELETE_WINDOW", self.close_program)

    def center_window(self, window):
        window.update_idletasks()
        width = window.winfo_width()
        height = window.winfo_height()
        x = (window.winfo_screenwidth() // 2) - (width // 2)
        y = (window.winfo_screenheight() // 2) - (height // 2)
        window.geometry(f"{width}x{height}+{x}+{y}")

    def continuar(self):
        algoritmo = self.algoritmo.get()
        self.root.destroy()
        self.solicitar_parametro(algoritmo, "Ingrese los parámetros:")

    def solicitar_parametro(self, algoritmo, mensaje):
        parametro_root = tk.Tk()
        parametro_root.title(f"Parámetros para {algoritmo}")
        parametro_root.geometry("800x400")
        parametro_root.resizable(False, False)

        tk.Label(parametro_root, text=mensaje, font=("Arial", 14)).pack(pady=10)

        tk.Label(parametro_root, text="Autos de izquierda a derecha:", font=("Arial", 12)).pack(pady=5)
        izquierda_entry = tk.Entry(parametro_root, font=("Arial", 12))
        izquierda_entry.pack(pady=5)

        tk.Label(parametro_root, text="Autos de derecha a izquierda:", font=("Arial", 12)).pack(pady=5)
        derecha_entry = tk.Entry(parametro_root, font=("Arial", 12))
        derecha_entry.pack(pady=5)

        tipos = ["Normal", "Deportivo", "Emergencia"]
        prioridad_vars = {}

        if algoritmo == "Prioridad":
            tk.Label(parametro_root, text="Seleccione la prioridad única para cada tipo (1=menor, 3=mayor):", font=("Arial", 12)).pack(pady=10)
            frame_prioridad = tk.Frame(parametro_root)
            frame_prioridad.pack(pady=5)
            opciones = ["1", "2", "3"]
            for tipo in tipos:
                tk.Label(frame_prioridad, text=tipo, font=("Arial", 12)).pack(side=tk.LEFT, padx=5)
                var = tk.StringVar(value=opciones[tipos.index(tipo)])
                combo = ttk.Combobox(frame_prioridad, values=opciones, textvariable=var, state="readonly", width=3, font=("Arial", 12))
                combo.pack(side=tk.LEFT, padx=10)
                prioridad_vars[tipo] = var

        def continuar_con_parametros():
            izquierda = izquierda_entry.get()
            derecha = derecha_entry.get()
            if not izquierda.isdigit() or not derecha.isdigit():
                tk.Label(parametro_root, text="Ambos valores deben ser números enteros positivos.", fg="red").pack()
                return

            if algoritmo == "Prioridad":
                prioridades = [prioridad_vars[tipo].get() for tipo in tipos]
                extra = ",".join(prioridades)
                if len(set(prioridades)) != 3:
                    tk.Label(parametro_root, text="Cada tipo debe tener una prioridad diferente (1, 2 y 3).", fg="red").pack()
                    return
                parametro = f"{izquierda},{derecha},{extra}"
            else:
                parametro = f"{izquierda},{derecha}"

            parametro_root.destroy()
            iniciar_calle(algoritmo, parametro)

        tk.Button(
            parametro_root,
            text="Continuar",
            font=("Arial", 14, "bold"),
            bg="#4caf50",
            fg="white",
            command=continuar_con_parametros
        ).pack(pady=20)

        parametro_root.protocol("WM_DELETE_WINDOW", parametro_root.destroy)
        parametro_root.mainloop()

    def close_program(self, event=None):
        print("Cerrando la ventana principal...")
        self.root.destroy()

# === Clase principal de la interfaz ===

class CalleApp:
    def __init__(self, root, algoritmo, parametro):
        self.root = root
        self.root.title(f"Scheduling Cars- Algoritmo: {algoritmo}")
        self.root.geometry("800x400")
        self.root.resizable(False, False)
        self.center_window(self.root)

        self.canvas = tk.Canvas(root, width=800, height=400, bg="white")
        self.canvas.pack()

        # --- Botón para volver al menú ---
        self.boton_volver = tk.Button(
            root,
            text="Volver al menú",
            font=("Arial", 12, "bold"),
            bg="#2196f3",
            fg="white",
            command=self.volver_al_menu
        )
        self.boton_volver.place(x=650, y=20, width=130, height=35)
        # ----------------------------------

        self.canvas.create_rectangle(0, 150, 800, 250, fill="gray", outline="gray")

        self.carros = {}
        self.lock = threading.Lock()
        self.running = True
        self.algoritmo = algoritmo
        self.parametro = parametro
        self.tipo_carro_actual = 0
        self.test_process = None

        self.tipos_carros = [
            {"tipo": "Normal", "velocidad": 5, "color": "blue"},
            {"tipo": "Deportivo", "velocidad": 10, "color": "red"},
            {"tipo": "Emergencia", "velocidad": 15, "color": "yellow"}
        ]

        self.root.bind("<o>", lambda e: self.generar_carro("IZQ"))
        self.root.bind("<p>", lambda e: self.generar_carro("DER"))
        self.root.protocol("WM_DELETE_WINDOW", self.close_program)
        self.root.bind("<w>", self.close_program)

        threading.Thread(target=self.update_loop, daemon=True).start()
        self.animate()

    def center_window(self, window):
        window.update_idletasks()
        width = window.winfo_width()
        height = window.winfo_height()
        x = (window.winfo_screenwidth() // 2) - (width // 2)
        y = (window.winfo_screenheight() // 2) - (height // 2)
        window.geometry(f"{width}x{height}+{x}+{y}")

    def close_program(self, event=None):
        print("Cerrando el programa...")
        self.stop()
        self.root.destroy()

    def volver_al_menu(self):
        self.stop()
        self.root.destroy()
        # Volver a mostrar el menú inicial
        root_menu = tk.Tk()
        app_menu = SeleccionAlgoritmoApp(root_menu)
        root_menu.mainloop()

    # funcion para obtener filas y cantidad de carros en cada fila desde el archivo test.c
    def obtener_filas_desde_c(self):
        """
        Obtiene las filas izquierda y derecha desde el programa en C.
        """
        # Obtener el número de carros en cada fila
        count_izquierda = lib.obtener_count_izquierda()
        count_derecha = lib.obtener_count_derecha()
        # Obtener los punteros a las filas
        fila_izquierda = lib.obtener_fila_izquierda()
        fila_derecha = lib.obtener_fila_derecha()

        # Depuración: Imprimir los punteros
        #print(f"Fila izquierda puntero: {fila_izquierda}")
        #print(f"Fila derecha puntero: {fila_derecha}")


        # print de conteos para probar
        print(f"Count izquierda python: {count_izquierda}, Count derecha python: {count_derecha}")

        

        # Convertir las filas en listas de Python
        carros_izquierda = [
            {
                "tid": fila_izquierda[i].tid,
                "activo": fila_izquierda[i].activo,
                "id": fila_izquierda[i].id,
                "lugar_inicio": fila_izquierda[i].lugar_inicio,
                "tipo": fila_izquierda[i].tipo,
                "velocidad": fila_izquierda[i].velocidad,
                "prioridad": fila_izquierda[i].prioridad,
                "tiempo": fila_izquierda[i].tiempo
            }
            for i in range(count_izquierda)
        ]

        carros_derecha = [
            {
                "tid": fila_derecha[i].tid,
                "activo": fila_derecha[i].activo,
                "id": fila_derecha[i].id,
                "lugar_inicio": fila_derecha[i].lugar_inicio,
                "tipo": fila_derecha[i].tipo,
                "velocidad": fila_derecha[i].velocidad,
                "prioridad": fila_derecha[i].prioridad,
                "tiempo": fila_derecha[i].tiempo
            }
            for i in range(count_derecha)
        ]

        # print de filas convertidas
        print("Carros izquierda son estos:", carros_izquierda)
        print("Carros derecha son estos:", carros_derecha)
    
        return carros_izquierda, carros_derecha    

    def generar_carro(self, lado, extra=None, pos_fila=None, tipo=None):
        with self.lock:
            # Obtener las filas desde C
            carros_izquierda, carros_derecha = self.obtener_filas_desde_c()
    
            # Seleccionar la fila correspondiente
            if lado == "IZQ":
                carros_en_fila = carros_izquierda
                fila_y = 150
                x = 0
                dx = 10  # Velocidad por defecto
            elif lado == "DER":
                carros_en_fila = carros_derecha
                fila_y = 270
                x = 760
                dx = -10  # Velocidad por defecto
            else:
                print(f"Lado inválido: {lado}")
                return
    
            # Generar los carros en la interfaz
            for i, carro in enumerate(carros_en_fila):
                y = fila_y + i * 30
                color = "blue" if carro["tipo"] == 0 else "red" if carro["tipo"] == 1 else "yellow"
                self.carros[i] = {
                    "x": x,
                    "y": y,
                    "dx": dx,
                    "rect": self.canvas.create_rectangle(x, y, x + 40, y + 20, fill=color),
                    "text": self.canvas.create_text(x + 20, y + 10, text=str(carro["velocidad"]), fill="white"),
                    "tipo": carro["tipo"],
                    "lado": lado,
                    "estado": "esperando"
                }
            """tid = threading.get_ident() + len(self.carros)
            self.carros[tid] = carro

            # Llama a la función en C para crear el hilo
            lib.crear_carro_desde_python(c_lado, tipo, c_float(abs(dx)))"""

            print("size of carros_derecha: " + str(len(carros_derecha)))
            #print(f"Carro generado: Lado={lado}, Tipo={tipo}, Extra={extra}, Estado={estado}, ID={tid}")

    def animate(self):
        with self.lock:
            to_delete = []
            for lado in ["IZQ", "DER"]:
                carros_lado = [ (tid, c) for tid, c in self.carros.items() if c["lado"] == lado ]
                if not carros_lado:
                    continue
                for idx, (tid, carro) in enumerate(sorted(carros_lado, key=lambda x: x[1]["y"])):
                    if carro["estado"] == "cruzando":
                        if carro["y"] != 180:
                            carro["y"] = 180
                            self.canvas.coords(carro["rect"], carro["x"], carro["y"], carro["x"]+40, carro["y"]+20)
                            self.canvas.coords(carro["text"], carro["x"]+20, carro["y"]+10)
                        carro["x"] += carro["dx"]
                        self.canvas.coords(carro["rect"], carro["x"], carro["y"], carro["x"]+40, carro["y"]+20)
                        self.canvas.coords(carro["text"], carro["x"]+20, carro["y"]+10)
                        if carro["x"] > 850 or carro["x"] < -50:
                            self.canvas.delete(carro["rect"])
                            self.canvas.delete(carro["text"])
                            to_delete.append(tid)
                            esperando = [ (tid2, c2) for tid2, c2 in sorted(carros_lado, key=lambda x: x[1]["y"]) if c2["estado"] == "esperando" ]
                            if esperando:
                                siguiente_tid, siguiente_carro = esperando[0]
                                siguiente_carro["estado"] = "cruzando"
                                siguiente_carro["y"] = 180
                                self.canvas.coords(siguiente_carro["rect"], siguiente_carro["x"], siguiente_carro["y"], siguiente_carro["x"]+40, siguiente_carro["y"]+20)
                                self.canvas.coords(siguiente_carro["text"], siguiente_carro["x"]+20, siguiente_carro["y"]+10)
                            # --- Mueve hacia adelante los que estaban esperando detrás ---
                            fila_y = 40 if lado == "IZQ" else 270
                            fila_step = 30
                            # Solo los que están esperando y tienen y > que el que salió
                            for tid2, c2 in esperando[1:]:
                                old_y = c2["y"]
                                c2["y"] -= fila_step
                                self.canvas.coords(c2["rect"], c2["x"], c2["y"], c2["x"]+40, c2["y"]+20)
                                self.canvas.coords(c2["text"], c2["x"]+20, c2["y"]+10)
                        break

            for tid in to_delete:
                del self.carros[tid]

        if self.running and self.root.winfo_exists():
            self.root.after(50, self.animate)

    def update_loop(self):
        while self.running:
            # This part calls the C function to get the list of active threads (cars)
            threads = self.list_threads()  # You'll need to implement `list_threads` properly.
            
            # Iterate over the threads and simulate each car's movement
            for tid, name in threads:
                if tid not in self.carros:
                    try:
                        # Verifica que 'name' tenga al menos 3 partes al separar por '_'
                        name_parts = name.split('_')
                        if len(name_parts) != 3:
                            #print(f"Error: nombre de hilo {name} no tiene el formato esperado")
                            continue  # Salta este hilo si no tiene el formato adecuado

                        lado, tipo, velocidad = name_parts
                        velocidad = int(velocidad)
                        
                        if lado == "IZQ":
                            x = 0
                            dx = velocidad
                        elif lado == "DER":
                            x = 800
                            dx = -velocidad
                        else:
                            continue
                        
                        y = 180
                        color = "blue"
                        for tc in self.tipos_carros:
                            if tc["tipo"] == tipo:
                                color = tc["color"]

                        carro = {
                            "x": x,
                            "y": y,
                            "dx": dx,
                            "rect": self.canvas.create_rectangle(x, y, x + 40, y + 20, fill=color),
                            "text": self.canvas.create_text(x + 20, y + 10, text=name, fill="white"),
                            "tipo": tipo,
                            "lado": lado,
                            "estado": "esperando"
                        }
                        self.carros[tid] = carro
                    except Exception as e:
                        print(f"Error creating car from thread {tid}: {e}")
            time.sleep(0.05)  # Wait for the next update cycle


    def list_threads(self):
        threads = []
        if not hasattr(self, "test_pid"):
            return threads
        try:
            task_dir = f"/proc/{self.test_pid}/task"
            for tid in os.listdir(task_dir):
                with open(f"{task_dir}/{tid}/comm", "r") as f:
                    name = f.read().strip()
                    threads.append((int(tid), name))
        except Exception as e:
            print(f" Error al listar hilos: {e}")
        return threads

    def stop(self):
        self.running = False
        if self.test_process:
            self.test_process.terminate()
            self.test_process.wait()

# === Función para iniciar la calle ===

def iniciar_calle(algoritmo, parametro):
    base_dir = os.path.dirname(os.path.abspath(__file__))
    ejecutable_path = os.path.join(base_dir, "../CEthreads/test")
    ruta_ce = os.path.join(base_dir, "../CEthreads")

    if not os.path.exists(ejecutable_path):
        print(f" El ejecutable 'test' no existe. Asegúrate de compilarlo.")
        return

    root = tk.Tk()
    app = CalleApp(root, algoritmo, parametro)

    partes = parametro.split(",")
    izquierda = int(partes[0])
    derecha = int(partes[1])
    extra = partes[2] if len(partes) > 2 else ""

    tipos = ["Normal", "Deportivo", "Emergencia"]

    if algoritmo == "Prioridad":
        # Asegura que los valores sean enteros y no haya espacios
        prioridades = [int(x.strip()) for x in extra.split(",")]
        prioridad_por_tipo = dict(zip(tipos, prioridades))
        carros_izq = [
            {"lado": "IZQ", "extra": prioridad_por_tipo[tipos[i % len(tipos)]], "tipo": tipos[i % len(tipos)]}
            for i in range(izquierda)
        ]
        carros_der = [
            {"lado": "DER", "extra": prioridad_por_tipo[tipos[i % len(tipos)]], "tipo": tipos[i % len(tipos)]}
            for i in range(derecha)
        ]
        carros_izq = sorted(carros_izq, key=lambda c: c["extra"], reverse=True)
        carros_der = sorted(carros_der, key=lambda c: c["extra"], reverse=True)
    elif algoritmo in ["SJF", "Tiempo real"]:
        tiempos = [int(x) for x in extra.split(",") if x.strip().isdigit()]
        carros_izq = [
            {"lado": "IZQ", "extra": tiempos[i] if i < len(tiempos) else 1, "tipo": tipos[i % len(tipos)]}
            for i in range(izquierda)
        ]
        carros_der = [
            {"lado": "DER", "extra": tiempos[izquierda + i] if izquierda + i < len(tiempos) else 1, "tipo": tipos[i % len(tipos)]}
            for i in range(derecha)
        ]
        carros_izq = sorted(carros_izq, key=lambda c: c["extra"])
        carros_der = sorted(carros_der, key=lambda c: c["extra"])
    else:
        carros_izq = [
            {"lado": "IZQ", "extra": 1, "tipo": tipos[i % len(tipos)]}
            for i in range(izquierda)
        ]
        carros_der = [
            {"lado": "DER", "extra": 1, "tipo": tipos[i % len(tipos)]}
            for i in range(derecha)
        ]

    for i, carro in enumerate(carros_izq):
        app.generar_carro("IZQ", extra=carro["extra"], pos_fila=i, tipo=carro["tipo"])
    for i, carro in enumerate(carros_der):
        app.generar_carro("DER", extra=carro["extra"], pos_fila=i, tipo=carro["tipo"])

    try:
        comando = [ejecutable_path, algoritmo]
        if parametro:
            comando.extend(parametro.split(","))

        app.test_process = subprocess.Popen(
            comando,
            cwd=ruta_ce
        )
        app.test_pid = app.test_process.pid
        root.mainloop()
    except KeyboardInterrupt:
        app.stop()

def compilar_algoritmos():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    carpeta_calendarizacion = os.path.join(base_dir, "../calendarizacion")
    carpeta_ce = os.path.join(base_dir, "../CEthreads")
    carpeta_flujo = os.path.join(base_dir, "../control_flujo")

    print(" Compilando el programa de prueba y algoritmos...")
    try:
        subprocess.run([
            "gcc",
            os.path.join(carpeta_calendarizacion, "RR.c"),
            os.path.join(carpeta_calendarizacion, "SJF.c"),
            os.path.join(carpeta_calendarizacion, "FCFS.c"),
            os.path.join(carpeta_calendarizacion, "c_prioridad.c"),
            os.path.join(carpeta_calendarizacion, "c_tiempo_real.c"),
            os.path.join(carpeta_ce, "CEthreads.c"),
            os.path.join(carpeta_flujo, "Equidad.c"),
            os.path.join(carpeta_flujo, "Letrero.c"),
            os.path.join(carpeta_ce, "test.c"),
            "-o",
            os.path.join(carpeta_ce, "test"),
            "-lpthread"
        ], check=True)
        print("Compilación completada")
    except subprocess.CalledProcessError as e:
        print("Error al compilar:", e)

def validar_parametro(parametro):
    try:
        izquierda, derecha = map(int, parametro.split(","))
        return izquierda >= 0 and derecha >= 0
    except ValueError:
        return False

if __name__ == "__main__":
    compilar_algoritmos()
    root = tk.Tk()
    app = SeleccionAlgoritmoApp(root)
    root.mainloop()
