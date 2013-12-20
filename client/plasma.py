__author__ = 'jan'

from ledstripe import LEDStripe
import numpy as np
import signal

def plasma():
    led = LEDStripe()
    x = np.linspace(0, 1, 240)
    t = 0
    while (1):
        t += 0.2
        yr = np.sin(2*np.pi*2*(1.3+np.sin(0.15*t))*x - 0.32*t) + \
             np.sin(2*np.pi*4.32*(1.3-np.sin(0.18*t))*x + 0.76*t) + \
             np.cos(2*np.pi*1.65*(1.3+np.sin(0.05*t))*x + 0.23*t)
        yr = (2.0 + yr) / 4.0
        yr = np.maximum(np.minimum(yr, 1.0), 0.0)
        yg = np.sin(2*np.pi*2.43*(1.3-np.sin(0.21*t))*x + 0.12*t) + \
             np.sin(2*np.pi*3.78*(1.3+np.sin(0.15*t))*x - 0.43*t) + \
             np.cos(2*np.pi*1.23*(1.3-np.sin(0.05*t))*x + 0.33*t)
        yg = (2.0 + yg) / 4.0
        yg = np.maximum(np.minimum(yg, 1.0), 0.0)
        yb = np.sin(2*np.pi*1.32*(1.3-np.sin(0.04*t))*x - 0.07*t) + \
             np.sin(2*np.pi*1.87*(1.3+np.sin(0.2*t))*x + 0.12*t) + \
             np.cos(2*np.pi*2.54*(1.3+np.sin(0.12*t))*x - 0.19*t)
        yb = (yb) / 4.0
        yb = np.maximum(np.minimum(yb, 1.0), 0.0)
        comb = np.array([yg, yr, yb]).transpose().flatten()
        led.write(comb)



if __name__ == '__main__':
    plasma()