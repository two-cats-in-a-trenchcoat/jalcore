#! /usr/bin/env python3
import pygame
import numpy as np
from time import sleep
from random import randint
from shutil import copyfile
import os

class Viewer:
    def __init__(self, update_func, display_size):
        self.display_size = display_size
        self.update_func = update_func
        pygame.init()
        self.display = pygame.display.set_mode(display_size)

    def set_title(self, title):
        pygame.display.set_caption(title)

    def start(self):
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

            z = self.update_func()
            surf = pygame.surfarray.make_surface(z)
            new = pygame.transform.scale(surf, self.display_size)
            self.display.blit(new, (0, 0))

            pygame.display.update()

        pygame.quit()

'''
32    111 000 00  = randint( 0, 7 ) << 5   r
64    000 111 00  = randint( 0, 7 ) << 2   g
96    000 000 11  = randint( 0, 3 )        b
128   111 111 11  = randint( 0, 255 )      w
'''

real = input("Enter File: ")
size = int(input("Size: "))
realsize = int(input("Window Size: "))

screenshot = bytearray( size * size )
filebits = (size * size)

print(filebits)

copyfile(real, "clipped.tmp")
os.truncate("clipped.tmp", filebits)

with open("clipped.tmp", "rb")as file:
    screenshot = np.array(bytearray(file.read()))  ##  128 *128 8-bit image, just for testing
    file.close()

os.truncate("clipped.tmp", 0)

row, col = 0, 0
arr = np .zeros( ( size, size, 3 ) )  ##  generate empty numpy array

for byte in screenshot:
    rrr = int( ( ( byte & 0b11100000 ) >> 5 ) *36.4285714286  )  ##  red
    ggg = int( ( ( byte & 0b00011100 ) >> 2 ) *36.4285714286 )  ##  green
    bb = int( ( byte & 0b00000011 ) *85 )  ##  blue  --  multiplied to max 255 range
    
    arr[ col ][ row ][ 0 ] = rrr  ##  insert color values directly into numpy cells
    arr[ col ][ row ][ 1 ] = ggg
    arr[ col ][ row ][ 2 ] = bb

    col += 1
    if col == size:
        col = 0  ##  \r  carriage return
        row += 1  ##  \n  newline


def update():
    image = arr
    sleep(0.25)
    return image

viewer = Viewer(update, (realsize, realsize))
viewer.start()
update()
