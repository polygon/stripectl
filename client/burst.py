from stripesim import Stripesim
from stripecache import Stripecache
import numpy as np
from time import time, sleep

def capcos(x):
    out = np.cos(x)
    out[abs(x) > np.pi] = 0.
    return out

class Burst():
    def __init__(self):
        #self.led = Stripesim(size = 240)
        self.led = Stripecache('burst.a')

    def run(self):
        self.collide()
        self.explode()
        self.fadeout()

    def collide(self):
        for pos in range(120):
            out = np.zeros((240, 3))
            out[pos, 1] = 1.
            out[239-pos, 1] = 1.
            self.led.write(out.flatten())

    def explode(self):
        coef = np.concatenate((np.linspace(-np.pi/2, 0, 120),
                               np.linspace(0, np.pi/2, 120)))
        for factor in np.logspace(-3, 0.6, 60)[::-1]:
            out = np.zeros((240, 3))
            out[:, 0] = capcos(factor/2 * coef)
            out[:, 1] = capcos(factor/5 * coef)
            out[:, 2] = capcos(factor * coef)
            self.led.write(np.maximum(0.0, out.flatten()))

    def fadeout(self):
        out = np.ones((240, 3))
        fade_b = 0.9 - np.random.rand(240) * 0.1
        fade_g = 0.97 - np.random.rand(240) * 0.04
        fade_r = 0.99 - np.random.rand(240) * 0.02
        for i in range(250):
            out[:, 0] = out[:, 0] * fade_g
            out[:, 1] = out[:, 1] * fade_r
            out[:, 2] = out[:, 2] * fade_b
            self.led.write(out.flatten())

if __name__ == '__main__':
    burst = Burst()
    #while (1):
    burst.run()