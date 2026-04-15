import time
import psutil
import serial
import pyautogui
import threading

from ctypes import POINTER, cast
from comtypes import CLSCTX_ALL
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume

ser = serial.Serial("COM5", 9600, timeout=1)

devices = AudioUtilities.GetSpeakers()
interface = devices.Activate(
    IAudioEndpointVolume._iid_, CLSCTX_ALL, None
)
volume_interface = cast(interface, POINTER(IAudioEndpointVolume))


def get_volume():
    return round(volume_interface.GetMasterVolumeLevelScalar() * 100)


def volume_up():
    pyautogui.press("volumeup")


def volume_down():
    pyautogui.press("volumedown")


def send_stats():
    while True:
        cpu = int(psutil.cpu_percent(interval=0.3))
        ram = int(psutil.virtual_memory().percent)
        vol = get_volume()

        try:
            ser.write(f"CPU:{cpu}\n".encode())
            ser.write(f"RAM:{ram}\n".encode())
            ser.write(f"VOL:{vol}\n".encode())
        except:
            pass

        time.sleep(0.1)


def listen():
    while True:
        try:
            cmd = ser.readline().decode().strip()
        except:
            continue

        if not cmd:
            continue

        if cmd == "VOLUP":
            volume_up()

        elif cmd == "VOLDOWN":
            volume_down()

        elif cmd == "PLAY":
            pyautogui.press("playpause")


t1 = threading.Thread(target=send_stats, daemon=True)
t2 = threading.Thread(target=listen, daemon=True)

t1.start()
t2.start()

while True:
    time.sleep(1)
