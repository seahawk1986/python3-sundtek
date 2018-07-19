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

// Actual module method definition - this is the code that will be called by
// sundtek.local_devices() in python
static PyObject *sundtek_local_devices(PyObject *self, PyObject *args)
{
   int fd = connect_sundtek_mediasrv();
   if (fd <= 0) {
      PyErr_SetString(PyExc_ConnectionError, "connecting to mediasrv failed");
      return (PyObject *) NULL;
   }

   PyObject *local_devices = PyDict_New();
   local_device_scan(fd, local_devices);

   net_close(fd); // close the connection to the local mediasrv

   return local_devices;
}

// Actual module method definition - this is the code that will be called by
// sundtek.network_devices() in python
static PyObject *sundtek_network_devices(PyObject *self, PyObject *args) {

   // scan for all devices
   void *fd = media_scan_network(ALLOCATE_DEVICE_OBJECT, NETWORK_SCAN_TIME);
   PyObject *network_devices = PyDict_New();
   network_device_scan(fd, network_devices);
   media_scan_free(&fd);
   return network_devices;
}

static PyObject *sundtek_enable_network(PyObject *self, PyObject *args) {
   int fd = connect_sundtek_mediasrv();
   if (fd <= 0) {
      PyErr_SetString(PyExc_ConnectionError, "connecting to mediasrv failed");
      return (PyObject *) NULL;
   }

   net_enablenetwork(1);
   net_close(fd);
   Py_RETURN_NONE;
}

static PyObject *sundtek_disable_network(PyObject *self, PyObject *args) {
   int fd = connect_sundtek_mediasrv();
   if (fd <= 0) {
      PyErr_SetString(PyExc_ConnectionError, "connecting to mediasrv failed");
      return (PyObject *) NULL;
   }

   net_enablenetwork(0);
   net_close(fd);
   Py_RETURN_NONE;
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
	"returns available sundtek devices (either connected locally or mounted) using the mcsimple api."
    },
    {   "network_devices",
	sundtek_network_devices,
	METH_NOARGS,
	"returns all network devices announced by other mediasrv instances"
    },
    {
        "enable_network",
	sundtek_enable_network,
	METH_NOARGS,
	"enable network sharing of local sundtek devices"
    },
    {
        "disable_network",
	sundtek_disable_network,
	METH_NOARGS,
	"disable network sharing of local sundtek devices"
    },
    /*
    {   "is_local_device",
	sundtek_is_local_device,
	METH_VARARGS,
	"accepts either a device id (int) or a serial (str), returns True if a matching device is physically connected."
    },
    */
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

int connect_sundtek_mediasrv(void) {
   /*
    * obtains a connection to the sundtek mediasrv and returns a file desriptor
    * if the connections fails fd is <= 0. You must close the connection
    * by calling net_close(fd)
    */

   // the sundtek libraries write error messages to stdout
   // so we redirect it to stderr...
   int old_stdout = dup(STDOUT_FILENO);
   dup2(STDERR_FILENO, STDOUT_FILENO);

   // obtain a connection to mediasrv
   int fd = net_connect(0);

   // switch back to "normal" stdout for dumping our JSON data after flushing remaining data
   fflush(stdout);
   dup2(old_stdout, STDOUT_FILENO);

   return fd;
}

int is_local_device(const char *serial) {
   struct media_device_enum *device;
   int device_index = 0;
   int subdevice = 0;
   int fd = connect_sundtek_mediasrv();
   if (fd <= 0) // if the local mediasrv is not reachable it can't be a local device
      return 0;
   while((device=net_device_enum(fd, &device_index, subdevice))!=0) {   // multi frontend support???
      do {
	 if ((strcmp((const char*) device->serial, serial) == 0) &&
             ((device->capabilities & MEDIA_REMOTE_DEVICE) == 0)) {
	    net_close(fd);
	    return 1;
	 }
	 free(device);
	 device = NULL;
      } while((device=net_device_enum(fd, &device_index, ++subdevice))!=0);
      subdevice = 0;
      device_index++;
   }
   net_close(fd);
   return 0;
}

void local_device_scan(int fd, PyObject *local_devices) {
   /*
    * use a given fd to communicate with mediasrv.
    * iterate over all available devices (mounted or connected locally),
    * call device2dict to add their data to the dictionary* local_devices passed to the function.
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

void network_device_scan(void *fd, PyObject *network_devices) {
   char *id = NULL;
   char *ip = NULL;
   char *name = NULL;
   char *serial = NULL;
   uint32_t *cap = NULL;
   uint32_t net_cap;

   int n = 0;
   while (media_scan_info(fd, n, "ip", (void**) &ip) == 0) {
      media_scan_info(fd, n, "serial",       (void**) &serial);
      if (!is_local_device(serial)) {
	  media_scan_info(fd, n, "capabilities", (void**) &cap);
	  media_scan_info(fd, n, "devicename",   (void**) &name);
	  media_scan_info(fd, n, "id",           (void**) &id);

	  PyObject *capabilities = PyDict_New();

	  net_cap = *cap | (uint32_t) MEDIA_REMOTE_DEVICE; // mark all devices as network devices
	  capabilities2dict(net_cap, capabilities);

	  PyObject *sundtek_device = PyDict_New();
	  PyDict_SetItemString(sundtek_device, "capabilities", capabilities);
	  PyDict_SetItemString(sundtek_device, "devicename",   Py_BuildValue("s", name));
	  PyDict_SetItemString(sundtek_device, "id",           Py_BuildValue("i", (atoi(id))));
	  PyDict_SetItemString(sundtek_device, "ip",           Py_BuildValue("s", ip));
	  PyDict_SetItemString(sundtek_device, "serial",       Py_BuildValue("s", serial));

	  PyDict_SetItemString(network_devices, serial, sundtek_device);
      }
      n++;
   }
}

void capabilities2dict(const uint32_t cap, PyObject *capabilities) {
   PyDict_SetItemString(capabilities, "analog_tv",        PyBool_FromLong(cap & MEDIA_ANALOG_TV));
   PyDict_SetItemString(capabilities, "atsc",             PyBool_FromLong(cap & MEDIA_ATSC));
   PyDict_SetItemString(capabilities, "bitmask",          PyLong_FromUnsignedLong(cap)); // raw bitmask
   PyDict_SetItemString(capabilities, "digitalca",        PyBool_FromLong(cap & MEDIA_DIGITAL_CA));
   PyDict_SetItemString(capabilities, "digitalci",        PyBool_FromLong(cap & MEDIA_DIGITAL_CI));
   PyDict_SetItemString(capabilities, "dvbc",             PyBool_FromLong(cap & MEDIA_DVBC));
   PyDict_SetItemString(capabilities, "dvbc2",            PyBool_FromLong(cap & MEDIA_DVBC2));
   PyDict_SetItemString(capabilities, "dvbs2",            PyBool_FromLong(cap & MEDIA_DVBS2));
   PyDict_SetItemString(capabilities, "dvbt",             PyBool_FromLong(cap & MEDIA_DVBT));
   PyDict_SetItemString(capabilities, "dvbt2",            PyBool_FromLong(cap & MEDIA_DVBT2));
   PyDict_SetItemString(capabilities, "initial_dvb_mode", PyBool_FromLong(cap & MEDIA_DIGITAL_TV));
   PyDict_SetItemString(capabilities, "network_device",   PyBool_FromLong(cap & MEDIA_REMOTE_DEVICE));
   PyDict_SetItemString(capabilities, "radio",            PyBool_FromLong(cap & MEDIA_RADIO));
   PyDict_SetItemString(capabilities, "remote",           PyBool_FromLong(cap & MEDIA_REMOTE));
}

void device2dict(struct media_device_enum *device, PyObject *local_devices) {
   /*
    * Take a media_device_enum and add the data transformed into python dicts
    * to the local_devices object.
    */

   PyObject *capabilities = PyDict_New();
   capabilities2dict(device->capabilities, capabilities);

   PyObject *sundtek_device = PyDict_New();
   PyDict_SetItemString(sundtek_device, "capabilities",  capabilities);
   PyDict_SetItemString(sundtek_device, "devicename",    Py_BuildValue("s", (char*)device->devicename));
   PyDict_SetItemString(sundtek_device, "id",            Py_BuildValue("i", device->id));
   PyDict_SetItemString(sundtek_device, "remote_device", Py_BuildValue("s", (char*)device->remote_node));
   PyDict_SetItemString(sundtek_device, "serial",        Py_BuildValue("s", (char*)device->serial));
   PyDict_SetItemString(sundtek_device, "ir_protocols",  get_ir_protocols((char*)device->frontend_node));

   PyDict_SetItemString(local_devices, (char*)device->serial, sundtek_device);
}

static PyObject *get_ir_protocols(char *frontend_path) {
	/*
	 * Takes a path to a dvb frontend (e.g. /dev/dvb/adapter0/frontend0) and
	 * returns an array of supported IR protocols
	 * As far as I understand there are two possibilites:
	 *  - older sticks with a software decoder support NEC, RC5, RC6 and RC6A
	 *  - newer sticks only support their NEC hardware devoder
	 *
	 *  So if the ioctl operation for DEVICE_ENUM_IR fails, we assume that we got a newer stick
	 *  else we return full protocol support
	 */

	PyObject *ir_protocols;
	int fd = net_open(frontend_path, O_RDONLY);
	struct media_ir_enum ir_enum[0]; // for some reason the sundtek api wants an array with one element
	int response = net_ioctl(fd, DEVICE_ENUM_IR, &ir_enum);
	if (response == 0) {
	   ir_protocols = Py_BuildValue("(ssss)", "NEC", "RC5", "RC6", "RC6A");
        } else {
	   ir_protocols = Py_BuildValue("(s)", "NEC");
	}

	net_close(fd);
	return ir_protocols;
}
