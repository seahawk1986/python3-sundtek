import sundtek_api
import sys

class SundtekDevice:
    """
    Class representation of sundtek devices.

    Takes the values of an element of the dict returned by either sundtek_api.local_devices or
    sundtek_api.network_devices as initialisation values, e.g.:

    >>> import sundtek_api
    >>> from sundtekdevice import SundtekDevice
    >>> devices = sundtek_api.local_devices()
    >>> for d in devices.values():
    ...     device = SundtekDevice(**d)
    ... 
    >>> for d in sundtek_api.network_devices().values():
    ...     device = SundtekDevice(**d)
    ... 
    """
    def __init__(self, capabilities=None, devicename=None, id=None,
            frontend_node=None, remote_node=None, serial=None, ip=None, mounted=False):
        if capabilities is None:
            self.capabilities = {
                    'analog_tv': False,
                    'atsc': False,
                    'bitmask': None, 
                    'digitalca': False, 
                    'digitalci': False, 
                    'dvbc': False, 
                    'dvbc2': False, 
                    'dvbs2': False, 
                    'dvbt': False,
                    'dvbt2': False,
                    'initial_dvb_mode': {},
                    'network_device': False, 
                    'radio': False,
                    'remote': False,
                    }
        else:
            self.capabilities = capabilities
            
        if devicename is None:
            self.devicename = "UNKNOWN"
        else:
            self.devicename = devicename

        self.id = id
        self.ip = ip
        self.mounted = mounted
        self.frontend_node = frontend_node
        self.remote_node = remote_node
        self.serial = serial
        if frontend_node is not None and remote_node is not None:
            ir_capabilites = sundtek_api.ir_protocols(frontend_node)
            self._active_ir_protocol = ir_capabilites.get('active_ir_protocol')
            self.ir_protocols = ir_capabilites.get('ir_protocols', {})
        else:
            self._active_ir_protocol = None
            self.ir_protocols = {}

    def __repr__(self):
        if self.ip is None:
            return f"<Local Device {self.id} ({self.serial})>"
        else:
            return f"<Remote Device {self.ip}:{self.id} ({self.serial})>"

    @property
    def ir_protocol(self):
        """
        Returns a tuple if the currently active ir protocol and a protocol description.
        """
        data = sundtek_api.ir_protocols(self.frontend_node)
        self._active_ir_protocol = data.get('active_ir_protocol')
        current_ir_protocol_desc = data.get('ir_protocols', {}).get(self._active_ir_protocol)
        return (self._active_ir_protocol, current_ir_protocol_desc)
    
    @ir_protocol.setter
    def ir_protocol(self, value: int) -> None:
        """
        sets the active ir protocol to the given integer value.

        You can use the variables defined in sundtek_api:
        sundtek_api.IR_PROTO_NEC
        sundtek_api.IR_PROTO_RC5
        sundtek_api.IR_PROTO_RC6_MODE0
        sundtek_api.IR_PROTO_RC6_MODE6A
        
        sundtek_api.set_ir_protocol raises a ValueError if an invalid value for
        the ir protocol is given. Please note, that devices
        with a hardware-decoder are limited to NEC and ignore all attempts
        to change the protocol without reporting errors. This method rises an
        NotImplemented Exception if changing the ir protocol had no effect.
        """
        self._active_ir_protocol = sundtek_api.ir_protocols(self.frontend_node).get('active_ir_protocol')
        sundtek_api.set_ir_protocol(self.remote_node, value)
        active_ir_protocol, _ = self.ir_protocol
        if value != active_ir_protocol:
            raise NotImplemented("This device is unable to change the ir protocol")


def local_sundtek_devices():
    sundtek_devices = []
    try:
        devices = sundtek_api.local_devices()
    except ConnectionError:
        return sundtek_devices
    for device_data in devices.values():
        device = SundtekDevice(**device_data)
        sundtek_devices.append(device)
    return sundtek_devices


def network_sundtek_devices():
    devices = sundtek_api.network_devices()
    sundtek_devices = []
    for device_data in devices.values():
        device = SundtekDevice(**device_data)
        sundtek_devices.append(device)
    return sundtek_devices


