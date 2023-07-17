from setuptools import Extension, setup

ext = Extension(
    name='window',
    sources=['./window.cpp'],
    define_macros=[('PY_SSIZE_T_CLEAN', None)],
    libraries=['gdi32', 'user32', 'opengl32', 'dwmapi', 'powrprof'],
)

setup(
    name='window',
    version='0.1.0',
    ext_modules=[ext],
)
