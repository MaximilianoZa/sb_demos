=====================================================================
== A basic Python extension which wraps the Storyboard IO library.
=====================================================================

**********************************
** Overview:
**********************************

The code has been built and run with Storyboard 8.1 release and Python 3.11.
This is built for use with Python3 and should be built on-target and assumes a Python install is already in-place on the platform.

**********************************
** Package contents:
**********************************
	setup.py			#to build code on target
	sbio_python.c 			# SBIO C wrapped in python  
	backend_LedControl.py		#comunicating: SB App <=> Backend Py <=> GPIO
	readme.txt
	samples/  			#examples to send and receive using sbio wrappers
		simple_send.py 
		simple_recv_loop.py
	sb_led_blink_app/ 		#SB front end app
		fonts
		images
		scripts
		LedBlinking.gapp
	launch_ledblink.sh  		#launching script
	

**********************************
** How to build:
**********************************

The setup.py file has paths to the target Storyboard runtime and may need to be adjusted depending on source location in the target filesystem.  Modify the 'incdirs' and 'libdirs' paths as required.

incdirs = '../crank/runtimes/linux-raspberry-aarch64-opengles_2.0-drm-obj/include'
libdirs = '../crank/runtimes/linux-raspberry-aarch64-opengles_2.0-drm-obj/lib'

To build and install the Python extension on a linux system use the following :

❯ sudo python3 setup.py install


**********************************
** Usage & Examples
**********************************

There are some examples of how to use the extension in the 'samples' directory.

If you are familiar with Storyboard IO, the usage of this extension should be fairly straightforward (if not, consult https://support.cranksoftware.com/hc/en-us/articles/360056943732-Sending-Events-to-a-Storyboard-Application).

The extension uses python dicts to represent messages, so the send_event function takes a dict as a parameter, and the receive_event function returns a dict to the caller. The format of the event dict is as follows:

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


