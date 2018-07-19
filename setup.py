from distutils.core import setup, Extension

sundtek = Extension(
    'sundtek',
    sources=['sundtek.c'],
    extra_link_args=['-L/opt/lib', '-lmcsimple', '-lmediaclient'],
    extra_compile_args=['-Wall'],
    )

setup(
    name='sundtek',
    version='1.0',
    description='Python Package which wraps some mediasrv api functions',
    ext_modules=[sundtek],
    url='http://adamlamers.com',
    author='Alexander Grothe',
    author_email='alexander.grothe at gmail dot com')
