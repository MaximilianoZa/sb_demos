#!/usr/bin/python

import sbio

# this is the name of the SBIO channel that events will be received on
channel = "sbio_test"

# loop until manually exited, will receive events on the channel and print their contents
print("Waiting for events...")
while 1:
    sample_event = sbio.receive_event(channel)

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
