#!/usr/bin/env python3
import sundtek_api

try:
    devices = sundtek_api.local_devices()
except (ConnectionError):
    pass
else:
    print("Local Devices:")
    for device in devices.values():
        if device.get('remote_node'):
            device.update(sundtek_api.ir_protocols(device.get('frontend_node')))
    print(devices)

print("Network Devices:")
print(sundtek_api.network_devices())
