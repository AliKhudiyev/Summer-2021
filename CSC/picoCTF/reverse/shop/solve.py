'''
Buy negative amount of foods, e.g. -10 quiet quiches
'''

flag = '112 105 99 111 67 84 70 123 98 52 100 95 98 114 111 103 114 97 109 109 101 114 95 55 57 55 98 50 57 50 99 125'

for c in flag.split(' '):
    print(chr(int(c)), end='')
print()
