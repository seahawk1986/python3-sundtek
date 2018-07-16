/*
 * Based on the excellent example by adamlamers (http://adamlamers.com/post/NUBSPFQJ50J1)
 */
#include <Python.h>
#include <stdio.h>
#include <unistd.h>

#include <sundtek/mcsimple.h>
#include <sundtek/mediacmds.h>
#include <sundtek/mediaclient.h>

#include "sundtek.h"

// The following definitions are needed for sundtek api calls:
#define ALLOCATE_DEVICE_OBJECT 1   // allocate device objects when running media_scan_network
#define NETWORK_SCAN_TIME 700      // Why 700 ms? It seems to work...

//Actual module method definition - this is the code that will be called by
//sundtek.local_devices
static PyObject* sundtek_local_devices(PyObject *self, PyObject *args)
{
   // the sundtek libraries write error messages to stdout
   // so we redirect it to stderr...
   int old_stdout = dup(STDOUT_FILENO);
   dup2(STDERR_FILENO, STDOUT_FILENO);

   int fd = net_connect(0);

   // switch back to "normal" stdout for dumping our JSON data after flushing remaining data
   fflush(stdout);
   dup2(old_stdout, STDOUT_FILENO);

   if (fd <= 0) {
      PyErr_SetString(PyExc_ConnectionError, "connection to mediasrv failed");
      return (PyObject *) NULL;
   }

   PyObject * local_devices = PyDict_New();
   local_device_scan(fd, local_devices);

   return local_devices;
}
static PyObject* sundtek_network_devices(PyObject *self, PyObject *args) {
   PyObject* network_devices = PyDict_New();
   network_device_scan(network_devices);
   return network_devices;
}

//Method definition object for this extension, these argumens mean:
//ml_name: The name of the method
//ml_meth: Function pointer to the method implementation
//ml_flags: Flags indicating special features of this method, such as
//          accepting arguments, accepting keyword arguments, being a
//          class method, or being a static method of a class.
//ml_doc:  Contents of this method's docstring
static PyMethodDef sundtek_methods[] = { 
    {   
        "local_devices",
        sundtek_local_devices,
        METH_NOARGS,
        "Returns available sundtek devices (either local or mounted) using the mcsimple api."
    },  
    {   "network_devices",
	sundtek_network_devices,
	METH_NOARGS,
	"Returns all network devices announced by other mediasrv instances"
    },
    {NULL, NULL, 0, NULL}
};

//Module definition
//The arguments of this structure tell Python what to call your extension,
//what it's methods are and where to look for it's method definitions
static struct PyModuleDef sundtek_definition = { 
    PyModuleDef_HEAD_INIT,
    "sundtek",
    "A Python module that calls the sundtek API from C code.",
    -1, 
    sundtek_methods
};

//Module initialization
//Python calls this function when importing your extension. It is important
//that this function is named PyInit_[[your_module_name]] exactly, and matches
//the name keyword argument in setup.py's setup() call.
PyMODINIT_FUNC PyInit_sundtek(void)
{
    Py_Initialize();

    return PyModule_Create(&sundtek_definition);
}

// helper functions (actual implementation of the c api calls)

void local_device_scan(int fd, PyObject* local_devices) {
   /*
    * Use a given fd to communicate with mediasrv.
    * Iterate over all available devices (mounted or local),
    * call device2dict to add their data to the dictionary
    * "local_devices" passed to the function.
    */

   struct media_device_enum *device;
   int device_index = 0;
   int subdevice = 0;
   while((device=net_device_enum(fd, &device_index, subdevice))!=0) {   // multi frontend support???
      do {
         device2dict(device, local_devices);
         free(device);
         device = NULL;
      } while((device=net_device_enum(fd, &device_index, ++subdevice))!=0);
      subdevice = 0;
      device_index++;
   }
}

void network_device_scan(PyObject *network_devices) {
   void *obj;
   char *id = NULL;
   char *ip = NULL;
   char *name = NULL;
   char *serial = NULL;
   uint32_t *cap;

   // scan for all devices
   obj = media_scan_network(ALLOCATE_DEVICE_OBJECT, NETWORK_SCAN_TIME);

   int n = 0;
   while (media_scan_info(obj, n, "ip", (void**) &ip) == 0) {
      media_scan_info(obj, n, "id", (void**) &id);

      media_scan_info(obj, n, "serial", (void**) &serial);
      media_scan_info(obj, n, "devicename", (void**) &name);
      media_scan_info(obj, n, "capabilities", (void**) &cap);
      PyObject* capabilities = PyDict_New();
      capabilities2dict(*cap, capabilities);

      PyObject *sundtek_device = PyDict_New();
      PyDict_SetItemString(sundtek_device, "capabilities", capabilities);
      PyDict_SetItemString(sundtek_device, "devicename", Py_BuildValue("s", name));
      PyDict_SetItemString(sundtek_device, "id", Py_BuildValue("i", (atoi(id))));
      PyDict_SetItemString(sundtek_device, "ip", Py_BuildValue("s", ip));
      PyDict_SetItemString(sundtek_device, "serial", Py_BuildValue("s", serial));

      PyDict_SetItemString(network_devices, serial, sundtek_device);
      n++;
   }
   media_scan_free(&obj);
}

void capabilities2dict(const uint32_t cap, PyObject* capabilities) {
   PyDict_SetItemString(capabilities, "analog_tv", PyBool_FromLong(cap & MEDIA_ANALOG_TV));
   PyDict_SetItemString(capabilities, "atsc",      PyBool_FromLong(cap & MEDIA_ATSC));
   PyDict_SetItemString(capabilities, "bitmask",   PyLong_FromUnsignedLong(cap)); // raw bitmask
   PyDict_SetItemString(capabilities, "digitalca", PyBool_FromLong(cap & MEDIA_DIGITAL_CA));
   PyDict_SetItemString(capabilities, "digitalci", PyBool_FromLong(cap & MEDIA_DIGITAL_CI));
   PyDict_SetItemString(capabilities, "dvbc",      PyBool_FromLong(cap & MEDIA_DVBC));
   PyDict_SetItemString(capabilities, "dvbc2",     PyBool_FromLong(cap & MEDIA_DVBC2));
   PyDict_SetItemString(capabilities, "dvbs2",     PyBool_FromLong(cap & MEDIA_DVBS2));
   PyDict_SetItemString(capabilities, "dvbt",      PyBool_FromLong(cap & MEDIA_DVBT));
   PyDict_SetItemString(capabilities, "dvbt2",     PyBool_FromLong(cap & MEDIA_DVBT2));
   PyDict_SetItemString(capabilities, "radio",     PyBool_FromLong(cap & MEDIA_RADIO));
   PyDict_SetItemString(capabilities, "remote",    PyBool_FromLong(cap & MEDIA_REMOTE));
   PyDict_SetItemString(capabilities, "is_network_device", PyBool_FromLong(cap & MEDIA_REMOTE_DEVICE));
   PyDict_SetItemString(capabilities, "supports_initial_dvb_mode", PyBool_FromLong(cap & MEDIA_DIGITAL_TV));
}

void device2dict(struct media_device_enum *device, PyObject* local_devices) {
   /*
    * Take a media_device_enum and add the data transformed into python dicts
    * to the local_devices object.
    */

   PyObject* capabilities = PyDict_New(); 
   PyObject* sundtek_device = PyDict_New(); 

   capabilities2dict(device->capabilities, capabilities);
   PyDict_SetItemString(sundtek_device, "capabilities",  capabilities);
   PyDict_SetItemString(sundtek_device, "devicename", Py_BuildValue("s", (char*)device->devicename));
   PyDict_SetItemString(sundtek_device, "id",            Py_BuildValue("i", device->id));
   PyDict_SetItemString(sundtek_device, "serial",        Py_BuildValue("s", (char*)device->serial));
   PyDict_SetItemString(sundtek_device, "remote_device", Py_BuildValue("s", (char*)device->remote_node));
   
   PyDict_SetItemString(local_devices, (char*)device->serial, sundtek_device);
}
