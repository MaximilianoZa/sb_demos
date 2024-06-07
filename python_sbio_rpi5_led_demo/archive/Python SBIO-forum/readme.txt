=====================================================================
== A basic Python extension which wraps the Storyboard IO library.
=====================================================================

The objective is to make it simple to interact with a Storyboard application using SBIO for test or prototype purposes.

It was inspired by this article:
https://realpython.com/build-python-c-extension-module/

**********************************
** Overview:
**********************************
It is a work in progress.. this is unsupported and untested.

The code has been built and run with Storyboard 6.2 release and Python 3.7 on arm using RaspberryPi4

This is built for use with Python3 and should be built on-target and assumes a Python 3.7 install is already in-place on the platform

**********************************
** Resolved issues
**********************************
1) String data payloads containing space ASCII characters were previously truncated in the returned disctionary object processing in receiveevent()
2) Removed superfluous print statements to remove trace noise

**********************************
** Known issues
**********************************
1) The received events are packaged and returned in a Python dictionary object but the data value types are all currently string regardless of format string.  This work to cast to correct object type depending on the SBIO format string is to be done
2) The SBIO gre_io_receive() call behind the wrapper function receiveevent() is blocking and cannot easily be terminated which results in a segmentation fault if terminated ungracefully. The current approach is to send a QUIT event with no payload to ther receive channel
3) The addeventlistener() API call is not complete or tested
4) Only one receive channel can be used at any time. Once receiveevent() has been called the receive channel is cached and must be closed using closereceivechannel() when complete or before opening a new receive channel
5) It is not fully tested or supported
6) No testing using TCP channels has been attepted
7) Additional work is needed to focus on the error handling on several functions 


**********************************
** Package contents:
**********************************
	Makefile
	setup.py
	sbio_python.c
	readme.txt
	send_event.py
	receive_event_simple.py

**********************************
** How to build:
**********************************
The setup.py file has paths to the target Storyboard runtime and may need to be adjusted depending on source location in the target filesystem namely:
incdirs = '../runtimes/linux-raspberry-armle-opengles_2.0-GST01-obj/include'
libdirs = '../runtimes/linux-raspberry-armle-opengles_2.0-GST01-obj/lib'

To build and install the Python 3.7 extension say on a raspberry pi using the gcc compiler use the following :

pi@raspberrypi:~/Crank/sbpython $ sudo python3 setup.py install
running install
running build
running build_ext
building 'sbio' extension
arm-linux-gnueabihf-gcc -pthread -DNDEBUG -g -fwrapv -O2 -Wall -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -fPIC -DNDEBUG=1 -I../runtimes/linux-raspberry-armle-opengles_2.0-GST01-obj/include -I/usr/include/python3.7m -c sbio_python.c -o build/temp.linux-armv7l-3.7/sbio_python.o
arm-linux-gnueabihf-gcc -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,relro -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 build/temp.linux-armv7l-3.7/sbio_python.o -L../runtimes/linux-raspberry-armle-opengles_2.0-GST01-obj/lib -lgreio -o build/lib.linux-armv7l-3.7/sbio.cpython-37m-arm-linux-gnueabihf.so
running install_lib
copying build/lib.linux-armv7l-3.7/sbio.cpython-37m-arm-linux-gnueabihf.so -> /usr/local/lib/python3.7/dist-packages
running install_egg_info
Removing /usr/local/lib/python3.7/dist-packages/sbio-1.0.1.egg-info
Writing /usr/local/lib/python3.7/dist-packages/sbio-1.0.1.egg-info

**********************************
** To use within python3...
**********************************
import sbio

in_channel="channel_in"
out_channel="channel_out"

sbio.iogen("sb", "-", "gre.quit")
sbio.iogen("sb","-","gre.press", "4u1:button", 0, "4u1:timestamp", 0, "2u1:subtype", 0, "2s1:x", 100, "2s1:y", 150, "2s1:z", 0, "2s1:id", 0, "2s1:spare", 0)

quit_event={
	"gre.quit": {}
	}
sbio.sendevent(out_channel, quit_event)

press_event={
	"gre.press": {
		"4u1:button": 0,
		"4u1:timestamp": 0,
		"2u1:subtype": 0,
		"2s1:x": 100, 
		"2s1:y": 150,
		"2s1:z": 0, 
		"2s1:id": 0, 
		"2s1:spare":0
		}
	}
sbio.sendevent(out_channel, press_event)

myevent=sbio.receiveevent(in_channel)

**********************************
** Examples and API list
**********************************

There are some example usages in the python 3 scripts:
	send_event.py
	receive_event_simple.py

The Python SBIO wrapper API currently consists of the following methods:
	{"iogen", (PyCFunction)method_iogen, METH_VARARGS, "Python interface for Crank Storyboard SBIO C library function to send an event using iogen syntax"},
	{"sendevent", (PyCFunction)method_sendevent, METH_VARARGS, "Python interface to send a Stoyboard event using gre_io_send syntax, with or without a payload"},
	{"receiveevent", (PyCFunction)method_receiveevent, METH_VARARGS, "Python interface which blocks waiting for a Stoyboard event on a channel, returning a dictionary with payload.  The channel is cached once opened"},	
	{"addeventlistener", (PyCFunction)method_addeventlistener, METH_VARARGS, "Python interface which opens a channel for listening to events by registering a callback handler"},	
	{"closereceivechannel", (PyCFunction)method_closereceivechannel, METH_VARARGS, "Python interface which will explicitly close a previously opened receive channel"},	


