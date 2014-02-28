from stripesim import Stripesim
import sys
import numpy as np

def calibrate():
    led = Stripesim()
    gamma = 1.0
    in_raw = np.arange(240) / 239.0
    while (1):
        out = np.zeros((240, 3))
        out[:, 1] = in_raw ** gamma
        print out.flatten()
        led.write(out.flatten())
        instr = sys.stdin.readline()
        gamma = float(instr)


if __name__ == '__main__':
    calibrate()