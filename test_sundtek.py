#!/usr/bin/env python3
import sundtek

try:
    devices = sundtek.local_devices()
except (ConnectionError):
    pass
else:
    print("Local Devices:")
    print(devices)

print("Network Devices:")
print(sundtek.network_devices())
