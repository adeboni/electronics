"""Defines the Wand implementation"""

import pyquaternion
import numpy as np
import threading
import socket
import time

PORT = 5005

class WandServer():
    def __init__(self, impact_callback=None) -> None:
        self.impact_callback = impact_callback
        self.udp_thread_running = False
        self.udp_thread = threading.Thread(target=self._udp_thread, daemon=True)
        self.wands = {}
        self.active_wand = None
        self.watchdog_thread = threading.Thread(target=self._watchdog, daemon=True)
        self.watchdog_thread.start()

    @property
    def connected_wands(self):
        return [w for w in self.wands.values() if w.connected]
        
    def wait_for_wand(self):
        while self.active_wand is None:
            time.sleep(0.5)
            if len(self.connected_wands) > 0:
                self.active_wand = self.connected_wands[0]
                print(f"Active wand is {self.active_wand.address}")

    def start_udp(self) -> None:
        self.udp_thread_running = True
        self.udp_thread.start()
        print("Waiting for wand to connect...")

    def stop_udp(self) -> None:
        if self.udp_thread_running:
            self.udp_thread_running = False
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.connect(('127.0.0.1', PORT))
            sock.sendto(bytes([]), ('127.0.0.1', 5005))
            sock.close()
            self.udp_thread.join()
            
    def _udp_thread(self) -> None:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(('0.0.0.0', PORT))

        while self.udp_thread_running:
            raw_data, addr = sock.recvfrom(4096)
            if len(raw_data) == 0:
                continue

            addr = addr[0]
            if addr not in self.wands:
                print(f"Found new wand at {addr}")
                self.wands[addr] = Wand(addr, self.impact_callback)

            data = np.frombuffer(raw_data, np.int16)
            seq_num_diff = data[0] - self.wands[addr].seq_num
            if seq_num_diff > -20 and seq_num_diff <= 0:
                self.wands[addr].seq_num = data[0]
                continue

            self.wands[addr].update_data(data)
            
    def _watchdog(self) -> None:
        while True:
            time.sleep(1)
            if self.active_wand and not self.active_wand.connected:
                print(f"Disconnected from {self.active_wand.address}")
                self.active_wand = None
                self.wait_for_wand()


class Wand:
    def __init__(self, address=None, impact_callback=None) -> None:
        self.POS_QUEUE_LIMIT = 5
        self.SPEED_THRESHOLD = 5
        self.pos_queue = []
        self.prev_speed = 0
        self.impact_callback = impact_callback
        self.position = pyquaternion.Quaternion()
        self.button = False
        self.address = address
        self.plugged_in = False
        self.charged = False
        self.battery_volts = 0
        self.connected = True
        self.seq_num = 0
        self.last_update = time.time()
        self.watchdog_thread = threading.Thread(target=self._watchdog, daemon=True)
        self.watchdog_thread.start()

    def update_data(self, udp_data) -> None:
        if not self.connected:
            self.connected = True
        self.last_update = time.time()
        self.seq_num = udp_data[0]
        self.plugged_in = udp_data[1] == 1
        self.charged = udp_data[2] == 1
        self.battery_volts = udp_data[3] / 4095 * 3.7
        self.button = udp_data[4] == 1
        self.position = pyquaternion.Quaternion((udp_data[5:9] - 16384) / 16384)
        if self.check_for_impact() and self.impact_callback:
            self.impact_callback(self.position)

    def check_for_impact(self) -> bool:
        tip_pos = self.position.rotate([0, -1, 0])[2]
        if len(self.pos_queue) > self.POS_QUEUE_LIMIT:
            self.pos_queue.pop(0)
        self.pos_queue.append((tip_pos, time.time()))
        d = self.pos_queue[-1][0] - self.pos_queue[0][0]
        t = self.pos_queue[-1][1] - self.pos_queue[0][1]
        if t < 0.01:
            return False
        new_speed = -d / t
        result = self.prev_speed > self.SPEED_THRESHOLD and new_speed < self.SPEED_THRESHOLD
        self.prev_speed = new_speed
        return result
        
    def _watchdog(self) -> None:
        while True:
            time.sleep(1)
            if time.time() > self.last_update + 10:
                self.connected = False
