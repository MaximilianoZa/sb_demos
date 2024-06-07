/*
 * Copyright 2007, Crank Software Inc. All Rights Reserved.
 *
 * For more information email info@cranksoftware.com.
 */

/**
 * This is a sample application that will generate IO messages
 * and inject them into the GRE application.
 *
 * It uses IO interface API to send the events
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if !defined(_WIN32)
#include <errno.h>
#endif

#include <gre/greio.h>
//#include <gre/sdk/greal.h>
#include <gre/iodata.h>
#include <Python.h>

#define PY_OBJECT_TOSTRING(obj)		strdup(PyBytes_AsString(PyUnicode_AsUTF8String(obj)))
#define PY_OBJECT_TOLONG(obj)		PyLong_AsLong(PyNumber_Long(obj))
#define PY_OBJECT_TODOUBLE(obj)		PyFloat_AsDouble(PyNumber_Float(obj))


static gre_io_serialized_data_t *
create_event_from_array(int argc, char **argv) {
    gre_io_serialized_data_t *buffer;
    char *  name_buffer;
    char *  target_buffer;
	char *  format_buffer = NULL;
    char *  data_buffer = NULL;
	int 	data_len = 0;
    int     index = 0;

    if(index >= argc) {
        return NULL;
    }

    if((index + 1) > argc) {
		printf("Missing arguments: [-|event_target] event_name [<size><type><length>}[:name] {value}] ...\n");
        return NULL;
    }

	//Allow the user to pass - to indicate no target
	if(strcmp(argv[index], "-") == 0) {
		target_buffer = NULL; 
		//printf("No event_target specified ...\n");
	} else {
		target_buffer = argv[index];
	}
    index++;

	name_buffer = argv[index];
	//printf("The event_name is '%s'\n", name_buffer );
    index++;

	//If we have no data payload to sent, that's OK but don't parse it
	if(index < argc) {
		//printf("Building event_data...\n" );
		format_buffer = greio_array_to_event_data(&argv[index], argc - index, (void **)&data_buffer, &data_len);
		if(!format_buffer) {
			printf("Problem converting command line to event payload\n");
			return NULL;
		}
	}

   	buffer = gre_io_serialize(NULL, target_buffer, name_buffer, format_buffer, data_buffer, data_len);

	if(format_buffer) {
		free(format_buffer);
	}
	if(data_buffer) {
		free(data_buffer);
	}

    return buffer;
}

static void
verify_event(gre_io_t *shandle, gre_io_serialized_data_t *buffer) {
    gre_io_t *          rhandle;
    char                *event_name, *revent_name;
    char                *event_target, *revent_target;
    char                *event_format, *revent_format;
    void                *event_data, *revent_data;
    int                 ret, nbytes, rnbytes;
    gre_io_serialized_data_t *nbuffer;
    iodata_get_event_t        nevent;

    //Take apart the sent message for comparison
    nbytes = gre_io_unserialize(buffer, &event_target,
                                &event_name, &event_format, &event_data);

    //If we didn't send a set event, then don't do the verification
    if(strcmp(event_name, GREIO_EVENT_DATA_SET) != 0) {
        return;
    }

    //Put our reply channel information in the 'get' event
    memset(&nevent, 0, sizeof(nevent));
    strcpy(nevent.reply_channel, "iogen-receive");

    rhandle = gre_io_open(nevent.reply_channel, GRE_IO_TYPE_RDONLY);
    if(rhandle == NULL) {
        printf("Can't open receive handle [%s], error %d\n", nevent.reply_channel, errno);
        return;
    }

    nbuffer = gre_io_serialize(NULL, event_target,
                               GREIO_EVENT_DATA_GET, event_format, &nevent, sizeof(nevent));

    //Send the get message, then wait for the reply
    ret = gre_io_send(shandle, nbuffer);
    if (ret < 0) {
    	printf("Can't send using handle, error %d\n", errno);
    }
    ret = gre_io_receive(rhandle, &nbuffer);
    if (ret < 0) {
    	printf("Can't receive on handle, error %d\n", errno);
    }

    rnbytes = gre_io_unserialize(nbuffer, &revent_target,
                                &revent_name, &revent_format, &revent_data);

    if(revent_target == NULL || event_target == NULL ||
              strcmp(revent_target, event_target) != 0) {
        printf("FAIL: Event targets don't match %s vs %s\n",
                (event_target) ? event_target : "NULL",
                (revent_target) ? revent_target : "NULL");
    } else if(rnbytes != nbytes) {
        printf("FAIL: Number of bytes don't match %d vs %d\n", nbytes, rnbytes);
    } else if(memcmp(revent_data, event_data, nbytes) != 0) {
        printf("FAIL: Event data is not equivalent \n");
    } else {
        printf("Verification OK!\n");
    }

	gre_io_free_buffer(nbuffer);
	gre_io_close(rhandle);
}

static PyObject *listener_callback = NULL;

PyObject * call_eventlistener( PyObject *event_dict )
{
	PyObject *arglist;
	PyObject *result = NULL;
	
	if( listener_callback != NULL)
	{
		/* Now call the callback */
		arglist = Py_BuildValue("(o)", event_dict);
		result = PyObject_CallObject(listener_callback, arglist);
		Py_DECREF(arglist);
		Py_XINCREF(result);
	}
	return result;
}


static PyObject *
method_addeventlistener(PyObject *self, PyObject *args)
{
    PyObject *result = NULL;
	PyObject *event_channel;
    PyObject *temp;
    char *  channel_name;

    if (PyArg_ParseTuple(args, "sO:set_callback", &event_channel, &temp)) {
		if( event_channel == NULL ) 
		{			
			PyErr_SetString(PyExc_TypeError,"Error! Bad or missing string argument 'channel'. Usage: sbio.closereceivechannel( string channel )");
			return NULL;
		}else
		{
			channel_name = PY_OBJECT_TOSTRING( event_channel); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
			printf("method_addeventlistener: channel name is '%s'\n", channel_name );
			free( channel_name );
		}

		if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);         		/* Add a reference to new callback */
        Py_XDECREF(listener_callback);  /* Dispose of previous callback */
        listener_callback = temp;       /* Remember new callback */
        printf("callback registered \n" );

        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
		Py_DECREF(event_channel);
        result = Py_None;
    }
    return result;
}

//**********************************************
// sbio.iogen("test_driver","-","gre.press", "4u1:button", 0, "4u1:timestamp", 0, "2u1:subtype", 0, "2s1:x", 100, "2s1:y", 150, "2s1:z", 0, "2s1:id", 0, "2s1:spare", 0)
//**********************************************
static PyObject *method_iogen(PyObject *self, PyObject *args) {

    char 	data_buffer[256];
	char 	**argv = NULL;
	
	PyObject *data_value;
    Py_ssize_t i;
	Py_ssize_t TupleSize;

    gre_io_t                 *send_handle;
    gre_io_serialized_data_t *buffer;

    TupleSize = PyTuple_Size(args);
	if(!TupleSize) {
        if(!PyErr_Occurred()) 
            PyErr_SetString(PyExc_TypeError,"You must supply at least one argument.");
        return NULL;
    }

	//printf("\nmethod_iogen() - Building event to send... \n");

	// allocate an array of strings, one for each argrument
	argv = malloc(TupleSize * sizeof(char *) ); 
	   
 	for( i=0; i<TupleSize; i++ )
	{
        data_value = PyTuple_GetItem(args,i);
        if(data_value == NULL)
		{ 
			PyErr_Format(PyExc_ValueError, "Error!: Argument [%d] is invalid\n", i);
			free(argv);
			return 0;
		}else
		{
			/// todo parse format and validate value type matches type
			if( PyFloat_Check( data_value ) )
			{
				sprintf(data_buffer, "%f", PY_OBJECT_TODOUBLE(data_value));
				argv[i] = strdup(data_buffer);  // Note: strdup uses malloc so requires freeing
				
			} else if( PyLong_Check( data_value ) )
			{
				sprintf(data_buffer, "%ld", PY_OBJECT_TOLONG(data_value));
				argv[i] = strdup(data_buffer);  // Note: strdup uses malloc so requires freeing

			} else if ( PyUnicode_Check( data_value ) )
			{
				argv[i] = PY_OBJECT_TOSTRING(data_value);  // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
			} else
			{
				argv[i] = strdup("nil");  // Note: strdup uses malloc so requires freeing
			}
		}
		Py_DECREF(data_value);
	}

	//printf("Connecting to Storyboard IO channel_name [%s] \n", argv[0]);
	send_handle = gre_io_open(argv[0], GRE_IO_TYPE_WRONLY);
	if(send_handle != NULL) {
		// skip first argument in the array which is the channel name 
		buffer = create_event_from_array(TupleSize-1, &argv[1]);

		if(buffer != NULL) {
			gre_io_send(send_handle, buffer);

			//Issue a get to verify the data we sent
			verify_event(send_handle, buffer);

			//printf("Cleaning up\n");
			gre_io_free_buffer(buffer);
		}
		gre_io_close(send_handle);
		PyErr_Clear();
	}else
	{
		PyErr_Clear();
        PyErr_Format(PyExc_ValueError, "Can't open send handle [%s] error %d\n", argv[0], errno);
	}	
	
	// clean-up dynamic argv structure
	if( argv != NULL )
	{		
		for( i=0; i<TupleSize; i++ )
			free(argv[i]);

		free(argv);
	}
	//printf("Freed\n");
	
	PyErr_Clear();
    return PyLong_FromLong(0);
}

/*
see : https://support.cranksoftware.com/hc/en-us/articles/360039999352-Sending-Events-to-a-Storyboard-Application

C/C++ Type 	| Format String | 	Data Size
============|===============|================
int8_t			1s1				1 byte
uint8_t			1u1				1 byte
int16_t			2s1				2 bytes
uint16_t		2u1				2 bytes
int32_t			4s1				4 bytes
uint32_t		4u1				4 bytes
int64_t			8s1				8 bytes
uint64_t		8u1				8 bytes
float (IEEE754)	4f1				4 bytes
char *			1s0				Length of string including nul terminator

Data parameters must be sent in order of descending alignment requirements. 
Example: 4u1 4u1 2u1 1s0 is good, 2u1 4u1 4u1 1s0 is not
*/

//**********************************************
// my_event={
//	"gre.press": {
//		"4u1:button": 0,
//		"4u1:timestamp": 0,
//		"2u1:subtype": 0,
//		"2s1:x": 100, 
//		"2s1:y": 150,
//		"2s1:z": 0, 
//		"2s1:id": 0, 
//		"2s1:spare":0
//		}
//	}
// sbio.sendevent( string channel, string target, dict my_event ) 
//**********************************************
static PyObject *method_sendevent(PyObject *self, PyObject *args) {
	
    Py_ssize_t TupleSize = PyTuple_Size(args);
	PyObject *event_dict, *key;
	PyObject *event_channel, *event_target, *event_data, *data_fmt_key, *data_value;
	Py_ssize_t pos_event = 0;
	Py_ssize_t pos_data = 0;
	Py_ssize_t num_payload_vars = 0;
	
    gre_io_t                 *send_handle;
    gre_io_serialized_data_t *buffer;
	
    char *  channel_name;
    char *  event_target_name;
    char *  event_name;
    char   	data_buffer[256];
    int     index = 0;
	int 	argc = 0;
	char 	**argv = NULL;
	
	if(TupleSize != 3 ) {
		if(!PyErr_Occurred()) 
			PyErr_SetString(PyExc_TypeError,"Error! Bad number of arguments - Usage: sbio.sendevent( string channel, string target, dict my_event )");
		return NULL;
	}
	
	event_channel = PyTuple_GetItem(args,index++);
	if( event_channel == NULL || PyUnicode_Check(event_channel) != 1 ) 
	{			
		PyErr_SetString(PyExc_TypeError,"Error! Bad or missing string argument 'channel'. Usage: sbio.sendevent( string channel, string target, dict my_event )");
		return NULL;
	}else
	{
		channel_name = PY_OBJECT_TOSTRING(event_channel); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
	}
	//Py_DECREF(event_channel);

	event_target = PyTuple_GetItem(args,index++);
	if( event_target == NULL || PyUnicode_Check(event_target) != 1 ) 
	{			
		PyErr_SetString(PyExc_TypeError,"Error! Bad or missing string argument 'target'. Usage: sbio.sendevent( string channel, string target, dict my_event )");
		return NULL;
	}else
	{
		event_target_name = PY_OBJECT_TOSTRING(event_target); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
	}
	//Py_DECREF(event_target);

    event_dict = PyTuple_GetItem(args,index++);
	if(event_dict == NULL || !PyDict_Check(event_dict) ) 
	{			
		PyErr_SetString(PyExc_TypeError,"Error! Invalid argument -  dict my_event");
		return NULL;
	}
 	
	/* there *should* only be one top level key-value dictionary entry in the event which
		is the name of the event and a dictionary containing key-value pairs
		though it is feasible to support multiple events in a dictionary to send in a sequence
		*/
	while (PyDict_Next(event_dict, &pos_event, &key, &event_data)) {

		num_payload_vars = PyDict_Size(event_data);
		if( num_payload_vars < 0 ) // return value negative if empty or invalid
		{
			num_payload_vars = 0; 
		}

		if( key == NULL || PyUnicode_Check(key) != 1 ) 
		{			
			PyErr_SetString(PyExc_TypeError,"Error! Bad or missing string argument 'event' in ductionary. Usage: sbio.sendevent( string channel, dict my_event )");
			Py_DECREF(key);
			return NULL;
		}else
		{
			event_name = PY_OBJECT_TOSTRING(key);	// Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
		}
		//Py_DECREF(key);
				
		argc = 0;
		argv = (char **)malloc( sizeof(char *) * 2 ); 		
		argv[argc++] = strdup(event_target_name); 	// use a copy as we re-use this for each event
		argv[argc++] = event_name; 			
		//printf("\nmethod_sendevent() - Building event '%s' to send on channel '%s'... \n", event_name, channel_name );

		/* do something with the values... */
		if(event_data == NULL || !PyDict_Check(event_data) || num_payload_vars == 0 ) 
		{			
			// No event payload = no problem!
			printf("\tNo payload\n"); // the event has no payload data
		}else
		{			
			pos_data = 0;
			// Reallocate array for the event payload again starting with event target and event name already in-place
			// where each parameter is 2 array entries - format and value
			argv = (char **)realloc( argv, sizeof(char *) * (( num_payload_vars * 2) + 2 ) ); 
			if( argv == NULL  ) 
			{			
				PyErr_SetString(PyExc_TypeError,"Error! Reallocation of array for payload failed.)");
				return NULL;
			}
		
			while (PyDict_Next(event_data, &pos_data, &data_fmt_key, &data_value)) {
				
				argv[argc] = PY_OBJECT_TOSTRING(data_fmt_key); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
				//printf("\t'%s' = ", argv[argc]);
				argc++;
				if( data_value != NULL )
				{
					// todo parse format and validate value type matches type
					if( PyFloat_Check( data_value ) )
					{
						sprintf(data_buffer, "%f", PY_OBJECT_TODOUBLE(data_value));
						argv[argc] = strdup(data_buffer);  // Note: strdup uses malloc so requires freeing
						
					} else if( PyLong_Check( data_value ) )
					{
						sprintf(data_buffer, "%ld", PY_OBJECT_TOLONG(data_value));
						argv[argc] = strdup(data_buffer);  // Note: strdup uses malloc so requires freeing

					} else if ( PyUnicode_Check( data_value ) )
					{
						argv[argc] = PY_OBJECT_TOSTRING(data_value);  // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
					} else
					{
						argv[argc] = strdup("nil");  // Note: strdup uses malloc so requires freeing
					}
					//printf("'%s'\n", argv[argc]);
					argc++;
				}
				//Py_DECREF(data_fmt_key);
				//Py_DECREF(data_value);
			}
		}
		//Py_DECREF(event_data);
		
		/// serialize and send the event
		//printf("Connecting to Storyboard IO channel_name [%s] \n", channel_name);
		send_handle = gre_io_open(channel_name, GRE_IO_TYPE_WRONLY);
		if(send_handle != NULL) {
			buffer = create_event_from_array(argc, &argv[0]);

			if(buffer != NULL) {
				//printf("Sending event '%s' to channel '%s' \n", event_name, channel_name); // todo parse format and validate value type
				gre_io_send(send_handle, buffer);

				//Issue a get to verify the data we sent
				//verify_event(send_handle, buffer);
				//printf("Cleaning up\n");
				gre_io_free_buffer(buffer);
			}

			gre_io_close(send_handle);
			send_handle = NULL;
		}else
		{
			PyErr_Clear();
			PyErr_Format(PyExc_ValueError, "Can't open send handle [%s] error %d\n", channel_name, errno);
			printf("Can't open send handle [%s] error %d\n", channel_name, errno);
		}	
		
		// clean-up dynamic argv structure for the event
		if( argv != NULL )
		{		
			for( index=0; index<argc; index++ )
			{
				free(argv[index]);
			}
			free(argv);
			argv = NULL;

		}
	}
	// finished with channel and target so release to clean-up
	free(channel_name);
	free(event_target_name);
	//printf("Freed\n");
	//printf("Releasing event dictionary\n");
    //Py_DECREF(event_dict);

	PyErr_Clear();
    return PyLong_FromLong(0);
}

//**********************************************
// A blocking call which wait on an event listening to the named channel
// returning the event and any payload as a dictionary
//
// The channel handle will be cached once opened until explicity closed by a  call to 
// 
// dict event = sbio.receiveevent( string channel ) 
//**********************************************
gre_io_t  *recv_handle = NULL;

static PyObject *method_receiveevent(PyObject *self, PyObject *args) {
    Py_ssize_t TupleSize = PyTuple_Size(args);
	PyObject *event_dict, *payload_dict, *event;
	PyObject *event_channel;
	
    gre_io_serialized_data_t *buffer = NULL;
	
    char *  channel_name;
    char *  event_target_name;
    char *  event_name;
    char *  event_format;
	char *	event_payload;
	uint8_t *event_data;
    int     index = 0;
	int		ret;
	int		nbytes;
	
	if(TupleSize != 1 ) {
		if(!PyErr_Occurred()) 
			PyErr_SetString(PyExc_TypeError,"Error! Bad number of arguments - Usage: sbio.receiveevent( string channel )");
		return NULL;
	}
	
	event_channel = PyTuple_GetItem(args,index++);
	if( event_channel == NULL || PyUnicode_Check(event_channel) != 1 ) 
	{			
		PyErr_SetString(PyExc_TypeError,"Error! Bad or missing string argument 'channel'. Usage: sbio.receiveevent( string channel )");
		return NULL;
	}else
	{
		channel_name = PY_OBJECT_TOSTRING(event_channel); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
	}
	//Py_DECREF(event_channel);
	
	
	// use cached handle if the channel has already been opened previously
	// this can be explicitly closed by a call to 
	if( recv_handle == NULL )
	{
		//printf("Opening IO channel [%s] for receive..\n", channel_name);
		recv_handle = gre_io_open(channel_name, GRE_IO_TYPE_RDONLY);
		if(recv_handle == NULL) {
			printf("Can't open IO channel %s\n", channel_name);
			return 0;
		}
	}

	// Initialise our return object, a Python dictionary containing an event
	event_dict = PyDict_New();
	Py_XINCREF(event_dict);
	
	Py_BEGIN_ALLOW_THREADS
	//printf("Waiting on channel [%s]\n", channel_name);
	ret = gre_io_receive(recv_handle, &buffer);
	if(ret < 0) {
		printf("Problem receiving data on channel [%s]\n", channel_name);
	}else
	{
		 nbytes = gre_io_unserialize(buffer,
								&event_target_name,
								&event_name,
								&event_format,
								(void **)&event_data);
		 //printf("Event Received [%s] on channel [%s]:\n", event_name, channel_name);
		 //printf("\tEvent Target: [%s]\n", event_target_name);    
		 //printf("\tEvent Format: [%s]\n", event_format);    
		 //printf("\tEvent Data (%d bytes):\n", nbytes);    

		/**
		 * Convert an user data event payload to a parseable data string.  Client are expected to
		 * expect a NULL return code if the format string or data is empty. Clients are responsible for
		 * the memory associated with the string returned and should call free() to deallocate it.
		 *
		 * @param event_format 	A null terminated string that contains a standard format string
		 * @param event_data    A pointer to a block of memory to be interpreted via event_format
		 * @param event_data_nbytes The number of bytes that event_data points to
		 * @return An allocated string parsable via greio_string_to_event_data or NULL on failure
		 */
		event_payload = greio_event_data_to_string(event_format, event_data, nbytes);
		if( event_payload != NULL )
		{
			printf("\tEvent Payload [%s]:\n", event_payload);
			event = Py_BuildValue("s", event_name);
			
			// Populate the payload dictionary from the payload key value pairs
			PyObject *key;
			PyObject *value;
			const char *p = event_payload;
			char * buffer = NULL;
			
			size_t i = 0; // count of key value pairs in payload
			
			payload_dict = PyDict_New();
			while ( *p )
			{
				// parse key
				while ( isspace( ( unsigned char )*p ) ) ++p;
				if ( *p )
				{
					const char *q = p;
					char *fmt = NULL;
					// parse key and format specifier
					while ( *p && !isspace( ( unsigned char )*p ) ) ++p;

					size_t length = p - q;
					buffer = calloc( length + 1, sizeof( char ) );
					strncpy( buffer, q, length );
					key = Py_BuildValue("s", buffer);
					
					// identify value parameter type from format specifier
					length = strcspn (buffer,":");
					fmt = calloc( length + 1, sizeof( char ) );
					strncpy( fmt, buffer, length );
					free(buffer);
					
					// parse value, skip to payload start
					while ( isspace( ( unsigned char )*p ) ) ++p;
					if ( *p )
					{
						const char *q = p;
						size_t length = 0;
						
						if( strcmp(fmt, "1s0") == 0 )	// it is a string so value delimited by ".."
						{
							q++;										// skip initial quotes to first string character
							while ( *p++ && (*p != '"') ) length++;		// count characters to trailing quotes		

							buffer = calloc( length + 1, sizeof( char ) );
							strncpy( buffer, q, length );
							value = Py_BuildValue("s", buffer);
							free(buffer);
						}
						else // it is a numeric value
						{
							while ( *p && !isspace( ( unsigned char )*p ) ) ++p; // skip to end of value marked by whitespace
							length = p - q;

							buffer = calloc( length + 1, sizeof( char ) );
							strncpy( buffer, q, length );
							value = Py_BuildValue("s", buffer); // TODO: parse and cast to correct python type based on fmt, everything is returned a string currently
							free(buffer);
						}
						++i;
						PyDict_SetItem( payload_dict, key, value);
					}
					free(fmt);
				}
			}

			
			PyDict_SetItem( event_dict, event, payload_dict );
			Py_XINCREF(payload_dict);
			Py_XINCREF(event);
		}else
		{
			//printf("\tEvent has no Payload \n");
			payload_dict = Py_None;
			event = Py_BuildValue("s", event_name);
			PyDict_SetItem( event_dict, event, payload_dict );
			Py_XINCREF(payload_dict);
			Py_XINCREF(event);
		}
		
		//call_eventlistener(event_dict);
	}
	//gre_io_close(recv_handle);
	Py_END_ALLOW_THREADS
	
	// finished with channel and target so release to clean-up
	free(channel_name);
	//printf("Freed\n");

	PyErr_Clear();
    return event_dict;
}

//**********************************************
// The receive channel will be closed and handle released
// 
// dict event = sbio.method_closereceivechannel( string channel ) 
//**********************************************

static PyObject *method_closereceivechannel(PyObject *self, PyObject *args) {
    Py_ssize_t TupleSize = PyTuple_Size(args);
	PyObject *event_channel;
	int     index = 0;
	
     char *  channel_name;
	
	if(TupleSize != 1 ) {
		if(!PyErr_Occurred()) 
			PyErr_SetString(PyExc_TypeError,"Error! Bad number of arguments - Usage: sbio.closereceivechannel( string channel )");
		return NULL;
	}
	
	event_channel = PyTuple_GetItem(args,index++);
	if( event_channel == NULL || PyUnicode_Check(event_channel) != 1 ) 
	{			
		PyErr_SetString(PyExc_TypeError,"Error! Bad or missing string argument 'channel'. Usage: sbio.closereceivechannel( string channel )");
		return NULL;
	}else
	{
		channel_name = PY_OBJECT_TOSTRING(event_channel); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
	}
	Py_DECREF(event_channel);
	
	// use cached handle if the channel has already been opened previously
	// this can be explicitly closed by a call to 
	if( recv_handle != NULL )
	{
		printf("Closing IO receive channel [%s]..\n", channel_name);
		gre_io_close(recv_handle);
		recv_handle = NULL;
	}
	
	// finished with channel and target so release to clean-up
	free(channel_name);
	//printf("Freed\n");

	PyErr_Clear();
    return PyLong_FromLong(0);
}


//**********************************************
// Python extension boilerplate
//**********************************************
struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

static PyObject *
error_out(PyObject *m) {
    struct module_state *st = GETSTATE(m);
    PyErr_SetString(st->error, "sbio - something bad happened");
	printf("sbio - something bad happened");
    return NULL;
}

static PyMethodDef sbio_methods[] = {
	{"iogen", (PyCFunction)method_iogen, METH_VARARGS, "Python interface for Crank Storyboard SBIO C library function to send an event using iogen syntax"},
	{"sendevent", (PyCFunction)method_sendevent, METH_VARARGS, "Python interface to send a Stoyboard event using gre_io_send syntax, with or without a payload"},
	{"receiveevent", (PyCFunction)method_receiveevent, METH_VARARGS, "Python interface which blocks waiting for a Stoyboard event on a channel, returning a dictionary with payload.  The channel is cached once opened"},	
	{"addeventlistener", (PyCFunction)method_addeventlistener, METH_VARARGS, "Python interface which opens a channel for listening to events by registering a callback handler"},	
	{"closereceivechannel", (PyCFunction)method_closereceivechannel, METH_VARARGS, "Python interface which will explicitly close a previously opened receive channel"},	
	{"error_out", (PyCFunction)error_out, METH_NOARGS, NULL},
	{NULL, NULL, 0, NULL}
};

static int sbio_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int sbio_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}

static struct PyModuleDef sbio_module = {
	PyModuleDef_HEAD_INIT,
	"sbio",
	"Python interface for Crank Storyboard SBIO C library functions",
	sizeof(struct module_state),
	sbio_methods,
	NULL,
	sbio_traverse,
	sbio_clear,
	NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC 
PyInit_sbio(void)
{
    PyObject *module = PyModule_Create(&sbio_module);

    if (module == NULL)
        INITERROR;
	
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("sbio.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }
    return module;
}
