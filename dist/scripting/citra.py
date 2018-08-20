import zmq
import struct
import random
import binascii

CURRENT_REQUEST_VERSION = 1
MAX_REQUEST_DATA_SIZE = 32

REQUEST_TYPE_READ_MEMORY = 1
REQUEST_TYPE_WRITE_MEMORY = 2
REQUEST_TYPE_PAD_STATE = 3
REQUEST_TYPE_TOUCH_STATE = 4
REQUEST_TYPE_MOTION_STATE = 5
REQUEST_TYPE_CIRCLE_STATE = 6
REQUEST_TYPE_SET_RESOLUTION = 7

CITRA_PORT = "45987"

# https://github.com/smealum/ctrulib/blob/bd34fd59dbf0691e2dba76be65f260303d8ccec7/libctru/include/3ds/services/hid.h#L9
def BIT(n):
    return (1 << n)

KEY_A = BIT(0)             # A
KEY_B = BIT(1)             # B
KEY_SELECT = BIT(2)        # Select
KEY_START = BIT(3)         # Start
KEY_DRIGHT = BIT(4)        # D-Pad Right
KEY_DLEFT = BIT(5)         # D-Pad Left
KEY_DUP = BIT(6)           # D-Pad Up
KEY_DDOWN = BIT(7)         # D-Pad Down
KEY_R = BIT(8)             # R
KEY_L = BIT(9)             # L
KEY_X = BIT(10)            # X
KEY_Y = BIT(11)            # Y
KEY_CPAD_RIGHT = BIT(28)   # Circle Pad Right
KEY_CPAD_LEFT = BIT(29)    # Circle Pad Left
KEY_CPAD_UP = BIT(30)      # Circle Pad Up
KEY_CPAD_DOWN = BIT(31)    # Circle Pad Down

class Citra:
    def __init__(self, address="127.0.0.1", port=CITRA_PORT):
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect("tcp://" + address + ":" + port)

    def is_connected(self):
        return self.socket is not None

    def _generate_header(self, request_type, data_size):
        request_id = random.getrandbits(32)
        return (struct.pack("IIII", CURRENT_REQUEST_VERSION, request_id, request_type, data_size), request_id)

    def _read_and_validate_header(self, raw_reply, expected_id, expected_type):
        reply_version, reply_id, reply_type, reply_data_size = struct.unpack("IIII", raw_reply[:4*4])
        if (CURRENT_REQUEST_VERSION == reply_version and
            expected_id == reply_id and
            expected_type == reply_type and
            reply_data_size == len(raw_reply[4*4:])):
            return raw_reply[4*4:]
        return None

    def read_memory(self, read_address, read_size):
        """
        >>> c.read_memory(0x100000, 4)
        b'\\x07\\x00\\x00\\xeb'
        """
        result = bytes()
        while read_size > 0:
            temp_read_size = min(read_size, MAX_REQUEST_DATA_SIZE)
            request_data = struct.pack("II", read_address, temp_read_size)
            request, request_id = self._generate_header(REQUEST_TYPE_READ_MEMORY, len(request_data))
            request += request_data
            self.socket.send(request)

            raw_reply = self.socket.recv()
            reply_data = self._read_and_validate_header(raw_reply, request_id, REQUEST_TYPE_READ_MEMORY)

            if reply_data:
                result += reply_data
                read_size -= len(reply_data)
            else:
                return None

        return result

    def write_memory(self, write_address, write_contents):
        """
        >>> c.write_memory(0x100000, b"\\xff\\xff\\xff\\xff")
        True
        >>> c.read_memory(0x100000, 4)
        b'\\xff\\xff\\xff\\xff'
        >>> c.write_memory(0x100000, b"\\x07\\x00\\x00\\xeb")
        True
        >>> c.read_memory(0x100000, 4)
        b'\\x07\\x00\\x00\\xeb'
        """
        write_size = len(write_contents)
        while write_size > 0:
            temp_write_size = min(write_size, MAX_REQUEST_DATA_SIZE - 8)
            request_data = struct.pack("II", write_address, temp_write_size)
            request_data += write_contents[:temp_write_size]
            request, request_id = self._generate_header(REQUEST_TYPE_WRITE_MEMORY, len(request_data))
            request += request_data
            self.socket.send(request)

            raw_reply = self.socket.recv()
            reply_data = self._read_and_validate_header(raw_reply, request_id, REQUEST_TYPE_WRITE_MEMORY)

            if None != reply_data:
                write_address += temp_write_size
                write_size -= temp_write_size
                write_contents = write_contents[temp_write_size:]
            else:
                return False
        return True

    def set_pad_state(self, pad_state):
        request_data = struct.pack("III", 0, 4, pad_state)
        request, request_id = self._generate_header(REQUEST_TYPE_PAD_STATE, len(request_data))
        request += request_data
        self.socket.send(request)
        self.socket.recv()

    def set_touch_state(self, x, y, valid):
        request_data = struct.pack("IIhh?", 0, 4, x, y, valid)
        request, request_id = self._generate_header(REQUEST_TYPE_TOUCH_STATE, len(request_data))
        request += request_data
        self.socket.send(request)
        self.socket.recv()

    def set_motion_state(self, x, y, z, roll, pitch, yaw):
        request_data = struct.pack("IIhhhhhh", 0, 4, x, y, z, roll, pitch, yaw)
        request, request_id = self._generate_header(REQUEST_TYPE_MOTION_STATE, len(request_data))
        request += request_data
        self.socket.send(request)
        self.socket.recv()

    def set_circle_state(self, x, y):
        request_data = struct.pack("IIhh", 0, 4, x, y)
        request, request_id = self._generate_header(REQUEST_TYPE_CIRCLE_STATE, len(request_data))
        request += request_data
        self.socket.send(request)
        self.socket.recv()

    def set_resolution(self, resolution):
        request_data = struct.pack("IIh", 0, 4, resolution)
        request, request_id = self._generate_header(REQUEST_TYPE_SET_RESOLUTION, len(request_data))
        request += request_data
        self.socket.send(request)
        self.socket.recv()

if "__main__" == __name__:
    import doctest
    doctest.testmod(extraglobs={'c': Citra()})
