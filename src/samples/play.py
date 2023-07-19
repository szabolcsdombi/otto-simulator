import math

import canvas
import ottosim
import zengl


class g:
    t = 0.0


def balance(t):
    return [
        0.0,
        -min(t, 0.8) + math.sin(t * 4.2) * 0.2 * min(max(t - 0.5, 0.0), 1.0),
        0.0,
        min(t, 0.8) + math.sin(t * 4.2) * 0.2 * min(max(t - 0.5, 0.0), 1.0),
    ]


ctx = zengl.context(canvas.Loader())
renderer = ottosim.Renderer(ctx, (1280, 720))

env = ottosim.make('Otto-v0')

env.reset()


def update():
    for _ in range(5):
        g.t += 1.0 / 300.0
        action = balance(g.t)
        env.step(action)

    renderer.render(env, (0.0, -0.3, 0.15), (0.0, 0.0, 0.07))
