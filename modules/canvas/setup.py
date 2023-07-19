from setuptools import Extension, setup

ext = Extension(
    name='canvas',
    sources=['./canvas.c'],
    define_macros=[('PY_SSIZE_T_CLEAN', None)],
)

setup(
    name='canvas',
    version='0.1.0',
    ext_modules=[ext],
)
