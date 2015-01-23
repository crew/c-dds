# DDS Python Extension Generator
# Usage: $ python setup.py build
# Author: Philip Blair (belph)
# Copyright (C) 2015 Northeastern University CCIS Crew

from distutils.core import setup, Extension

dds_mod = Extension('dds', sources = ['dds_python_module.c', 'dds_python_helpers.c'],
                                     runtime_library_dirs=['../build'],library_dirs=['../../build'],libraries=['dds'])
                                     #extra_compile_args=['-L../../build -ldds'])

setup (name = 'DDS Plugin API',
            version = '0.1',
            description = 'API for DDS plugin extensions',
            ext_modules = [dds_mod])
