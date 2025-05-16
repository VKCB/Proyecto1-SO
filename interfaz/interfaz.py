import os
import time
import threading
import tkinter as tk
from tkinter import ttk
import subprocess

from ctypes import *
import ctypes 
from ctypes import Structure, c_int, c_float, c_void_p, c_long, c_char_p

# Carga la biblioteca desde la raíz del proyecto
lib = CDLL(os.path.join(os.path.dirname(__file__), "../libsimulacion.so"))

class Car(Structure):
    _fields_ = [
        ("tid", c_long),
        ("activo", c_int),
        ("id", c_int),
        ("stack", c_void_p),
        ("done", c_int),
        ("lugar_inicio", c_int),
        ("tipo", c_int),
        ("velocidad", c_float),
        ("prioridad", c_int),
        ("tiempo", c_int)
    ]   

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

        # Agrega selección de método de control
        tk.Label(parametro_root, text="Método de control:", font=("Arial", 12)).pack(pady=5)
        control_var = tk.StringVar(value="Equidad")
        control_combo = ttk.Combobox(parametro_root, textvariable=control_var, values=["Equidad", "Letrero", "FIFO"], state="readonly", font=("Arial", 12))
        control_combo.pack(pady=5)

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
            control = control_var.get()
            if not izquierda.isdigit() or not derecha.isdigit():
                tk.Label(parametro_root, text="Ambos valores deben ser números enteros positivos.", fg="red").pack()
                return

            if algoritmo == "Prioridad":
                prioridades = [prioridad_vars[tipo].get() for tipo in tipos]
                extra = ",".join(prioridades)
                if len(set(prioridades)) != 3:
                    tk.Label(parametro_root, text="Cada tipo debe tener una prioridad diferente (1, 2 y 3).", fg="red").pack()
                    return
                parametro = f"{izquierda},{derecha},{extra},{control}"
            else:
                parametro = f"{izquierda},{derecha},{control}"

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

        # Etiqueta para mostrar el lado por donde cruza el carro
        self.lado_label = tk.Label(
            root,
            text="Esperando carros...",
            font=("Arial", 18, "bold"),
            bg="#f0f0f0",
            fg="#333"
        )
        self.lado_label.place(x=250, y=10, width=300, height=40)

        # --- Indicadores de sentido ---
        # Izquierda a Derecha
        self.canvas.create_rectangle(10, 110, 170, 140, fill="#e0e0e0", outline="#888")
        self.canvas.create_text(90, 125, text="Izquierda → Derecha", font=("Arial", 12, "bold"), fill="black")
        # Derecha a Izquierda
        self.canvas.create_rectangle(630, 260, 790, 290, fill="#e0e0e0", outline="#888")
        self.canvas.create_text(710, 275, text="Derecha → Izquierda", font=("Arial", 12, "bold"), fill="black")
        # --------------------------------

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

        self.lado_activo = "IZQ"  # Empieza por la izquierda
        self.carros_cruzados_ventana = 0
        self.ventana = 3  # Tamaño de la ventana

        self.root.bind("<o>", lambda e: self.agregar_carro_manual("IZQ"))
        self.root.bind("<p>", lambda e: self.agregar_carro_manual("DER"))
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
                fila_y = 220 + sum(1 for c in self.carros.values() if c["lado"] == "DER") * 30  # Cambia 270 por 220
                x = 760
                dx = -10  # Velocidad por defecto
            else:
                print(f"Lado inválido: {lado}")
                return
    
            # Generar los carros en la interfaz
            for i, carro in enumerate(carros_en_fila):
                y = fila_y + i * 30
                # Asignar color según el tipo
                if carro["tipo"] == 0 or carro["tipo"] == "Normal":
                    color = "blue"
                elif carro["tipo"] == 1 or carro["tipo"] == "Sport":
                    color = "red"
                elif carro["tipo"] == 2 or carro["tipo"] == "Prioridad":
                    color = "yellow"
                else:
                    color = "gray"
                key = f"{lado}_{i}"
                self.carros[key] = {
                    "x": x,
                    "y": y,
                    "dx": dx,
                    "rect": self.canvas.create_rectangle(x, y, x + 40, y + 20, fill=color),
                    "text": self.canvas.create_text(x + 20, y + 10, text=str(carro["velocidad"]), fill="white"),
                    "tipo": carro["tipo"],
                    "lado": lado,
                    "estado": "esperando"
                }

            # Hacer que solo un carro (de cualquier lado) cruce a la vez
            cruzando = any(c["estado"] == "cruzando" for c in self.carros.values())
            if not cruzando:
                esperando = [c for c in self.carros.values() if c["estado"] == "esperando"]
                if esperando:
                    primero = min(esperando, key=lambda c: c["y"])
                    primero["estado"] = "cruzando"
                    # Actualiza la etiqueta del lado
                    if primero["lado"] == "IZQ":
                        self.lado_label.config(text="Cruzando: Izquierda → Derecha")
                    else:
                        self.lado_label.config(text="Cruzando: Derecha → Izquierda")

            print("size of carros_derecha: " + str(len(carros_derecha)))

    def agregar_carro_manual(self, lado):
        with self.lock:
            # Determina la posición y dirección según el lado
            if lado == "IZQ":
                x = 0
                y = 150 + sum(1 for c in self.carros.values() if c["lado"] == "IZQ") * 30
                dx = 10
            elif lado == "DER":
                x = 760
                y = 220 + sum(1 for c in self.carros.values() if c["lado"] == "DER") * 30  # Cambia 270 por 220
                dx = -10
            else:
                return

            # Alterna tipo de carro para ejemplo visual
            tipos = ["Normal", "Deportivo", "Emergencia"]
            tipo = tipos[len(self.carros) % 3]
            color = {"Normal": "blue", "Deportivo": "red", "Emergencia": "yellow"}[tipo]

            key = f"{lado}_manual_{len(self.carros)}"
            self.carros[key] = {
                "x": x,
                "y": y,
                "dx": dx,
                "rect": self.canvas.create_rectangle(x, y, x + 40, y + 20, fill=color),
                "text": self.canvas.create_text(x + 20, y + 10, text=tipo, fill="white"),
                "tipo": tipo,
                "lado": lado,
                "estado": "esperando"
            }

            # Si no hay ningún carro cruzando, deja que este cruce
            cruzando = any(c["estado"] == "cruzando" for c in self.carros.values())
            if not cruzando:
                self.carros[key]["estado"] = "cruzando"
                if lado == "IZQ":
                    self.lado_label.config(text="Cruzando: Izquierda → Derecha")
                else:
                    self.lado_label.config(text="Cruzando: Derecha → Izquierda")

    def animate(self):
        with self.lock:
            to_delete = []
            cruzando = any(c["estado"] == "cruzando" for c in self.carros.values())
            if not cruzando:
                # Busca carros esperando del lado activo
                esperando_lado = [ (tid, c) for tid, c in self.carros.items() if c["estado"] == "esperando" and c["lado"] == self.lado_activo ]
                if esperando_lado and self.carros_cruzados_ventana < self.ventana:
                    # El primero en la fila de ese lado cruza
                    siguiente_tid, siguiente_carro = sorted(esperando_lado, key=lambda x: x[1]["y"])[0]
                    siguiente_carro["estado"] = "cruzando"
                    siguiente_carro["y"] = 180
                    self.canvas.coords(siguiente_carro["rect"], siguiente_carro["x"], siguiente_carro["y"], siguiente_carro["x"]+40, siguiente_carro["y"]+20)
                    self.canvas.coords(siguiente_carro["text"], siguiente_carro["x"]+20, siguiente_carro["y"]+10)
                    # Actualiza la etiqueta del lado
                    if self.lado_activo == "IZQ":
                        self.lado_label.config(text="Cruzando: Izquierda → Derecha")
                    else:
                        self.lado_label.config(text="Cruzando: Derecha → Izquierda")
                    self.carros_cruzados_ventana += 1
                else:
                    # Si ya cruzaron 3 o no hay más en este lado, cambia de lado y reinicia contador
                    otro_lado = "DER" if self.lado_activo == "IZQ" else "IZQ"
                    esperando_otro = [ (tid, c) for tid, c in self.carros.items() if c["estado"] == "esperando" and c["lado"] == otro_lado ]
                    if esperando_otro:
                        self.lado_activo = otro_lado
                        self.carros_cruzados_ventana = 0
                    else:
                        self.lado_label.config(text="Esperando carros...")

            # --- Movimiento y eliminación de carros ---
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
                        break

            for tid in to_delete:
                del self.carros[tid]

            if not any(c["estado"] == "cruzando" for c in self.carros.values()):
                self.lado_label.config(text="Esperando carros...")

        if self.running and self.root.winfo_exists():
            self.root.after(50, self.animate)

    def update_loop(self):
        while self.running:
            threads = self.list_threads()

            for tid, name in threads:
                if tid not in self.carros:
                    try:
                        name_parts = name.split('_')
                        if len(name_parts) != 4:  # Adjusted to handle the new format
                            continue

                        lado, tipo, velocidad, tiempo = name_parts
                        velocidad = int(velocidad)
                        tiempo = int(tiempo)

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
                            "text": self.canvas.create_text(x + 20, y + 10, text=f"{tipo} ({tiempo}s)", fill="white"),
                            "tipo": tipo,
                            "lado": lado,
                            "estado": "esperando"
                        }
                        self.carros[tid] = carro
                    except Exception as e:
                        print(f"Error creating car from thread {tid}: {e}")
            time.sleep(0.05)

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
        print("El ejecutable 'test' no existe. Asegúrate de compilarlo.")
        return

    partes = parametro.split(",")
    if len(partes) < 3:
        print("Error: se necesitan al menos 3 parámetros: izquierda,derecha,control")
        return

    izquierda = int(partes[0])
    derecha = int(partes[1])
    control = partes[-1]  # Último parámetro es el método de control

    controles_validos = ["Equidad", "Letrero", "FIFO"]
    if control not in controles_validos:
        print(f"Error: método de control '{control}' no es válido. Usa: {controles_validos}")
        return

    extra = partes[2:-1]  # Lo que está entre izquierda,derecha y control
    tipos = ["Normal", "Deportivo", "Emergencia"]

    root = tk.Tk()
    app = CalleApp(root, algoritmo, parametro)

    if algoritmo == "Prioridad":
        try:
            prioridades = [int(x.strip()) for x in extra]
        except ValueError:
            print("Error: prioridades deben ser números enteros.")
            return

        if len(prioridades) < len(tipos):
            prioridades *= (len(tipos) // len(prioridades)) + 1

        prioridad_por_tipo = dict(zip(tipos, prioridades[:len(tipos)]))

        carros_izq = [
            {"lado": "IZQ", "extra": prioridad_por_tipo[tipos[i % len(tipos)]], "tipo": tipos[i % len(tipos)]}
            for i in range(izquierda)
        ]
        carros_der = [
            {"lado": "DER", "extra": prioridad_por_tipo[tipos[i % len(tipos)]], "tipo": tipos[i % len(tipos)]}
            for i in range(derecha)
        ]
        carros_izq.sort(key=lambda c: c["extra"], reverse=True)
        carros_der.sort(key=lambda c: c["extra"], reverse=True)

    elif algoritmo in ["SJF", "Tiempo real"]:
        try:
            tiempos = [int(x.strip()) for x in extra]
        except ValueError:
            print("Error: los tiempos deben ser números enteros.")
            return

        carros_izq = [
            {"lado": "IZQ", "extra": tiempos[i] if i < len(tiempos) else 1, "tipo": tipos[i % len(tipos)]}
            for i in range(izquierda)
        ]
        carros_der = [
            {"lado": "DER", "extra": tiempos[izquierda + i] if izquierda + i < len(tiempos) else 1,
             "tipo": tipos[i % len(tipos)]}
            for i in range(derecha)
        ]
        carros_izq.sort(key=lambda c: c["extra"])
        carros_der.sort(key=lambda c: c["extra"])

    else:
        # Algoritmos sin prioridad ni tiempo (FCFS, RR, etc.)
        carros_izq = [
            {"lado": "IZQ", "extra": 1, "tipo": tipos[i % len(tipos)]}
            for i in range(izquierda)
        ]
        carros_der = [
            {"lado": "DER", "extra": 1, "tipo": tipos[i % len(tipos)]}
            for i in range(derecha)
        ]

    for i, carro in enumerate(carros_izq):
        app.generar_carro(carro["lado"], extra=carro["extra"], pos_fila=i, tipo=carro["tipo"])
    for i, carro in enumerate(carros_der):
        app.generar_carro(carro["lado"], extra=carro["extra"], pos_fila=i, tipo=carro["tipo"])

    try:
        comando = [ejecutable_path, algoritmo, str(izquierda), str(derecha), control]
        app.test_process = subprocess.Popen(comando, cwd=ruta_ce)
        app.test_pid = app.test_process.pid
        root.mainloop()
    except KeyboardInterrupt:
        app.stop()

        
def compilar_algoritmos():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    carpeta_calendarizacion = os.path.join(base_dir, "../calendarizacion")
    carpeta_ce = os.path.join(base_dir, "../CEthreads")
    carpeta_flujo = os.path.join(base_dir, "../control_flujo")
    carpeta_raiz = os.path.join(base_dir, "..")

    print(" Compilando el programa de prueba y algoritmos...")
    try:
        subprocess.run([
            "gcc",
            os.path.join(carpeta_calendarizacion, "RR.c"),
            os.path.join(carpeta_calendarizacion, "SJF.c"),
            os.path.join(carpeta_calendarizacion, "FCFS.c"),
            os.path.join(carpeta_calendarizacion, "c_prioridad.c"),
            os.path.join(carpeta_calendarizacion, "c_tiempo_real.c"),
            os.path.join(carpeta_calendarizacion, "calendarizador.c"),
            os.path.join(carpeta_ce, "CEthreads.c"),
            os.path.join(carpeta_ce, "test.c"),
            os.path.join(carpeta_flujo, "Equidad.c"),
            os.path.join(carpeta_flujo, "Letrero.c"),
            os.path.join(carpeta_flujo, "FIFO.c"),
            os.path.join(carpeta_raiz, "config.c"),
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
