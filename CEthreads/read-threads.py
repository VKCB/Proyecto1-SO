import os
import time

def get_test_pid():
    """Busca el PID del proceso llamado 'test'."""
    for pid in os.listdir("/proc"):
        if pid.isdigit():
            try:
                with open(f"/proc/{pid}/comm") as f:
                    if f.read().strip() == "test":
                        return int(pid)
            except FileNotFoundError:
                continue
    return None

def list_threads(pid):
    """Devuelve una lista de TIDs y nombres de hilos del proceso con PID dado."""
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

def monitor_test_threads():
    seen_threads_global = set()
    all_threads = []

    print("🔄 Iniciando monitor de proceso 'test'...\n")
    try:
        while True:
            pid = get_test_pid()

            if pid is None:
                print("⏳ Esperando a que se inicie el proceso 'test'...")
                time.sleep(2)
                continue

            print(f"✅ Proceso 'test' activo con PID {pid}. Monitoreando hilos...\n")
            seen_threads = set()

            while os.path.exists(f"/proc/{pid}/task"):
                threads = list_threads(pid)
                new_threads = [(tid, name) for tid, name in threads if tid not in seen_threads_global]

                for tid, name in new_threads:
                    try:
                        side, tipo, vel, tiempo = name.split('_')
                        print(f"Hilo detectado - TID: {tid} | Lado: {side} | Tipo: {tipo} | Velocidad: {vel} | Tiempo: {tiempo}")
                        all_threads.append({
                            "tid": tid,
                            "lado": side,
                            "tipo": tipo,
                            "velocidad": int(vel),
                            "tiempo": int(tiempo)
                        })
                    except ValueError:
                        print(f"Hilo erróneo - TID: {tid}, Nombre: {name}")

                    seen_threads.add(tid)
                    seen_threads_global.add(tid)

                time.sleep(1)

            print("Proceso 'test' terminó. Reintentando en 2 segundos...\n")
            time.sleep(2)

    except KeyboardInterrupt:
        print("\n Monitor detenido manualmente.")
        print(f"\n Total de hilos registrados: {len(all_threads)}")
        for thread in all_threads:
            print(thread)

if __name__ == "__main__":
    monitor_test_threads()
