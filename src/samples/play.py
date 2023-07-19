import zengl
import canvas
import ottosim

ctx = zengl.context(canvas.Loader())
renderer = ottosim.Renderer(ctx, (1280, 720))

env = ottosim.make('Otto-v0')

env.reset()

def update():
    renderer.render(env, (0.0, -0.3, 0.15), (0.0, 0.0, 0.07))
