from distutils.core import setup, Extension

sundtek = Extension(
    'sundtek_api',
    sources=['sundtek_api.c'],
    extra_link_args=['-L/opt/lib', '-lmcsimple'],
    extra_compile_args=['-Wall'],
    )

setup(
    name='sundtek_api',
    version='1.0',
    description='Python Package which wraps some mediasrv api functions',
    py_modules=['sundtekdevice'],
    ext_modules=[sundtek],
    url='https://github.com/seahawk1986/python3_sundtek',
    author='Alexander Grothe',
    author_email='alexander.grothe at gmail dot com')
