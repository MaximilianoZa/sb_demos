#!/usr/bin/env python

from distutils.core import Extension, setup

incdirs = "/home/pi/crank/runtimes/linux-raspberry-aarch64-opengles_2.0-drm-obj/include"
libdirs = "/home/pi/crank/runtimes/linux-raspberry-aarch64-opengles_2.0-drm-obj/lib"


def main():
    setup(
        name="sbio",
        version="1.0.2",
        description="Python interface for Crank Storyboard SBIO C library function to send an event",
        author="Maximiliano Zapata",
        author_email="maximiliano.zapata@ametek.com",
        ext_modules=[
            Extension(
                "sbio",
                ["sbio_python.c"],
                include_dirs=[incdirs],
                library_dirs=[libdirs],
                libraries=["greio"],
                define_macros=[("NDEBUG", "1")],
            )
        ],
    )


if __name__ == "__main__":
    main()
