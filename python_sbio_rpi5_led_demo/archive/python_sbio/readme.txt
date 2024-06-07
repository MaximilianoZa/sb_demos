=====================================================================
== A basic Python extension which wraps the Storyboard IO library.
=====================================================================

**********************************
** Overview:
**********************************

The code has been built and run with Storyboard 8.0 release and Python 3.10.12.

This is built for use with Python3 and should be built on-target and assumes a Python install is already in-place on the platform.

**********************************
** Package contents:
**********************************
	setup.py
	sbio_python.c
	readme.txt
	samples/
		simple_send.py
		simple_recv_loop.py

**********************************
** How to build:
**********************************

The setup.py file has paths to the target Storyboard runtime and may need to be adjusted depending on source location in the target filesystem.  Modify the 'incdirs' and 'libdirs' paths as required.

To build and install the Python extension on a linux system use the following :

❯ sudo python3 setup.py install
running install
running build
running build_ext
building 'sbio' extension
x86_64-linux-gnu-gcc -Wno-unused-result -Wsign-compare -DNDEBUG -g -fwrapv -O2 -Wall -g -fstack-protector-strong -Wformat -Werror=format-security -g -fwrapv -O2 -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -fPIC -DNDEBUG=1 -I/home/dcarson/Crank_Software_8/Storyboard_Engine/8.0-stable.202303220149/linux-x86_64-opengles_2.0-x11-obj/include -I/usr/include/python3.10 -c sbio_python.c -o build/temp.linux-x86_64-3.10/sbio_python.o
x86_64-linux-gnu-gcc -shared -Wl,-O1 -Wl,-Bsymbolic-functions -Wl,-Bsymbolic-functions -g -fwrapv -O2 -Wl,-Bsymbolic-functions -g -fwrapv -O2 -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 build/temp.linux-x86_64-3.10/sbio_python.o -L/home/dcarson/Crank_Software_8/Storyboard_Engine/8.0-stable.202303220149/linux-x86_64-opengles_2.0-x11-obj/lib -lgreio -o build/lib.linux-x86_64-3.10/sbio.cpython-310-x86_64-linux-gnu.so
running install_lib
copying build/lib.linux-x86_64-3.10/sbio.cpython-310-x86_64-linux-gnu.so -> /usr/local/lib/python3.10/dist-packages
running install_egg_info
Removing /usr/local/lib/python3.10/dist-packages/sbio-1.0.0.egg-info
Writing /usr/local/lib/python3.10/dist-packages/sbio-1.0.0.egg-info

**********************************
** Usage & Examples
**********************************

There are some examples of how to use the extension in the 'samples' directory.

If you are familiar with Storyboard IO, the usage of this extension should be fairly straightforward (if not, consult https://support.cranksoftware.com/hc/en-us/articles/360056943732-Sending-Events-to-a-Storyboard-Application).

The extension uses python dicts to represent messages, so the send_event function takes a dict as a parameter, and the receive_event function returns a dict to the caller. The format of the event dict is as follows:

{
	'event_name': {
		... (event data)
	}
}

For example, an event named "sample.event" with a payload of a uint16_t named 'number' and a string named 'text' would be represented by the extension as:

{
	'sample.event': {
		'2u1:number': 5,
		'1s0:text': 'Hello, world!'
	}
}

An event that does not have a payload is represented specifically with 'None' for the payload:

{
	'sample.event': None
}

**********************************
** Caveats
**********************************

Currently this extension does not support events containing arrays of types (for example, 4s2 for an array of int32_t of size two).

**********************************
** API listing
**********************************

sbio.send_event(channel, target, event)
	channel - the SBIO channel to send the event on
	target - the target of the event, or you can specify "-" if there is no specific target
	event - a python dict representing a SBIO event

	This function sends an SBIO event on a specified SBIO channel.


sbio.receive_event(channel)
	channel - the SBIO channel to receive an event on

	This function will create a receive handle for the specified channel if one does not exist, and wait for events on that channel.  When an event is received, it will return the event to the caller as a python dict.


sbio.close_receive_channel(channel)
	channel - the SBIO channel to close

	This function closes an existing SBIO channel.


