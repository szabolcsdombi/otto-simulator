import math

import canvas
import ottosim
import zengl


class g:
    t = 0.0
    blending = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    prev_action = [0.0, 0.0, 0.0, 0.0]
    prev_reset = False


def jump(t, b):
    return [0.0, math.cos(t * math.pi * 3.0) * 0.5 * b, 0.0, -math.cos(t * math.pi * 3.0) * 0.5 * b]


def dance(t, b):
    return [0.0, math.sin(t * math.pi * 3.0) * 0.5 * b, 0.0, math.sin(t * math.pi * 3.0) * 0.5 * b]


def balance(t, b):
    return [0.0, -0.8 * b + math.sin(t * 4.2) * 0.2 * b, 0.0, 0.8 * b + math.sin(t * 4.2) * 0.2 * b]


def turn_left(t, b):
    return [math.sin(t * math.pi * 3.0) * 0.5 * b, math.cos(t * math.pi * 3.0) * 0.25 * b, 0.0, 0.0]


def turn_right(t, b):
    return [0.0, 0.0, math.sin(t * math.pi * 3.0) * 0.5 * b, math.cos(t * math.pi * 3.0) * 0.25 * b]


def walk(t, b):
    return [
        math.sin(t * math.pi * 3.0) * 0.5 * b,
        math.cos(t * math.pi * 3.0) * 0.5 * b,
        -math.sin(t * math.pi * 3.0) * 0.5 * b,
        -math.cos(t * math.pi * 3.0) * 0.5 * b,
    ]


def blend(a, b):
    a[0] += b[0]
    a[1] += b[1]
    a[2] += b[2]
    a[3] += b[3]


ctx = zengl.context(canvas.Loader())
renderer = ottosim.Renderer(ctx, canvas.window.size)

env = ottosim.make('Otto-v0')

env.reset()


def update():
    gamepad = canvas.window.gamepad()
    keyboard = canvas.window.keyboard()
    reset = gamepad['reset'] or keyboard['reset']
    if reset and not g.prev_reset:
        env.reset()
        g.time = 0.0
        g.prev_action = [0.0, 0.0, 0.0, 0.0]
    g.prev_reset = reset

    for i, key in enumerate('123456'):
        g.blending[i] = min(max(g.blending[i] + (0.1 if keyboard[key] else -0.1), 0.0), 1.0)

    for k in range(5):
        g.t += 1.0 / 300.0
        lx, ly = gamepad['left']
        rx, ry = gamepad['right']

        c = (k + 1) / 5.0

        total = sum(g.blending)
        b = 1.0 - min(total, 1.0)
        new_action = [rx * 0.8 * b, -ry * 0.8 * b, lx * 0.8 * b, ly * 0.8 * b]
        blend(new_action, jump(g.t, g.blending[0]))
        blend(new_action, dance(g.t, g.blending[1]))
        blend(new_action, balance(g.t, g.blending[2]))
        blend(new_action, turn_left(g.t, g.blending[3]))
        blend(new_action, turn_right(g.t, g.blending[4]))
        blend(new_action, walk(g.t, g.blending[5]))
        if total > 1.0:
            new_action[0] /= total
            new_action[1] /= total
            new_action[2] /= total
            new_action[3] /= total
        action = [u * c + v * (1.0 - c) for u, v in zip(new_action, g.prev_action)]
        g.prev_action = new_action
        env.step(action)

    renderer.render(env, (0.0, -0.3, 0.15), (0.0, 0.0, 0.07), canvas.window.aspect)
