__author__ = 'jan'

import serial
import numpy as np
import time

class LEDStripe:
    def __init__(self, port = '/dev/ttyACM3', num_led = 240):
        self.ser = serial.Serial(port=port, baudrate=3000000)
        if self.ser.isOpen() is False:
            raise IOError('Failed to open serial port')
        self.num_led = num_led

    def write(self, data):
        # Convert and write data
        assert(len(data) == 3*self.num_led)
        out_data = [chr(int(255. * x)) for x in data]
        self.ser.write(''.join(out_data))
        time.sleep(0.03)


        # Wait for acknowledgement
#        inchar = self.ser.read(1)
#        assert(ord(inchar) == 48)
