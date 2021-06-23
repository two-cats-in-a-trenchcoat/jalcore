import pygame

import numpy as np

from time import sleep

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


def update():
    image = np.fromfile("core.bin",dtype=np., sep='')
    print(image)
    sleep(0.25)
    return image.astype('uint8')


#viewer = Viewer(update, (1920, 1080))
#viewer.start()
update()