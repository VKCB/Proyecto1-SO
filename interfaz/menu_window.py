import tkinter as tk
from tkinter import ttk

class MenuWindow:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Traffic Control Configuration")
        self.root.geometry("400x300")
        
        # Variables to store selections
        self.scheduling = tk.StringVar()
        self.control = tk.StringVar()
        
        # Scheduling algorithm selection
        ttk.Label(self.root, text="Select scheduling:").pack(pady=10)
        scheduling_combo = ttk.Combobox(self.root, textvariable=self.scheduling)
        scheduling_combo['values'] = ('SJF', 'FCFS', 'RR', 'Prioridad', 'Tiempo Real')
        scheduling_combo.current(0)
        scheduling_combo.pack(pady=5)
        
        # Control algorithm selection
        ttk.Label(self.root, text="Select control algorithm:").pack(pady=10)
        control_combo = ttk.Combobox(self.root, textvariable=self.control)
        control_combo['values'] = ('Letrero', 'Equidad', 'FIFO')
        control_combo.current(0)
        control_combo.pack(pady=5)
        
        # Start button
        ttk.Button(self.root, text="Start Simulation", command=self.start_simulation).pack(pady=20)
        
        self.result = None
        
    def start_simulation(self):
        self.result = {
            'scheduling': self.scheduling.get(),
            'control': self.control.get()
        }
        self.root.quit()
        self.root.destroy()
        
    def show(self):
        self.root.mainloop()
        return self.result