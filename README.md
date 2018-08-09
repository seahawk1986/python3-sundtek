# sundtek_api Module
`python3-sundtek` provides a C-extension for the sundtek api.

At the moment the following functions have been implemented:
```python
import sundtek_api

# list local devices - returns a dict of devices connected locally
sundtek_api.local_devices()

# list network devices - returns a dict of devices announced by other mediasrv instances
sundtek_api.network_device()

# enable network support
sundtek_api.enable_network()

# disable network support
sundtek_api.disable_network()

# mount a remote sundtek device with given IP and device ID
sundtek_api.mount("192.168.1.104:0")

# umount a remote sundtek device with given IP and device ID
sundtek_api.umount("192.168.1.104:0")

# get the active and supported ir protocols from a frontend node
sundtek_api.ir_protocols("/dev/dvb/adapter0/frontend0")
```
