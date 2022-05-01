#!/usr/bin/python3
from pprint import pprint
buf = [1]
for i in range(1, 6):
    buf.append((buf[i-1] * 0x83) % 0x3b9aca09)

for k in buf:
    print(hex(k))
exit();
d = {}
for i in range(1, 6):
    d[i] = {}
    for j in range(1, 10):
        d[i][j] = hex(buf[i] * (ord("0") + j) % 0x3b9aca09)

pprint(d)

for l in range(111111, 1000000):
    k = str(l).zfill(6)
    s = ord("0") + int(k[0])
    for (i, j) in enumerate(k[1:]):
        if int(j) == 0:
            continue
        s += int(d[i+1][int(j)], 16)
    print(f'{k}: {hex(s)}')
    if s == 0x3c0431a5:
        print(l)
        break
