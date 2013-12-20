__author__ = 'jan'

from ledstripe import LEDStripe
import numpy as np
import time

def speedtest():
    led = LEDStripe()
    data = (0.25,0.5,0)*240
    print("Serial Strip Speed Test, 240 LEDs")
    print("Writing 1000 full updates")
    start = time.time()
    for i in range(1000):
        led.write(data)
    stop = time.time()
    diff = stop - start
    print("Took %f seconds" % (diff))
    print("That is %f FPS" % (1000 / diff))

if __name__ == '__main__':
    speedtest()