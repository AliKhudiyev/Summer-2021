from tkinter import *
import numpy as np
import sys, random


class FlappyBird:
    def __init__(self, width, height):
        self.width = width
        self.height = height

        self.animation_speed = 10
        self.score = 0

        self.max_wall_width = width//4
        self.min_wall_width = self.max_wall_width//2
        self.max_wall_height = 2*height//5
        self.min_wall_height = self.max_wall_height//2
        
        self.root = Tk()
        self.root.title('Flappy Bird')
        
        self.canvas = Canvas(self.root, width=self.width, height=self.height)
        self.canvas.pack()

        self.bird = self.canvas.create_oval(20, height//2-13, 60, height//2+13, fill='orange')
        self.walls = [self.canvas.create_rectangle(-1, 0, -1, 0, fill='green') for i in range(6)]

        self.root.bind('<space>', lambda e: self._fly())
        self.root.bind('<a>', lambda e: self._move(-25))
        self.root.bind('<d>', lambda e: self._move(25))
        self.root.after(1000, self.run)
        self.root.mainloop()

    def _out_of_border(self):
        x1, y1, x2, y2 = self.canvas.coords(self.bird)
        return x1 < 0 or x2 > self.width or y1 < 0 or y2 > self.height

    def _collision_with_walls(self):
        x1, y1, x2, y2 = self.canvas.coords(self.bird)
        for wall in self.walls:
            wall_coords = self.canvas.coords(wall)
            if wall_coords[0] <= x1 <= wall_coords[2] or wall_coords[0] <= x2 <= wall_coords[2]:
                if wall_coords[1] <= y1 <= wall_coords[3] or wall_coords[1] <= y2 <= wall_coords[3]:
                    return True
        return False

    def _dead(self):
        return self._out_of_border() or self._collision_with_walls()

    def _move(self, direction):
        self.canvas.move(self.bird, direction, 0)

    def _fly(self):
        self.canvas.move(self.bird, 0, -30)

    def _gravity(self):
        self.canvas.move(self.bird, 0, 6)

    def _modify_wall(self, wall, position, respawn_time, width=None, height=None):
        x1, y1, x2, y2 = 0, 0, 0, 0
        dist = self.width + respawn_time * self.animation_speed

        if width is None:
            width = random.randint(self.min_wall_width, self.max_wall_width)
        elif width < self.min_wall_width:
            width = self.min_wall_width
        elif width > self.max_wall_width:
            width = self.max_wall_width

        if height is None:
            height = random.randint(self.min_wall_height, self.max_wall_height)
        elif height < self.min_wall_height:
            height = self.min_wall_height
        elif height > self.max_wall_height:
            height = self.max_wall_height

        x1 = dist
        x2 = x1 + width

        if position == 'top':
            y1 = 0
            y2 = y1 + height
        else: # position == 'bottom'
            y2 = self.height
            y1 = y2 - height

        self.canvas.coords(wall, x1, y1, x2, y2)

    def _update_walls(self):
        for i, wall in enumerate(self.walls):
            x1, y1, x2, y2 = self.canvas.coords(wall)
            if x2 > 0:
                # draw the wall
                self.canvas.move(wall, -15, 0)
            else:
                # modify the wall
                self._modify_wall(wall, 'top' if random.randint(0,1) else 'bottom', respawn_time=30*i, width=None, height=None)
                self.canvas.move(wall, self.width, 0)

    def _update_score(self):
        _, _, x2, _ = self.canvas.coords(self.bird)
        self.score += x2 / self.width

    def run(self):
        if self._dead():
            print('Score: %.3f' % self.score)
            self.root.destroy()
            return
        self._gravity()
        self._update_walls()
        self._update_score()
        self.root.after(100, self.run)


game = FlappyBird(720, 720)

