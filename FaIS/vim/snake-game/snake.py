import numpy as np
import random
from tkinter import *


class Snake:
	def __init__(self, coords, speed, size):
		self.speed = speed
		self.size = size
		self.coords = coords
		self.blocks = [coords]
	
	def move(self, direction):
		self.coords = self.coords[0]+direction[0]*self.size, self.coords[1]+direction[1]*self.size
		for i in reversed(range(1, len(self.blocks))):
			self.blocks[i] = self.blocks[i-1]
		self.blocks[0] = self.coords
	
	def grow(self, direction):
		last_block = (self.blocks[-1][0], self.blocks[-1][1])
		self.move(direction)
		self.blocks.append(last_block)
		# print(f'grow triggered, created {self.blocks[-1]}')
	
	def is_dead(self, borders):
		(x0, y0), (x1, y1) = borders
		x, y = self.coords

		if x <= x0 or x >= x1 or y <= y0 or y >= y1:
			return True

		for i in range(len(self.blocks)):
			for j in range(i+1, len(self.blocks)):
				if self.blocks[i] == self.blocks[j]:
					return True

		return False
	
	def accelerate(self, amount):
		self.speed += amount

	def get_rectangle(self, block_index):
		block = self.blocks[block_index]
		return (block[0]-self.size/2, block[1]-self.size/2), (block[0]+self.size/2, block[1]+self.size/2)


class Game:
	def __init__(self, width=640, height=640, speed=1, size=20, acceleration=0):
		self.width = width
		self.height = height

		self.root = Tk()
		self.root.title('Snake')
		self.canvas = Canvas(self.root, width=width, height=height)
		self.canvas.pack()
		
		coords = random.randrange(size/2, width-size/2, size), random.randrange(size/2, height-size/2, size)
		directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]
		self.snake = Snake(coords, speed, size)
		self.rects = []
		self.direction = (random.choice(directions))
		self.food_coords = random.randrange(size/2, width-size/2, size), random.randrange(size/2, height-size/2, size)
		
		(x0, y0), (x1, y1) = self.snake.get_rectangle(0)
		self.rects.append(self.canvas.create_rectangle(x0, y0, x1, y1, fill='green'))
		self.food = self.canvas.create_oval(self.food_coords[0]-size/2, self.food_coords[1]-size/2, self.food_coords[0]+size/2, self.food_coords[1]+size/2, fill='red')

		self.root.bind_all('a', lambda e: self.move((-1, 0)))
		self.root.bind_all('d', lambda e: self.move((1, 0)))
		self.root.bind_all('w', lambda e: self.move((0, -1)))
		self.root.bind_all('s', lambda e: self.move((0, 1)))
	
		self.root.after(500, self.run)
		self.root.mainloop()

	def move(self, direction):
		# print(direction)
		self.direction = direction
	
	def run(self, speed=1, size=1):
		if self.snake.is_dead(((0, 0), (self.width, self.height))):
			self.quit()

		size = self.snake.size
		food_coords = self.food_coords
		
		self.snake.move((self.direction[0], self.direction[1]))

		print(self.snake.blocks[0], self.food_coords)
		if self.snake.blocks[0] == self.food_coords:
			self.snake.grow((self.direction[0], self.direction[1]))
			(x0, y0), (x1, y1) = self.snake.get_rectangle(-1)
			self.food_coords = random.randrange(size/2, self.width-size/2, size), random.randrange(size/2, self.height-size/2, size)
			# print(x0, y0, x1, y1, 'good job!')
			self.rects.append(self.canvas.create_rectangle(x0, y0, x1, y1, fill='green'))
			self.canvas.move(self.food, self.food_coords[0]-food_coords[0], self.food_coords[1]-food_coords[1])

		for i in range(len(self.rects)):
			coords = self.canvas.coords(self.rects[i])
			new_coords = self.snake.blocks[i]
			dx, dy = new_coords[0]-coords[0], new_coords[1]-coords[1]
			# print(dx, dy)
			self.canvas.move(self.rects[i], dx, dy)
		
		self.root.after(250, self.run)

	def quit(self):
		self.root.destroy()
			

game = Game()
game.run()
