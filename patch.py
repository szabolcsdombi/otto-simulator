import os

index = open('/app/dist/index.html').read()
script = '<script>' + open('/app/dist/home.js').read() + '</script>'
index = index.replace('<script src="/otto-simulator/home.js"></script>', script)
open('/app/dist/index.html', 'w').write(index)
os.unlink('/app/dist/home.js')
