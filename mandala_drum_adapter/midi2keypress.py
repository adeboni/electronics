import mido
import pydirectinput
import time
import threading

pydirectinput.FAILSAFE = False
pydirectinput.PAUSE = 0.02
input_port = mido.open_input()

note_to_key = {
    48: 'j',
    50: 'j',
    52: 'j',
    53: 'j',
    60: 'k',
    62: 'k',
    64: 'k',
    65: 'k',
    67: 'k',
    69: 'd',
    71: 'd',
    72: 'd',
    84: 'd'
}

print("Running")

for message in input_port:
    if message.type == 'note_on' and message.velocity > 0:
        #print(message.note, message.velocity)
        note = message.note
        if note in note_to_key:
            threading.Thread(target=lambda: pydirectinput.press(note_to_key[note])).start()
            