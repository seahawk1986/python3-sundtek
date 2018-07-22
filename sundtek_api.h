#ifndef sundtek_python_wrapper_H
#define sundtek_python_wrapper_H

static PyObject *sundtek_api_local_devices(PyObject *self, PyObject *args);
static PyObject *sundtek_api_network_devices(PyObject *self, PyObject *args);
int connect_sundtek_mediasrv(void);
int device_type(const char *serial);
int only_NEC_support(char *frontend_path);
void local_device_scan(int fd, PyObject *local_devices);
void network_device_scan(void *fd, PyObject *network_devices);
void device2dict(struct media_device_enum *device, PyObject *local_devices);
void capabilities2dict(const uint32_t cap, PyObject *capabilities);
#endif /* sundtek_python_wrapper_H */
