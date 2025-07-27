import serial
import threading
import queue
import time
import numpy as np
from collections import deque
import tkinter as tk
from tkinter import ttk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.gridspec import GridSpec

# === SERIAL CONFIGURATION ===
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 250000

telemetry_queue = queue.Queue()
MAX_SAMPLES = 250

timestamps = deque(maxlen=MAX_SAMPLES)
control_setpoint = deque(maxlen=MAX_SAMPLES)
sense_control_point = deque(maxlen=MAX_SAMPLES)
sense_input_voltage = deque(maxlen=MAX_SAMPLES)
sense_output_voltage = deque(maxlen=MAX_SAMPLES)
pwm_duty = deque(maxlen=MAX_SAMPLES)
pwm_effective_duty = deque(maxlen=MAX_SAMPLES)

serial_lock = threading.Lock()
serial_port = serial.Serial(SERIAL_PORT, BAUD_RATE)

def read_serial():
    print("Reading serial with readline (ASCII mode)")
    while True:
        try:
            with serial_lock:
                line = serial_port.readline().decode('utf-8').strip()
            if not line:
                continue
            fields = line.split(',')

            data = {}
            for field in fields:
                if ':' not in field:
                    continue
                key, value = field.strip().split(':', 1)
                key = key.strip()
                value = value.strip()
                if key == 'VI':
                    data['sense_input_voltage'] = float(value)
                elif key == 'II':
                    data['sense_input_current'] = float(value)
                elif key == 'VO':
                    data['sense_output_voltage'] = float(value)
                elif key == 'CP':
                    data['sense_control_point'] = float(value)
                elif key == 'SP':
                    data['control_setpoint'] = float(value)
                elif key == 'ER':
                    data['control_error'] = float(value)
                elif key == 'D':
                    data['pwm_duty'] = float(value)
                elif key == 'ED':
                    data['pwm_effective_duty'] = float(value)
                elif key == 'ST':
                    data['status_connected'] = int(value)

            required_keys = {
                'sense_input_voltage',
                'sense_input_current',
                'sense_output_voltage',
                'sense_control_point',
                'control_setpoint',
                'control_error',
                'pwm_duty',
                'pwm_effective_duty',
                'status_connected'
            }

            if required_keys.issubset(data.keys()):
                telemetry_queue.put(data)

        except Exception as e:
            print("Serial read error:", e)
            continue

def start_telemetry_thread():
    t = threading.Thread(target=read_serial, daemon=True)
    t.start()

# === TKINTER UI ===
def create_control_panel(parent):
    frame = ttk.Frame(parent, padding=10)
    ttk.Label(frame, text="Set PID Gains").grid(row=0, column=0, columnspan=2)

    def send_command(cmd, val):
        message = f"<{cmd}:{val}>\n".encode('utf-8')
        with serial_lock:
            serial_port.write(message)

    def make_entry_command(label, cmd, row):
        ttk.Label(frame, text=label).grid(row=row, column=0, sticky="w")
        entry = ttk.Entry(frame, width=10)
        entry.grid(row=row, column=1)
        ttk.Button(frame, text="Send", command=lambda: send_command(cmd, entry.get())).grid(row=row, column=2)

    make_entry_command("P gain", "p", 1)
    make_entry_command("I gain", "i", 2)
    make_entry_command("D gain", "d", 3)

    # Generic command entry
    ttk.Label(frame, text="Custom <C:val>").grid(row=4, column=0, columnspan=2)
    custom_entry = ttk.Entry(frame, width=15)
    custom_entry.grid(row=5, column=0, columnspan=2)
    ttk.Button(frame, text="Send", command=lambda: send_command(*custom_entry.get().strip('<>').split(':'))).grid(row=5, column=2)

    # Reset MCU
    ttk.Button(frame, text="Reset MCU", command=lambda: send_command('R', 0)).grid(row=6, column=0, columnspan=3, pady=10)

    return frame

# === MATPLOTLIB SETUP ===
fig = plt.Figure(figsize=(10, 6))
gs = GridSpec(3, 2, figure=fig, width_ratios=[3, 1])

# Time plot: control
ax_cp = fig.add_subplot(gs[0, 0])
ax_cp.set_ylabel("Control (±1)")
ax_cp.set_ylim(-1.1, 1.1)
line_setpoint, = ax_cp.plot([0], [0], label="Setpoint")
line_cp, = ax_cp.plot([0], [0], label="Control Point")
ax_cp.legend(loc="upper right")
ax_cp.grid(True)

# Polar plot
ax_polar = fig.add_subplot(gs[0, 1], polar=True)
cp_line, = ax_polar.plot([], [], color='tab:red', label="Control Point")
sp_line, = ax_polar.plot([], [], color='tab:blue', label="Setpoint")
ax_polar.set_ylim(0, 1)
ax_polar.set_yticklabels([])
ax_polar.set_title("Polar Control")
ax_polar.legend(loc="lower center", bbox_to_anchor=(0.5, -0.2))
ax_polar.set_theta_zero_location("N")
ax_polar.set_theta_direction(-1)

# Voltage
ax_v = fig.add_subplot(gs[1, :])
ax_v.set_ylabel("Voltage (V)")
line_vin, = ax_v.plot([0], [0], label="Vin")
line_vout, = ax_v.plot([0], [0], label="Vout")
ax_v.legend(loc="upper right")
ax_v.grid(True)

# PWM Duty Plot (instead of Current)
ax_d = fig.add_subplot(gs[2, :])
ax_d.set_ylabel("Duty")
ax_d.set_xlabel("Time (s)")
line_duty, = ax_d.plot([0], [0], label="Duty")
line_eff_duty, = ax_d.plot([0], [0], label="Eff Duty")
ax_d.legend(loc="upper right")
ax_d.grid(True)

def update_plot(frame):
    while not telemetry_queue.empty():
        data = telemetry_queue.get()
        now = time.time()
        timestamps.append(now - start_time)
        control_setpoint.append(data['control_setpoint'])
        sense_control_point.append(data['sense_control_point'])
        sense_input_voltage.append(data['sense_input_voltage'])
        sense_output_voltage.append(data['sense_output_voltage'])
        pwm_duty.append(data['pwm_duty'])
        pwm_effective_duty.append(data['pwm_effective_duty'])

    if not timestamps:
        return

    line_setpoint.set_data(timestamps, control_setpoint)
    line_cp.set_data(timestamps, sense_control_point)
    line_vin.set_data(timestamps, sense_input_voltage)
    line_vout.set_data(timestamps, sense_output_voltage)
    line_duty.set_data(timestamps, pwm_duty)
    line_eff_duty.set_data(timestamps, pwm_effective_duty)

    ax_cp.set_xlim(max(0, timestamps[0]), timestamps[-1])
    for ax in [ax_v, ax_d]:
        ax.relim()
        ax.autoscale_view()

    def val_to_angle(val):
        return (val + 1) * np.pi

    angle_cp = val_to_angle(np.clip(sense_control_point[-1], -1, 1))
    angle_sp = val_to_angle(np.clip(control_setpoint[-1], -1, 1))

    cp_line.set_data([angle_cp, angle_cp], [0, 1])
    sp_line.set_data([angle_sp, angle_sp], [0, 1])

    return line_setpoint, line_cp, line_vin, line_vout, line_duty, line_eff_duty, cp_line, sp_line

# === MAIN APPLICATION ===
if __name__ == '__main__':
    start_time = time.time()
    start_telemetry_thread()

    root = tk.Tk()
    root.title("Telemetry & Control")

    plot_frame = ttk.Frame(root)
    plot_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

    canvas = FigureCanvasTkAgg(fig, master=plot_frame)
    canvas.draw()
    canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    control_panel = create_control_panel(root)
    control_panel.pack(side=tk.RIGHT, fill=tk.Y)

    ani = animation.FuncAnimation(fig, update_plot, interval=25, blit=False)
    root.mainloop()
