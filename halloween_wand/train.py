import wand
import json
import time

ws = wand.WandServer()
ws.start_udp()
ws.wait_for_wand()
print("Waiting 10 seconds for wand to stabilize")
time.sleep(10)

data = dict()
index = 1
while True:
    val = input("Point the wand at the target location and press Enter to continue or press q + Enter to quit.\n").strip()
    if val == 'q':
        break
    data[index] = ws.active_wand.position.rotate([0, -1, 0])
    print(f"Saved position {data[index]} to position {index}")
    index += 1

with open("points.json", "w") as outfile: 
    json.dump(data, outfile)
