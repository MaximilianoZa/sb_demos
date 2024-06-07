# A basic Python extension which wraps the Storyboard IO library.

## Overview:
This project provides a basic Python extension that wraps the Storyboard IO (SBIO) library, facilitating seamless integration of SBIO C libraries with Python code. It is compatible with Storyboard 8.1 and Python 3.11. The backend code is designed to be built on-target, assuming a pre-installed Python environment on the platform. The idea of the demo is to demonstrate how to seamlessly integrate SBIO C libraries within Python code, allowing for straightforward communication between the Storyboard application and Python, in this example we are communicating particularly with the GPIOs.
Note: This project does not include the Storyboard Runtime engine. The runtime engine can be found in the installation path of Storyboard.

## Connection diagram:
![alt text](https://github.com/MaximilianoZa/sb_demos/blob/main/python_sbio_rpi5_led_demo/Connection_diagram.png?raw+true)

## Package contents:
- **frontend/**
  - **sb_led_blink_app/**: SB front end app
- **backend/**
  - **setup.py**: Script to build code on target
  - **sbio_python.c**: SBIO C wrapped in Python
  - **backend_LedControl.py**: Communicating: SB App <=> Backend Py <=> GPIO
  - **samples/**: Examples to send and receive using SBIO wrappers
- **launch_ledblink.sh**: Launching script
- **README.txt**
- **Connection_diagram.png**

## How to build:
The `setup.py` file has paths to the target Storyboard runtime and may need to be adjusted depending on source location in the target filesystem. Modify the `incdirs` and `libdirs` paths as required.

`incdirs = '/home/pi/crank/runtimes/linux-raspberry-aarch64-opengles_2.0-drm-obj/include'`

`libdirs = '/home/pi/crank/runtimes/linux-raspberry-aarch64-opengles_2.0-drm-obj/lib'`

To build and install the Python extension on a Linux system use the following:
sudo python3 setup.py install

## Usage & Examples
There are some examples of how to use the extension in the 'samples' directory.

If you are familiar with Storyboard IO, the usage of this extension should be fairly straightforward (if not, consult https://support.cranksoftware.com/hc/en-us/articles/360056943732-Sending-Events-to-a-Storyboard-Application).

The extension uses python dicts to represent messages, so the send_event function takes a dict as a parameter, and the receive_event function returns a dict to the caller. The format of the event dict is as follows:

`{
	'event_name': {
		... (event data)
	}
}`

For example, an event named "sample.event" with a payload of a uint16_t named 'number' and a string named 'text' would be represented by the extension as:

`{
	'sample.event': {
		'2u1:number': 5,
		'1s0:text': 'Hello, world!'
	}
}`

An event that does not have a payload is represented specifically with 'None' for the payload:

`{
	'sample.event': None
}`

## Caveats
- Currently, this extension does not support events containing arrays of types (for example, `4s2` for an array of `int32_t` of size two).
- The `1s1` data type in events does not seem to behave correctly. The value can change between when it is sent and received. This is, however, not related to the Python wrapper and can be worked around using `2s1`.

## API listing

## `sbio.send_event(channel, target, event)`
- `channel`: The SBIO channel to send the event on
- `target`: The target of the event, or you can specify "-" if there is no specific target
- `event`: A Python dict representing a SBIO event

This function sends an SBIO event on a specified SBIO channel.

## `sbio.receive_event(channel)`
- `channel`: The SBIO channel to receive an event on

This function will create a receive handle for the specified channel if one does not exist, and wait for events on that channel. When an event is received, it will return the event to the caller as a Python dict.

## `sbio.close_receive_channel(channel)`
- `channel`: The SBIO channel to close

This function closes an existing SBIO channel.

