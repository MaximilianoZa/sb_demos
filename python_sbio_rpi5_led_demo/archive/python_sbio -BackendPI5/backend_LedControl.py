#!/usr/bin/python

import sbio
import gpiozero

# GPIO setup
ledblue = gpiozero.LED(2)
ledwhite = gpiozero.LED(3)
ledred = gpiozero.LED(4)

# Local Variables
toggleblue = 0
togglewhite = 0
togglered = 0
msg = "placeholder"

# SBIO channels
channelRcv = "sbio_backend"   #to receive events on the backend from the UI
channelSnd = "sbio_frontend" #to send events from the backend to the UI
ledState = "placeHolder" #string aux
event_data = {} #event payload

# LED frontend events
# Define the LEDACK_EVENT and LEDACK_FMT constants
LEDACK_EVENT = "ledAck"
LEDACK_FMT = "1s0 ack"

# loop until manually exited, will receive events on the channel and print their contents
print("Waiting for events...")
while 1:
    sample_event = sbio.receive_event(channelRcv)

    for message in sample_event.keys():
        print("%s:" % message)
        

        if sample_event[message] is not None:
            for inner_key in sample_event[message]:
                print("  %s : %s (type %s)" % (inner_key, sample_event[message][inner_key], type(sample_event[message][inner_key])))

    msg = sample_event[message][inner_key]
    match msg:

        case "blue":
            toggleblue = toggleblue + 1
            if toggleblue == 1:
                ledblue.on()
                ledState = "blue-ON"
            else:
                ledblue.off()
                toggleblue = 0
                ledState = "blue-OFF"

        case "white":
            togglewhite = togglewhite + 1
            if togglewhite == 1:
                ledwhite.on()
                ledState = "white-ON"
            else:
                ledwhite.off()
                togglewhite = 0
                ledState = "white-OFF"

        case "red":
            togglered = togglered + 1
            if togglered == 1:
                ledred.on()
                ledState = "red-ON"
            else:
                ledred.off()
                togglered = 0
                ledState= "red-OFF"

    event_data = {
        LEDACK_EVENT: {
            LEDACK_FMT: ledState
        }
    }

    sbio.send_event(channelSnd ,"-", event_data)
    print(event_data)






