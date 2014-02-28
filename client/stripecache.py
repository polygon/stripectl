import pygame
import sys
from time import time, sleep

class Stripecache:
    def __init__(self, outfile, size=240):
        self.size = size
        self.out = open(outfile, 'wb')

    def write(self, data):
        data_bin = [chr(int(255. * (x**3.0))) for x in data]
        self.out.write(''.join(data_bin))