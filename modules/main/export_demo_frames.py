import motion
import ottosim

f = open('otto.frames.bin', 'wb')

env = ottosim.make()
env.reset()
t = 0.0
for i in range(180):
    for _ in range(5):
        t += 1.0 / 300.0
        env.step(motion.balance(t))
    f.write(env.bones())

env.reset()
t = 0.0
for i in range(180):
    for _ in range(5):
        t += 1.0 / 300.0
        env.step(motion.walk(t))
    f.write(env.bones())

env.reset()
t = 0.0
for i in range(180):
    for _ in range(5):
        t += 1.0 / 300.0
        env.step(motion.dance(t))
    f.write(env.bones())

env.reset()
t = 0.0
for i in range(180):
    for _ in range(5):
        t += 1.0 / 300.0
        env.step(motion.turn_left(t))
    f.write(env.bones())

env.reset()
t = 0.0
for i in range(180):
    for _ in range(5):
        t += 1.0 / 300.0
        env.step(motion.turn_right(t))
    f.write(env.bones())

env.reset()
t = 0.0
for i in range(180):
    for _ in range(5):
        t += 1.0 / 300.0
        env.step(motion.jump(t))
    f.write(env.bones())
