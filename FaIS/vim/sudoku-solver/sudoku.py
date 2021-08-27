from tkinter import *
import numpy as np
import random
import copy
import sys
import threading
import getch

class Sudoku:
    def __init__(self, difficulty='normal'):
        self.table = np.zeros((9, 9), dtype=np.int8)
        self.n_iter = 0
        self.reset(difficulty)

    def __str__(self):
        return str(self.table)
    
    def reset(self, difficulty):
        self.solve()
        print(f'Prepared (#iter: {self.n_iter})')
        self.n_iter = 0

        hide_prob = 0.5
        if difficulty == 'easy':
            hide_prob = 0.25
        elif difficulty == 'hard':
            hide_prob = 0.75

        for y in range(9):
            for x in range(9):
                if random.uniform(0, 1) <= hide_prob:
                    self.table[y][x] = 0

    def is_solved(self):
        for i in range(9):
            if np.sum(self.table, axis=0) != 45 or np.sum(self.table, axis=1) != 45 or np.sum(self.table[i/3*3:i/3*3+3, i%3*3:i%3*3+3]) != 45:
                    return False
        return True

    def _get_block(self, block_id):
        block = self.table[block_id//3*3:block_id//3*3+3, block_id%3*3:block_id%3*3+3]
        # print(block_id, block_id//3*3+3, block.shape)
        return block

    def _get_block_id(self, y, x):
        return y//3*3 + x//3

    def _get_possibilities_for(self, y, x):
        possibilities = []
        block_id = self._get_block_id(y, x)
        block = self._get_block(block_id)
        impossibilites = []

        for i in range(9):
            if self.table[y][i] != 0 and self.table[y][i] not in impossibilites:
                impossibilites.append(self.table[y][i])
            if self.table[i][x] != 0 and self.table[i][x] not in impossibilites:
                impossibilites.append(self.table[i][x])
            if block[i//3][i%3] != 0 and block[i//3][i%3] not in impossibilites:
                impossibilites.append(block[i//3][i%3])

        for i in range(1, 10):
            if i not in impossibilites:
                possibilities.append(i)

        return possibilities

    def solve(self):
        self.n_iter += 1
        for y in range(9):
            for x in range(9):
                if self.table[y][x] == 0:
                    possibilities = self._get_possibilities_for(y, x)
                    random.shuffle(possibilities)
                    if len(possibilities) == 0:
                        return False
                    for possibility in possibilities:
                        self.table[y][x] = possibility
                        is_solved = self.solve()
                        if not is_solved:
                            self.table[y][x] = 0
                        else:
                            return True
                    return False
        return True


if len(sys.argv) > 1 and sys.argv[1] == 'gui':
    root = Tk()
    root.title('Sudoku')

    frame = Frame(root)
    frame.pack()

    labels = [[Label(frame, text='', width=4, height=2, bg='gray', highlightthickness=2, highlightbackground='cyan') for i in range(9)] for j in range(9)]

sudoku = Sudoku()
table = None
active_label = None


def _solve(game):
    game.solve()
    print(f'Solved (#iter: {game.n_iter})')
    gui(game, solution=True)

def solve(game):
    th = threading.Thread(target=_solve, args=(game,))
    th.start()

def tell_index(label):
    global labels, table

    for y in range(9):
        for x in range(9):
            if labels[y][x] == label:
                return y, x
    return None, None

def modifiable(label):
    y, x = tell_index(label)

    return y is not None and x is not None and table[y][x] <= 0

def select(event):
    global active_label, table, labels
    
    if active_label is not None:
        active_label.config(bg='gray')
    event.widget.config(bg='purple')
    active_label = event.widget

def is_valid(table, y, x):
    block_id = y//3*3 + x//3
    block = table[block_id//3*3:block_id//3*3+3, block_id%3*3:block_id%3*3+3]
    num = np.absolute(table[y][x]) 
    vblock = list(np.absolute(table[np.r_[0:y,y+1:9],x]))
    hblock = list(np.absolute(table[y,np.r_[0:x,x+1:9]]))
    rblock = list(np.absolute(block.flatten(order='C')))
    rblock = rblock[0:y%3*3+x%3] + rblock[y%3*3+x%3+1:9]

    return num not in vblock and num not in hblock and num not in rblock

def put(digit):
    global active_label, table, labels, sudoku

    if modifiable(active_label) and len(digit.char) == 1 and 48 <= ord(digit.char) <= 57:
        y, x = tell_index(active_label)
        table[y][x] = -ord(digit.char)+48
        color = 'orange'

        if ord(digit.char) == 48:
            active_label.config(text='', fg=color)
        else:
            if not is_valid(table, y, x):
                color = 'red'
            active_label.config(text=digit.char, fg=color)

        for y in range(9):
            for x in range(9):
                if table[y][x] < 0:
                    if is_valid(table, y, x):
                        labels[y][x].config(fg='orange')
                    else:
                        labels[y][x].config(fg='red')

def gui(game, solution=False):
    global root, frame, labels, table

    if not solution:
        table = copy.copy(game.table)

    for y in range(9):
        for x in range(9):
            color = 'white' if table[y][x] > 0 else 'green'
            if (not solution and table[y][x] > 0) or solution:
                labels[y][x].config(text=str(game.table[y][x]), fg=color)
                if solution and table[y][x] <= 0:
                    table[y][x] = -game.table[y][x]

def init(difficulty):
    global sudoku, labels

    for y in range(9):
        for x in range(9):
            labels[y][x].config(text='')
    sudoku.reset(difficulty)
    gui(sudoku)


if len(sys.argv) > 1 and sys.argv[1] == 'gui':
    difficulty = StringVar(root)
    difficulty.set('normal')

    for i in range(9):
        root.bind(f'<Key>', lambda e: put(e))
        for j in range(9):
            labels[i][j].bind('<Button-1>', lambda e: select(e))
            labels[i][j].grid(row=i, column=j)

    OptionMenu(root, difficulty, 'easy', 'normal', 'hard').pack()
    Button(root, text='Solve', command=lambda: solve(sudoku)).pack()
    Button(root, text='Reset', command=lambda: init(difficulty.get())).pack()

    init('normal')
    root.mainloop()
else:
    while True:
        print('(e)asy, (n)ormal, (h)ard ?')
        c = getch.getch()
        if c != 'e' and c != 'n' and c != 'h':
            break
        sudoku = Sudoku(difficulty='easy' if c == 'e' else ('normal' if c == 'n' else 'hard'))
        print(sudoku)
        sudoku.solve()
        print(f'Solved (#iter: {sudoku.n_iter}):\n', sudoku)

