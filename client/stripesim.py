import pygame
import sys
from time import time, sleep

class Stripesim:
    def __init__(self, size = 240, csize=5, cdist=0, fpslim=25):
        self.csize = csize    # Size of circles
        self.cdist = cdist    # Distance between circles
        self.size = size      # Length of strip
        self.limit = 1. / fpslim

        self.screen_width = self.size*csize + (self.size+1)*cdist
        self.screen_height = csize + 2*cdist
        self.last_update = time()

        pygame.init()
        self.screen = pygame.display.set_mode((self.screen_width, self.screen_height))


    def draw_point(self, pos, color):
        if pos < 0 or pos >= self.size:
            return

        pygame.draw.circle(self.screen,
                           (255 * color[0], 255 * color[1], 255 * color[2]),
                           ((self.csize+self.cdist)*pos + self.csize/2+self.cdist, self.csize/2+self.cdist),
                           self.csize/2
                           )

    def write(self, data):
        now = time()
        if now - self.last_update < self.limit:
            sleep(self.limit - now + self.last_update)
        self.last_update = now
        for pt in range(self.size):
            self.draw_point(pt, (data[3*pt+1], data[3*pt], data[3*pt+2]))
        pygame.display.update()

    def exit(self):
        pygame.quit()

import numpy as np
s = Stripesim()
s.write(np.random.rand(720))
sleep(5)