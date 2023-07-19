import math

import canvas
import ottosim
import zengl


class g:
    t = 0.0
    prev_action = [0.0, 0.0, 0.0, 0.0]
    prev_reset = False


def balance(t):
    return [
        0.0,
        -min(t, 0.8) + math.sin(t * 4.2) * 0.2 * min(max(t - 0.5, 0.0), 1.0),
        0.0,
        min(t, 0.8) + math.sin(t * 4.2) * 0.2 * min(max(t - 0.5, 0.0), 1.0),
    ]


ctx = zengl.context(canvas.Loader())
renderer = ottosim.Renderer(ctx, canvas.window.size)

env = ottosim.make('Otto-v0')

env.reset()



def update():
    gamepad = canvas.window.gamepad()
    reset = gamepad['reset']
    if gamepad['reset'] and not g.prev_reset:
        env.reset()
        g.time = 0.0
        g.prev_action = [0.0, 0.0, 0.0, 0.0]
    g.prev_reset = reset

    for k in range(5):
        g.t += 1.0 / 300.0
        lx, ly = gamepad['left']
        rx, ry = gamepad['right']

        c = (k + 1) / 5.0
        new_action = [rx * 0.8, -ry * 0.8, lx * 0.8, ly * 0.8]
        action = [u * c + v * (1.0 - c) for u, v in zip(new_action, g.prev_action)]
        g.prev_action = new_action
        env.step(action)

    renderer.render(env, (0.0, -0.3, 0.15), (0.0, 0.0, 0.07), canvas.window.aspect)
