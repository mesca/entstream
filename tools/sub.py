import time
import struct
import zmq

def deserialize(data):
    return {
        'topic': data[0],
        'serial': data[1],
        'timestamp': struct.unpack('d', data[2])[0],
        'data': data[3],
        'setbits': data[4],
        'zscore': struct.unpack('d', data[5])[0]
    }

context = zmq.Context.instance()
sub = context.socket(zmq.SUB)
sub.connect('ipc:///tmp/backend')
#sub.connect('tcp://127.0.0.1:5000')
sub.setsockopt(zmq.SUBSCRIBE, 'entropy'.encode('utf-8'))

while True:
    try:
        while True:
            packet = sub.recv_serialized(deserialize, zmq.NOBLOCK)
            print(packet)
    except zmq.ZMQError:
        pass # No more data
    time.sleep(1)

