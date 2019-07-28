#!/usr/bin/env python3
import sys
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
        if data.get('frontend_node') and data.get('remote_node'):
            for ir_protocol, description in data.get('ir_protocols').items():
                if not "hardware decoder" in description:
                    print(f"trying set protocol for {data.get('frontend_node')} to {ir_protocol}")
                    try:
                        sundtek_api.set_ir_protocol(data.get('frontend_node'), ir_protocol)
                    except ValueError as e:
                        print(e, file=sys.stderr)
                        print("More recent sundtek devices only support a NEC hardware decoder.")
                    else:
                        if not sundtek_api.ir_protocols(data.get('frontend_node')).get('active_ir_protocol') == ir_protocol:
                            print(f"Could not set protocol to {description} ({ir_protocol})", file=sys.stderr)

print("Network Devices:")
print(sundtek_api.network_devices())
