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
            for ir_protocol in data.get('ir_protocols').keys():
                print(f"set protocol for {data.get('frontend_node')} to {ir_protocol}")
                sundtek_api.set_ir_protocol(data.get('frontend_node'), ir_protocol)
                assert sundtek_api.ir_protocols(data.get('frontend_node')).get('active_ir_protocol') == ir_protocol

print("Network Devices:")
print(sundtek_api.network_devices())
