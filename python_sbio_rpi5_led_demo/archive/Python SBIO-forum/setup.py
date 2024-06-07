#!/usr/bin/env python

from distutils.core import setup, Extension

incdirs = '../runtimes/linux-raspberry-armle-opengles_2.0-GST01-obj/include'
libdirs = '../runtimes/linux-raspberry-armle-opengles_2.0-GST01-obj/lib'

def main():
    setup(name="sbio",
          version="1.0.1",
          description="Python interface for Crank Storyboard SBIO C library function to send an event",
          author="Garry Clarkson",
          author_email="gclarkson@cranksoftware.com",
          ext_modules=[Extension(
						"sbio", 
						["sbio_python.c"], 
						include_dirs=[incdirs],
						library_dirs=[libdirs],
						libraries=['greio'],
						define_macros=[('NDEBUG', '1')]
						)
					]
			)

if __name__ == "__main__":
    main()