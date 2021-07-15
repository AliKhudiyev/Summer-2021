with open('enc') as f:
    text = f.read()

for c in text:
    tmp = ord(c) // 2**8
    print(f'{chr(tmp)}{chr(ord(c)-tmp*2**8)}', end='')
print()
