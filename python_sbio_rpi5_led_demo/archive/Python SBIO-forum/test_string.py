#!/usr/bin/python

import threading
import queue
import time
import sbio
import sys

channel_out = "frontend_send"
channel_in = "frontend_send"
q = queue.Queue()

global exitFlag
exitFlag = 0


def event_callback(event):
    print("in event_callback")
    q.put(event)


def receive_events():
    myevent = {}
    quitFlag = 0
    print("Receiver listening and waiting to queue events...")
    # sbio.addeventlistener( channel_in, event_callback )
    while not exitFlag and not quitFlag:
        myevent = sbio.receiveevent(channel_in)
        q.put(myevent)
        # If the event was QUIT then exit receive thread
        if "QUIT" in myevent:
            print("receive_events exit request received ")
            quitFlag = 1

    sbio.closereceivechannel(channel_in)
    print("receive_events exiting ")


def worker():
    print("Worker waiting to process events...")
    quitFlag = 0
    while not exitFlag and not quitFlag:
        the_event = q.get()
        print("processing receive event: ")
        if "QUIT" in the_event:
            print("worker exit on 'QUIT' request received ")
            quitFlag = 1
        else:
            # Do something useful with the event
            for event, payload in the_event.items():
                print(">", event)
                if (payload):
                    for key, value in payload.items():
                        print("-->", key, value)
                else:
                    print("No payload")
        q.task_done()
    print("worker exiting ")


def post_events(counter, delay):
    # send a test event to client via 'channel_out'
    print("in post_events")
    # while counter:
    time.sleep(delay)
    my_custom_event = {}
    my_event_data = {}
    my_event_data['2u1 rows'] = 4
    my_event_data['2u1 selected_index'] = 2
    my_event_data['1s0 row_test_list'] = ",Power off,Cancel,"
    my_custom_event['get_table'] = my_event_data
    sbio.sendevent(channel_out, "-", my_custom_event)
    quit_receiver()

    print("post_events exiting ")

def quit_receiver():
    print("in quit_receiver")
    no_payload = None  # specifically nothing (null) rather than empty
    my_custom_event = {}
    my_custom_event['QUIT'] = no_payload
    sbio.sendevent(channel_in, "-", my_custom_event)


# ensure channel is closed
sbio.closereceivechannel(channel_in)

# daemon threads themselves are killed as soon as the main program exits
receive_thread = threading.Thread(name='receiver', target=receive_events)
receive_thread.setDaemon(True)
worker_thread = threading.Thread(name='worker', target=worker)
worker_thread.setDaemon(True)

# turn-on the worker and receive threads
worker_thread.start()
receive_thread.start()

# Test post events to own listen channel
post_events(1, 1)

time.sleep(1)
quit_receiver()

# block until all tasks are done
# Wait for all threads to complete
receive_thread.join()
worker_thread.join()

print("Exiting Main Thread")
sys.exit()
