#!/usr/bin/python

import sbio

# this is the name of the SBIO channel the event will be sent on
channel = "sbio_test"

# this dict represents an SBIO event
sample_event = {
    "test.event1": {
        "2u1:val": 1
    }
}

# this is another way of building an event dict
another_event = {}
another_event_data = {}
another_event_data['4s1:signed_val'] = 37
another_event_data['4f1:float_val'] = 5E-02
another_event_data['2u1:unsigned_val'] = 5
another_event_data['1s0:string'] = "Test String"
another_event['test.event2'] = another_event_data

# this is an event dict that does not have a payload
no_payload = None # specifically nothing (null) rather than empty
empty_event = {}
empty_event['test.event3'] = no_payload

# this will send the above event dicts on the above specified channel
sbio.send_event(channel, "-", sample_event)
sbio.send_event(channel, "-", another_event)
sbio.send_event(channel, "-", empty_event)