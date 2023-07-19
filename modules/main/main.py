from itertools import cycle

import motion
import ottosim
import zengl
from window import window

renderer = ottosim.Renderer(zengl.context(), window.size)
animations = cycle([
    motion.balance,
    motion.walk,
    motion.dance,
    motion.turn_left,
    motion.turn_right,
    motion.jump,
])


class g:
    env = ottosim.make('OttoLowFriction-v0')
    animation = None
    t = None


def render():
    if g.t is None or g.t > 3.0:
        g.env.reset()
        g.t = 0.0
        g.animation = next(animations)

    for _ in range(5):
        g.t += 1.0 / 300.0
        action = g.animation(g.t)
        g.env.step(action)

    renderer.render(g.env, (0.0, -0.3, 0.15), (0.0, 0.0, 0.07))


window.on_frame = render
window.run()
