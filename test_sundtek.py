#!/usr/bin/env python3
import sundtek_api

try:
    devices = sundtek_api.local_devices()
except (ConnectionError):
    pass
else:
    print("Local Devices:")
    print(devices)

print("Network Devices:")
print(sundtek_api.network_devices())
