import numpy as np
import wand
import os
import json
import subprocess
import time
import threading
import pydub
from pydub.playback import play
import random
from pythonosc import udp_client
from pynput import keyboard

data = None
if os.path.isfile("points.json"):
    with open("points.json") as f:
        data = json.load(f)

osc_client = udp_client.SimpleUDPClient("192.168.0.101", 8000)

# To add new audio banks, copy the line below and change the "target_1_audio" folder name to your other folder.
# Then change the bank variable name in the sound_at_targets dictionary
audio_bank_1 = [pydub.AudioSegment.from_wav(f"target_1_audio/{file}") for file in os.listdir("target_1_audio")]
audio_bank_thunder = [pydub.AudioSegment.from_wav(f"thunder/{file}") for file in os.listdir("thunder")]
sound_at_targets = {
  1: [sound.pan(-1) for sound in audio_bank_1],
  2: [sound.pan(-0.66) for sound in audio_bank_1],
  3: [sound.pan(-0.33) for sound in audio_bank_1],
#  4: [sound.pan(-0.1) for sound in audio_bank_thunder],
  4: [sound.pan(-0.1) - 6 for sound in audio_bank_thunder],
  5: [sound.pan(+0.1) for sound in audio_bank_1],
  6: [sound.pan(+0.33) for sound in audio_bank_1],
  7: [sound.pan(+0.66) for sound in audio_bank_1],
  8: [sound.pan(+1) for sound in audio_bank_1],
  9: [sound.pan(0) for sound in audio_bank_1]
}
# Pan values range from -1 (100% left) to +1 (100% right). -0.5 would be 50% left

def dist(point1, point2):
    return np.arccos(np.clip(np.dot(point1, point2), -1.0, 1.0))

def play_audio(a):
    play(a)


def turn_on_relay(relay_num):
    osc_client.send_message("/wand", int(relay_num))
    if relay_num in sound_at_targets:
        threading.Thread(target=play_audio, args=(random.choice(sound_at_targets[relay_num]), )).start()
        
    os.system(f"sudo usbrelay 6QMBS_{relay_num}=1")
    time.sleep(2)
    os.system(f"sudo usbrelay 6QMBS_{relay_num}=0")
    
    
# This is an array of targets that will flash instead of just be on for 2 seconds
flashing_targets = [4]
def flash_relay(relay_num):
    osc_client.send_message("/wand", int(relay_num))
    if relay_num in sound_at_targets:
        threading.Thread(target=play_audio, args=(random.choice(sound_at_targets[relay_num]), )).start()
        
    for _ in range(random.choice([2, 3, 4, 5])):
        os.system(f"sudo usbrelay 6QMBS_{relay_num}=1")
        time.sleep(random.uniform(0.03, 0.09))
        os.system(f"sudo usbrelay 6QMBS_{relay_num}=0")
        time.sleep(random.uniform(0.03, 0.09))

    
def on_press(key):
    try:
        if key.char == 'a':
            threading.Thread(target=flash_relay, args=(4,)).start()
        elif key.char == 'b':
            for i in [1, 2, 3, 4, 5, 6, 7, 8]:
                relay_target = flash_relay if i in flashing_targets else turn_on_relay
                threading.Thread(target=relay_target, args=(i,)).start()
    except:
        pass

def start_keyboard_listener():
    with keyboard.Listener(on_press=on_press) as listener:
        listener.join()
threading.Thread(target=start_keyboard_listener).start()

def staff_callback():
    # This is an array of relays to trigger for the staff impact. For example if you want to trigger relays 1, 2, and 3, change [4] to [1, 2, 3] 
    for t in [4]:
        relay_target = flash_relay if t in flashing_targets else turn_on_relay
        threading.Thread(target=relay_target, args=(t,)).start()

def impact_callback(q):
    pos = q.rotate([0, -1, 0])
    print(f'Hit at position {pos}')
    if data:
        dists = sorted([(i, dist(pos, v)) for i, v in data.items()], key=lambda x: x[1])
        t, d = dists[0]
        if d < 1:
            print(f"Closest target was {t} with distance {d}")
            t = int(t)
            relay_target = flash_relay if t in flashing_targets else turn_on_relay
            threading.Thread(target=relay_target, args=(t,)).start()

ws = wand.WandServer(impact_callback, staff_callback)
ws.start_udp()
ws.wait_for_wand()

try:
    while True:
        pass
except KeyboardInterrupt:
    quit()