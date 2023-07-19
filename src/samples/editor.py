import math

import canvas
import ottosim
import zengl

renderer = ottosim.Renderer(zengl.context(canvas.Loader()), canvas.window.size)

env = ottosim.make('Otto-v0')
env.reset()


def update():
    for _ in range(5):
        action = [
            0.0, -min(env.time, 0.8) + math.sin(env.time * 4.2) * 0.2,
            0.0, min(env.time, 0.8) + math.sin(env.time * 4.2) * 0.2,
        ]
        env.step(action)

    renderer.render(env, (0.0, -0.3, 0.15), (0.0, 0.0, 0.07), canvas.window.aspect)
