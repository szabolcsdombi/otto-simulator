import os

routes = [
    'play',
    'editor',
]

index = open('/app/dist/index.html', 'rb').read()
open('/app/dist/404.html', 'wb').write(index)

for route in routes:
    filename = os.path.join('/app/dist/', route, 'index.html')
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    open(filename, 'wb').write(index)
