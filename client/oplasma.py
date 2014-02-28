__author__ = 'jan'

from ledstripe import LEDStripe
from stripesim import Stripesim
from stripecache import Stripecache
import numpy as np
import signal

def plasma():
    #led = LEDStripe()
    #led = Stripesim()
    led = Stripecache('oplasma3.a')
    x = np.linspace(0, 1, 240)
    t = 0
    for i in range(100000):
        t += 0.1
        yr = np.sin(2*np.pi*1*(1.3+np.sin(0.15*t))*x - 0.32*t) + \
             np.sin(2*np.pi*3.32*(1.3-np.sin(0.18*t))*x + 0.76*t) + \
             np.cos(2*np.pi*0.65*(1.3+np.sin(0.05*t))*x + 0.23*t)
        yr = (1.0 + yr) / 4.0
        yr = np.maximum(np.minimum(yr, 1.0), 0.0)
        yg = np.sin(2*np.pi*1.43*(1.3-np.sin(0.21*t))*x + 0.12*t) + \
             np.sin(2*np.pi*2.78*(1.3+np.sin(0.15*t))*x - 0.43*t) + \
             np.cos(2*np.pi*0.23*(1.3-np.sin(0.05*t))*x + 0.33*t)
        yg = (1.0 + yg) / 4.0
        yg = np.maximum(np.minimum(yg, 1.0), 0.0)
        yb = np.sin(2*np.pi*0.32*(1.3-np.sin(0.04*t))*x - 0.07*t) + \
             np.sin(2*np.pi*0.87*(1.3+np.sin(0.2*t))*x + 0.12*t) + \
             np.cos(2*np.pi*1.54*(1.3+np.sin(0.12*t))*x - 0.19*t)
        yb = (yb - 2.0) / 12.0
        yb = np.maximum(np.minimum(yb, 1.0), 0.0)
        comb = np.array([yg, yr, yb]).transpose().flatten()
        led.write(comb)



if __name__ == '__main__':
    plasma()
