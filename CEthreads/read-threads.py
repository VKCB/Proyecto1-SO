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
    print("🔍 Buscando proceso 'test'...")
    pid = None
    while pid is None:
        pid = get_test_pid()
        time.sleep(1)

    print(f"✅ Proceso 'test' encontrado con PID {pid}. Monitoreando hilos...\n")

    seen_threads = set()
    try:
        while True:
            # Verifica si /proc/[pid]/task todavía existe
            if not os.path.exists(f"/proc/{pid}/task"):
                print("⚠️ Proceso terminó.")
                break

            threads = list_threads(pid)
            new_threads = [(tid, name) for tid, name in threads if tid not in seen_threads]

            for tid, name in new_threads:
                try:
                    side, tipo, vel = name.split('_')
                    print(f"🧵 Hilo detectado - TID: {tid} | Lado: {side} | Tipo: {tipo} | Velocidad: {vel}")
                except ValueError:
                    print(f"🧵 Hilo err detectado - TID: {tid}, Nombre: {name}")
                seen_threads.add(tid)

            time.sleep(1)
    except KeyboardInterrupt:
        print("\n🛑 Monitor detenido.")


if __name__ == "__main__":
    monitor_test_threads()
