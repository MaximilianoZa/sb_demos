#!/usr/bin/env python

from distutils.core import setup, Extension

incdirs = '/home/linux/Crank_Software/Storyboard_Engine/8.0-stable.202303220149/linux-x86_64-opengles_2.0-x11-obj/include'
libdirs = '/home/linux/Crank_Software/Storyboard_Engine/8.0-stable.202303220149/linux-x86_64-opengles_2.0-x11-obj/lib'

def main():
    setup(name="sbio",
          version="1.0.0",
          description="Python interface for Crank Storyboard SBIO C library",
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