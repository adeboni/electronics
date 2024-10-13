"""This module visualizes wand positions in 3D"""

import numpy as np
from matplotlib import pyplot as plt
from matplotlib import animation
import wand
import os
import json

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.set_xlim((-2, 2))
ax.set_ylim((-2, 2))
ax.set_zlim((-2, 2))
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.view_init(20, 50)

data = None
if os.path.isfile("points.json"):
    with open("points.json") as f:
        data = json.load(f)
    for v in data.values():
        ax.plot([v[0]], [v[1]], [v[2]], c='k', linestyle='', marker='o', alpha=0.2)
    
lines = sum([ax.plot([], [], [], c=c) for c in ['r', 'g', 'b']], [])
endpoints = np.array([[-0.25, 0, 0], [0, -1, 0], [0, 0, -0.25]])
 
def dist(point1, point2):
    return np.arccos(np.clip(np.dot(point1, point2), -1.0, 1.0))
    
def impact_callback(q):
    pos = q.rotate([0, -1, 0])
    print(f'Hit at position {pos}')
    if data:
        dists = sorted([(i, dist(pos, v)) for i, v in data.items()], key=lambda x: x[1])
        t, d = dists[0]
        print(f"Closest target was {t} with distance {d}")

ws = wand.WandServer(impact_callback)
ws.start_udp()
ws.wait_for_wand()

def animate(_):
    if not ws.active_wand:
        return
    q = ws.active_wand.position
    for line, end in zip(lines, endpoints):
        v = q.rotate(end)
        line.set_data([0, v[0]], [0, v[1]])
        line.set_3d_properties([0, v[2]])

ani = animation.FuncAnimation(fig, animate, interval=25, cache_frame_data=False)
plt.show()
