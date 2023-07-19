from setuptools import Extension, setup

ext = Extension(
    name='ottosim.ottosim',
    sources=[
        'ottosim/ottosim.cpp',
        'bullet/btBulletAll.cpp',
    ],
    define_macros=[
        ('PY_SSIZE_T_CLEAN', None),
        ('BT_USE_DOUBLE_PRECISION', None),
    ],
    include_dirs=['bullet'],
)

setup(
    name='ottosim',
    version='0.1.0',
    packages=['ottosim'],
    package_data={
        'ottosim': [
            'assets/otto.mesh.bin',
        ],
    },
    include_package_data=True,
    ext_modules=[ext],
)
