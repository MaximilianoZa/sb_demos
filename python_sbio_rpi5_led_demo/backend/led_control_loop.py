#!/usr/bin/python
import gpiozero
import sbio

# GPIO Setup
ledblue = gpiozero.LED(2)
ledwhite = gpiozero.LED(3)
ledred = gpiozero.LED(4)

# Local Variables
toggleblue = 0
togglewhite = 0
togglered = 0
msg = "placeholder"

# SBIO Channels
channelRcv = "sbio_backend"  # to receive events in the backend from the user interface
channelSnd = "sbio_frontend"  # to send events from the backend to the user interface
ledState = "placeHolder"  # string aux
event_data = {}  # event payload

# LED Frontend Events: Constants
LEDACK_EVENT = "ledAck"
LEDACK_FMT = "1s0 ack"

# Main Loop
# Loop until manually exited
# It receives events on the channel, toggles the GPIOs accordingly and responds ack
print("Waiting for events...")
while 1:
    # Receive data form SBIO channel
    sample_event = sbio.receive_event(channelRcv)
    for message in sample_event.keys():
        print("%s:" % message)

        if sample_event[message] is not None:
            for inner_key in sample_event[message]:
                print(
                    "  %s : %s (type %s)"
                    % (
                        inner_key,
                        sample_event[message][inner_key],
                        type(sample_event[message][inner_key]),
                    )
                )

    # Process data
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
                ledState = "red-OFF"

    # Set data to SBIO channel
    event_data = {LEDACK_EVENT: {LEDACK_FMT: ledState}}

    # Send data to SBIO channel
    sbio.send_event(channelSnd, "-", event_data)
    print(event_data)
