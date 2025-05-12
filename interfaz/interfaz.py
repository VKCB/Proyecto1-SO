import os
import time
import threading
import tkinter as tk
import subprocess  # Importar subprocess para ejecutar el programa 'test'

# === Clase principal de la interfaz ===

class CalleApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Calle de 1 carril - Monitor de carros")

        # Configuración del lienzo
        self.canvas = tk.Canvas(root, width=600, height=150, bg="white")
        self.canvas.pack()

        # Dibujar la calle
        self.canvas.create_rectangle(0, 50, 600, 100, fill="gray", outline="gray")

        self.carros = {}
        self.lock = threading.Lock()
        self.running = True

        # Iniciar el programa 'test'
        self.test_process = subprocess.Popen(
            ["./test"],
            cwd="/home/vale/Escritorio/ProyectoSO/Proyecto1-SO/CEthreads"
        )
        self.test_pid = self.test_process.pid

        # Iniciar el bucle de actualización
        threading.Thread(target=self.update_loop, daemon=True).start()
        self.animate()

    def update_loop(self):
        while self.running:
            threads = self.list_threads()
            for tid, name in threads:
                print(f"🔍 Hilo detectado: TID={tid}, Nombre={name}")
                if tid not in self.carros:
                    try:
                        lado, tipo, velocidad = name.split('_')
                        velocidad = int(velocidad)
                        print(f"🧵 Hilo válido - TID: {tid}, Lado: {lado}, Tipo: {tipo}, Velocidad: {velocidad}")
                        x = 0 if lado == "IZQ" else 600
                        y = 70
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
                if carro["x"] > 640 or carro["x"] < -50:
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

# === Ejecutar interfaz ===

if __name__ == "__main__":
    root = tk.Tk()
    app = CalleApp(root)
    try:
        root.mainloop()
    except KeyboardInterrupt:
        app.stop()
