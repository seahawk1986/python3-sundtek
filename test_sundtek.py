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

    for device, data in devices.items():
        print("check device", device, data)
        if data.get('frontend_node') and data.get('remote_node'):
            print(f"set protocol for {data.get('frontend_node')} to NEC")
            sundtek_api.set_ir_protocol(data.get('frontend_node'), sundtek_api.IR_PROTO_NEC)

print("Network Devices:")
print(sundtek_api.network_devices())
