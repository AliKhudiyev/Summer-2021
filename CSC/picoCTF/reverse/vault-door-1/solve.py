import re

with open('VaultDoor1.java') as f:
    text = f.read()

matches = re.findall("charAt(.*)", text)
# print(matches)
text = '\n'.join(matches)
# print(text)
matches = re.findall("(\d{1,2}).*==", text)
# print(matches)
matches2 = re.findall("'.'", text)
# print(matches2)
flag = ['' for i in range(32)]

for m1, m2 in zip(matches, matches2):
    # print(int(m1), m2.replace("'", ""))
    flag[int(m1)] = m2.replace("'", "")
print(f"picoCTF{{{''.join(flag)}}}")
