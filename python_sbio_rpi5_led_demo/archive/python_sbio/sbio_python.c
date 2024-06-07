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
#include <gre/iodata.h>
#include <Python.h>

#define PY_OBJECT_TOSTRING(obj) strdup(PyBytes_AsString(PyUnicode_AsUTF8String(obj)))
#define PY_OBJECT_TOLONG(obj) PyLong_AsLong(PyNumber_Long(obj))
#define PY_OBJECT_TODOUBLE(obj) PyFloat_AsDouble(PyNumber_Float(obj))

static gre_io_serialized_data_t *
create_event_from_array(int argc, char **argv)
{
	gre_io_serialized_data_t *buffer;
	char *name_buffer;
	char *target_buffer;
	char *format_buffer = NULL;
	char *data_buffer = NULL;
	int data_len = 0;
	int index = 0;

	if (index >= argc)
	{
		return NULL;
	}

	if ((index + 1) > argc)
	{
		return NULL;
	}

	// Allow the user to pass - to indicate no target
	if (strcmp(argv[index], "-") == 0)
	{
		target_buffer = NULL;
	}
	else
	{
		target_buffer = argv[index];
	}
	index++;

	name_buffer = argv[index];
	index++;

	// If we have no data payload to send, that's OK but don't parse it
	if (index < argc)
	{
		format_buffer = greio_array_to_event_data(&argv[index], argc - index, (void **)&data_buffer, &data_len);
		if (!format_buffer)
		{
			return NULL;
		}
	}

	buffer = gre_io_serialize(NULL, target_buffer, name_buffer, format_buffer, data_buffer, data_len);

	if (format_buffer)
	{
		free(format_buffer);
	}
	if (data_buffer)
	{
		free(data_buffer);
	}

	return buffer;
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
// sbio.send_event( string channel, string target, dict my_event )
//**********************************************
static PyObject *method_send_event(PyObject *self, PyObject *args)
{
	Py_ssize_t TupleSize = PyTuple_Size(args);
	PyObject *event_dict, *key;
	PyObject *event_channel, *event_target, *event_data, *data_fmt_key, *data_value;
	Py_ssize_t pos_event = 0;
	Py_ssize_t pos_data = 0;
	Py_ssize_t num_payload_vars = 0;

	gre_io_t *send_handle;
	gre_io_serialized_data_t *buffer;

	char *channel_name;
	char *event_target_name;
	char *event_name;
	char data_buffer[256];
	int index = 0;
	int argc = 0;
	char **argv = NULL;

	if (TupleSize != 3)
	{
		if (!PyErr_Occurred())
			PyErr_SetString(PyExc_TypeError, "Error! Bad number of arguments - Usage: sbio.send_event( string channel, string target, dict my_event )");
		return NULL;
	}

	event_channel = PyTuple_GetItem(args, index++);
	if (event_channel == NULL || PyUnicode_Check(event_channel) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "Error! Bad or missing string argument 'channel'. Usage: sbio.send_event( string channel, string target, dict my_event )");
		return NULL;
	}
	else
	{
		channel_name = PY_OBJECT_TOSTRING(event_channel); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
	}

	event_target = PyTuple_GetItem(args, index++);
	if (event_target == NULL || PyUnicode_Check(event_target) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "Error! Bad or missing string argument 'target'. Usage: sbio.send_event( string channel, string target, dict my_event )");
		return NULL;
	}
	else
	{
		event_target_name = PY_OBJECT_TOSTRING(event_target); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
	}

	event_dict = PyTuple_GetItem(args, index++);
	if (event_dict == NULL || !PyDict_Check(event_dict))
	{
		PyErr_SetString(PyExc_TypeError, "Error! Invalid argument -  dict my_event");
		return NULL;
	}

	/* there *should* only be one top level key-value dictionary entry in the event which
		is the name of the event and a dictionary containing key-value pairs
		though it is feasible to support multiple events in a dictionary to send in a sequence
		*/
	while (PyDict_Next(event_dict, &pos_event, &key, &event_data))
	{
		num_payload_vars = PyDict_Size(event_data);
		if (num_payload_vars < 0) // return value negative if empty or invalid
		{
			num_payload_vars = 0;
		}

		if (key == NULL || PyUnicode_Check(key) != 1)
		{
			PyErr_SetString(PyExc_TypeError, "Error! Bad or missing string argument 'event' in ductionary. Usage: sbio.send_event( string channel, dict my_event )");
			Py_DECREF(key);
			return NULL;
		}
		else
		{
			event_name = PY_OBJECT_TOSTRING(key); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
		}

		argc = 0;
		argv = (char **)malloc(sizeof(char *) * 2);
		argv[argc++] = strdup(event_target_name); // use a copy as we re-use this for each event
		argv[argc++] = event_name;

		/* do something with the values... */
		if (event_data == NULL || !PyDict_Check(event_data) || num_payload_vars == 0)
		{
			// No event payload = no problem!
		}
		else
		{
			pos_data = 0;
			// Reallocate array for the event payload again starting with event target and event name already in-place
			// where each parameter is 2 array entries - format and value
			argv = (char **)realloc(argv, sizeof(char *) * ((num_payload_vars * 2) + 2));
			if (argv == NULL)
			{
				PyErr_SetString(PyExc_TypeError, "Error! Reallocation of array for payload failed.)");
				return NULL;
			}

			while (PyDict_Next(event_data, &pos_data, &data_fmt_key, &data_value))
			{

				argv[argc] = PY_OBJECT_TOSTRING(data_fmt_key); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
				argc++;
				if (data_value != NULL)
				{
					if (PyFloat_Check(data_value))
					{
						sprintf(data_buffer, "%f", PY_OBJECT_TODOUBLE(data_value));
						argv[argc] = strdup(data_buffer); // Note: strdup uses malloc so requires freeing
					}
					else if (PyLong_Check(data_value))
					{
						sprintf(data_buffer, "%ld", PY_OBJECT_TOLONG(data_value));
						argv[argc] = strdup(data_buffer); // Note: strdup uses malloc so requires freeing
					}
					else if (PyUnicode_Check(data_value))
					{
						argv[argc] = PY_OBJECT_TOSTRING(data_value); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
					}
					else
					{
						argv[argc] = strdup("nil"); // Note: strdup uses malloc so requires freeing
					}
					argc++;
				}
			}
		}

		// serialize and send the event
		send_handle = gre_io_open(channel_name, GRE_IO_TYPE_WRONLY);
		if (send_handle != NULL)
		{
			buffer = create_event_from_array(argc, &argv[0]);

			if (buffer != NULL)
			{
				gre_io_send(send_handle, buffer);
				gre_io_free_buffer(buffer);
			}

			gre_io_close(send_handle);
			send_handle = NULL;
		}
		else
		{
			PyErr_Clear();
			PyErr_Format(PyExc_ValueError, "Can't open send handle [%s] error %d\n", channel_name, errno);
		}

		// clean-up dynamic argv structure for the event
		if (argv != NULL)
		{
			for (index = 0; index < argc; index++)
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

	PyErr_Clear();
	return PyLong_FromLong(0);
}

//**********************************************
// A blocking call which wait on an event listening to the named channel
// returning the event and any payload as a dictionary
//
// The channel handle will be cached once opened until explicity closed by a  call to
//
// dict event = sbio.receive_event( string channel )
//**********************************************
typedef struct receive_handle
{
	char *channel_name;
	gre_io_t *handle;
	struct receive_handle *next_handle;
} receive_handle_t;

receive_handle_t *first_recv_handle = NULL;

static gre_io_t *get_receive_handle(char *channel)
{
	receive_handle_t *next = first_recv_handle;
	receive_handle_t *previous = NULL;
	while (next != NULL)
	{
		if (strcmp(next->channel_name, channel) == 0)
		{
			return next->handle;
		}
		previous = next;
		next = next->next_handle;
	}

	// channel isn't already open, so open it
	gre_io_t *handle = gre_io_open(channel, GRE_IO_TYPE_RDONLY);
	if (handle == NULL)
	{
		return NULL;
	}
	else
	{
		next = malloc(sizeof(receive_handle_t));
		next->channel_name = malloc(strlen(channel) + 1);
		strcpy(next->channel_name, channel);
		next->handle = handle;
		next->next_handle = NULL;
		if (previous == NULL)
		{
			first_recv_handle = next;
		}
		else
		{
			previous->next_handle = next;
		}
		return handle;
	}
}

static void close_receive_handle(char *channel)
{
	receive_handle_t *next = first_recv_handle;
	receive_handle_t *previous = NULL;
	while (next != NULL)
	{
		if (strcmp(next->channel_name, channel) == 0)
		{
			gre_io_close(next->handle);
			if (previous == NULL)
			{
				first_recv_handle = next->next_handle;
			}
			else
			{
				previous->next_handle = next->next_handle;
			}
			free(next->channel_name);
			free(next);
		}
		previous = next;
		next = next->next_handle;
	}
}

static void close_all_receive_handles()
{
	receive_handle_t *current = first_recv_handle;
	receive_handle_t *next = NULL;
	while (current != NULL)
	{
		next = current->next_handle;
		if (current->handle != NULL)
		{
			gre_io_close(current->handle);
		}
		if (current->channel_name != NULL)
		{
			free(current->channel_name);
		}
		free(current);
		current = next;
	}
}

static PyObject *method_receive_event(PyObject *self, PyObject *args)
{
	Py_ssize_t TupleSize = PyTuple_Size(args);
	PyObject *event_dict, *payload_dict, *event;
	PyObject *event_channel;

	gre_io_serialized_data_t *buffer = NULL;

	char *channel_name;
	char *event_target_name;
	char *event_name;
	char *event_format;
	char *event_payload;
	uint8_t *event_data;
	int index = 0;
	int ret;
	int nbytes;

	if (TupleSize != 1)
	{
		if (!PyErr_Occurred())
			PyErr_SetString(PyExc_TypeError, "Error! Bad number of arguments - Usage: sbio.receive_event( string channel )");
		return NULL;
	}

	event_channel = PyTuple_GetItem(args, index++);
	if (event_channel == NULL || PyUnicode_Check(event_channel) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "Error! Bad or missing string argument 'channel'. Usage: sbio.receive_event( string channel )");
		return NULL;
	}
	else
	{
		channel_name = PY_OBJECT_TOSTRING(event_channel); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
	}

	gre_io_t *recv_handle = get_receive_handle(channel_name);
	free(channel_name);

	if (recv_handle == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "Error! Can't open IO channel");
		return NULL;
	}

	// Initialise our return object, a Python dictionary containing an event
	event_dict = PyDict_New();
	Py_XINCREF(event_dict);

	Py_BEGIN_ALLOW_THREADS
	ret = gre_io_receive(recv_handle, &buffer);
	Py_END_ALLOW_THREADS
	
	if (ret < 0)
	{
		PyErr_SetString(PyExc_TypeError, "Error! Problem receiving data, exiting");
		return NULL;
	}
	else
	{
		nbytes = gre_io_unserialize(buffer,
									&event_target_name,
									&event_name,
									&event_format,
									(void **)&event_data);

		event_payload = greio_event_data_to_string(event_format, event_data, nbytes);
		if (event_payload != NULL)
		{
			event = Py_BuildValue("s", event_name);

			// Populate the payload dictionary from the payload key value pairs
			PyObject *key;
			PyObject *value;
			const char *p = event_payload;
			char *buffer = NULL;

			size_t i = 0; // count of key value pairs in payload

			payload_dict = PyDict_New();
			while (*p)
			{
				// parse key
				while (isspace((unsigned char)*p))
					++p;
				if (*p)
				{
					const char *q = p;
					char *fmt = NULL;
					// parse key and format specifier
					while (*p && !isspace((unsigned char)*p))
						++p;

					size_t length = p - q;
					buffer = calloc(length + 1, sizeof(char));
					strncpy(buffer, q, length);
					key = Py_BuildValue("s", buffer);

					// identify value parameter type from format specifier
					length = strcspn(buffer, ":");
					fmt = calloc(length + 1, sizeof(char));
					strncpy(fmt, buffer, length);
					free(buffer);

					// parse value, skip to payload start
					while (isspace((unsigned char)*p))
						++p;
					if (*p)
					{
						const char *q = p;
						size_t length = 0;

						if (strcmp(fmt, "1s0") == 0) // it is a string so value delimited by ".."
						{
							q++; // skip initial quotes to first string character
							while (*p++ && (*p != '"'))
								length++; // count characters to trailing quotes

							buffer = calloc(length + 1, sizeof(char));
							strncpy(buffer, q, length);
							value = Py_BuildValue("s", buffer);
							free(buffer);
						}
						else // it is a numeric value
						{
							while (*p && !isspace((unsigned char)*p))
								++p; // skip to end of value marked by whitespace
							length = p - q;

							buffer = calloc(length + 1, sizeof(char));
							strncpy(buffer, q, length);
							// convert float messages to Python float
							if (strcmp(fmt, "4f1") == 0)
							{
								value = Py_BuildValue("d", strtod(buffer, NULL));
								// all other numeric values will be converted to Python integer
							}
							else if (fmt[1] == 'u')
							{
								value = Py_BuildValue("K", strtoull(buffer, NULL, 10));
							}
							else
							{
								value = Py_BuildValue("L", strtoll(buffer, NULL, 0));
							}
							free(buffer);
						}
						++i;
						PyDict_SetItem(payload_dict, key, value);
					}
					free(fmt);
				}
			}
			PyDict_SetItem(event_dict, event, payload_dict);
			Py_XINCREF(payload_dict);
			Py_XINCREF(event);
		}
		else
		{
			payload_dict = Py_None;
			event = Py_BuildValue("s", event_name);
			PyDict_SetItem(event_dict, event, payload_dict);
			Py_XINCREF(payload_dict);
			Py_XINCREF(event);
		}
	}

	PyErr_Clear();
	return event_dict;
}

//**********************************************
// The receive channel will be closed and handle released
//
// dict event = sbio.method_close_receive_channel( string channel )
//**********************************************

static PyObject *method_close_receive_channel(PyObject *self, PyObject *args)
{
	Py_ssize_t TupleSize = PyTuple_Size(args);
	PyObject *event_channel;
	int index = 0;

	char *channel_name;

	if (TupleSize != 1)
	{
		PyErr_SetString(PyExc_TypeError, "Error! Bad number of arguments - Usage: sbio.close_receive_channel( string channel )");
		return NULL;
	}

	event_channel = PyTuple_GetItem(args, index++);
	if (event_channel == NULL || PyUnicode_Check(event_channel) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "Error! Bad or missing string argument 'channel'. Usage: sbio.close_receive_channel( string channel )");
		return NULL;
	}
	else
	{
		channel_name = PY_OBJECT_TOSTRING(event_channel); // Note: PY_OBJECT_TOSTRING uses strdup which uses malloc so requires freeing
	}
	Py_DECREF(event_channel);

	close_receive_handle(channel_name);

	// finished with channel and target so release to clean-up
	free(channel_name);

	PyErr_Clear();
	return PyLong_FromLong(0);
}

//**********************************************
// Python extension boilerplate
//**********************************************
struct module_state
{
	PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state *)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

static PyObject *
error_out(PyObject *m)
{
	struct module_state *st = GETSTATE(m);
	PyErr_SetString(st->error, "sbio - something bad happened");
	return NULL;
}

static PyMethodDef sbio_methods[] = {
	{"send_event", (PyCFunction)method_send_event, METH_VARARGS, "Python interface to send a Stoyboard event using gre_io_send syntax, with or without a payload"},
	{"receive_event", (PyCFunction)method_receive_event, METH_VARARGS, "Python interface which blocks waiting for a Stoyboard event on a channel, returning a dictionary with payload.  The channel is cached once opened"},
	{"close_receive_channel", (PyCFunction)method_close_receive_channel, METH_VARARGS, "Python interface which will explicitly close a previously opened receive channel"},
	{"error_out", (PyCFunction)error_out, METH_NOARGS, NULL},
	{NULL, NULL, 0, NULL}};

static int sbio_traverse(PyObject *m, visitproc visit, void *arg)
{
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}

static int sbio_clear(PyObject *m)
{
	Py_CLEAR(GETSTATE(m)->error);
	return 0;
}

static void sbio_free()
{
	close_all_receive_handles();
}

static struct PyModuleDef sbio_module = {
	PyModuleDef_HEAD_INIT,
	"sbio",
	"Python interface for Crank Storyboard SBIO C library",
	sizeof(struct module_state),
	sbio_methods,
	NULL,
	sbio_traverse,
	sbio_clear,
	sbio_free};

#define INITERROR return NULL

PyMODINIT_FUNC
PyInit_sbio(void)
{
	PyObject *module = PyModule_Create(&sbio_module);

	if (module == NULL)
		INITERROR;

	struct module_state *st = GETSTATE(module);

	st->error = PyErr_NewException("sbio.Error", NULL, NULL);
	if (st->error == NULL)
	{
		Py_DECREF(module);
		INITERROR;
	}
	return module;
}
