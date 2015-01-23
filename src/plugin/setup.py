# DDS Python Extension Generator
# Usage: $ python setup.py build
# Author: Philip Blair (belph)
# Copyright (C) 2015 Northeastern University CCIS Crew

from distutils.core import setup, Extension

dds_mod = Extension('dds', sources = ['dds_python_module.c', '../dict.c', '../dds_globals.c', 'dds_python_helpers.c'])

setup (name = 'DDS Plugin API',
            version = '0.1',
            description = 'API for DDS plugin extensions',
            ext_modules = [dds_mod])
