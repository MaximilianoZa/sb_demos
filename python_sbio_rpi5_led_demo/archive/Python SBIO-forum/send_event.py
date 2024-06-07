import sbio

channel_out = "LuaModulesTest"
channel_in = "test_driver"

my_event={
	"custom": {
		"4u1:rpm": 100,
		"4f1:speed": 123.456,
		"4s1:negative": -105543,
		"1s0:dummy":"dummy text"
		},
	"gre.press": {
		"4u1:button": 0,
		"4u1:timestamp": 0,
		"2u1:subtype": 0,
		"2s1:x": 100,
		"2s1:y": 150,
		"2s1:z": 0,
		"2s1:id": 0,
		"2s1:spare":0
		},
	"gre.release": {
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
sbio.sendevent(channel_out, "-", my_event)

#send event with no payload
no_payload = None # specifically nothing (null) rather than empty
my_custom_event = {}
my_custom_event['ev_temp_update'] = no_payload
sbio.sendevent(channel_out, "-", my_custom_event)

#send a data set event
# set the string variable 'screen_name' to "MainScreen"
my_custom_event = {}
my_event_data = {}
my_event_data['1s0:screen_name']="MainScreen"
my_custom_event['greio.iodata_set'] = my_event_data
sbio.sendevent(channel_out, "screen_name", my_custom_event)

# empty dictionary for event and any data
my_custom_event = {}
my_event_data = {}
my_event_data['4u1:target_temp']=39
my_event_data['4u1:actual_temp']=37
my_event_data['4f1:step_temp']=5E-02
my_event_data['1s0:label']="custom_settings"
my_custom_event['ev_temp_update'] = my_event_data
sbio.sendevent(channel_out, "-", my_custom_event)

sbio.iogen(channel_out,"-","gre.press", "4u1:button", 0, "4u1:timestamp", 0, "2u1:subtype", 0, "2s1:x", 100, "2s1:y", 150, "2s1:z", 0, "2s1:id", 0, "2s1:spare", 0)

sbio.iogen(channel_out,"-","number_event", "4u1:integer", 12345, "4f1:float", 27e-3)

#send a data set event
# set the string variable 'screen_name' to "SecondScreen"
my_custom_event = {}
my_event_data = {}
my_event_data['1s0:screen_name']="SecondScreen"
my_custom_event['greio.iodata_set'] = my_event_data
sbio.sendevent(channel_out, "screen_name", my_custom_event)

#sbio.iogen(channel_out,"-","number_event", "4u1:integer", 54321, "4f1:float", 125e2)

print("Sending QUIT to listener")
no_payload = None # specifically nothing (null) rather than empty
my_custom_event = {}
my_custom_event['QUIT'] = no_payload
sbio.sendevent(channel_out, "-", my_custom_event)

print("done")
