---
title: f0rizen's Hash checker 0x01
---

A solution for a crackme.

## Decompile

Dump the binary into Radare2 and decompile it, we get

```c
ulong main(ulong argc, int64_t argv)

{
    canary = *(in_FS_OFFSET + 0x28);
    s = argv;
    var_74h._0_4_ = argc;

    /* Check it's pure numeric & length = 6 */

    /* Begining of Useful Code */
    var_5ch._0_4_ = sym.imp.strlen(*(s + 8));   /* slen = strlen(argv[1]) */
    var_50h = *(s + 8);                         /* s = argv[1] a.k.a our input */
    var_48h = var_5ch + -1;                     /* temp = slen - 1 */
    uVar4 = (var_5ch * 8 + 0xfU) / 0x10;        /* halflen = slen / 2 */
    iVar1 = uVar4 * -0x10;                      /* probably a decompiler issue */
    var_40h = &main::s + uVar4 * -2;            /* long buf[6] */
    *var_40h = 1;                               /* buf[0] = 1 */
    for (var_60h = 1; var_60h < var_5ch; var_60h = var_60h + 1) {
        /* buf[i] = (buf[i-1] * 0x83) % 0x3b9aca09 */
        var_40h[var_60h] = (var_40h[var_60h + -1] * 0x83) % 0x3b9aca09;
    }
    /* at this point buffer look like */
    /* buf = { 0x1, 0x83, 0x4309, 0x224d9b, 0x118db651, 0x228a4e1d } */

    stack0xffffffffffffffa0 = *var_50h;         /* sum = argv[1][0] */
    for (var_64h = 1; var_64h < var_5ch; var_64h = var_64h + 1) {
        /* sum = sum + ((input[i] % 0x3b9aca09) * buf[i]) % 0x3b9aca09 */
        stack0xffffffffffffffa0 =
             stack0xffffffffffffffa0 + ((var_50h[var_64h] % 0x3b9aca09) * var_40h[var_64h]) % 0x3b9aca09;
    }
    /* check sum == 0x3c0431a5 */
    if (stack0xffffffffffffffa0 == 0x3c0431a5) {
        *(&stack0xffffffffffffff70 + iVar1) = 0x1396;
        sym.imp.puts("Key is valid");
    }
    else {
        *(&stack0xffffffffffffff70 + iVar1) = 0x13a4;
        sym.imp.puts("Wrong key!");
    }
    /* End of Useful Code */
    uVar3 = 0;
    goto canary_check;
    canary_check:
    /*...*/
}
```

## Werid Stack Behavior

From the decompiled code we can see that this code is doing something werid
using `argv` and `slen`.

```c
uVar4 = (var_5ch * 8 + 0xfU) / 0x10;        /* halflen = slen / 2 */
iVar1 = uVar4 * -0x10;                      /* probably a decompiler issue */
var_40h = &main::s + uVar4 * - 2;           /* char buf[6] */
*var_40h = 1;                               /* buf[0] = 1 */
```

After looking at the assembly, I think this may be a decompiler's misbehavior.

```asm
0x0000127d      488d14c50000.  lea rdx, [rax*8]   /* rdx = slen * 8 */
0x00001285      b810000000     mov eax, 0x10      /* rax = 0x10 */
0x0000128a      4883e801       sub rax, 1         /* rax = rax - 1 = 0xf */
0x0000128e      4801d0         add rax, rdx       /* rax = slen * 8 + 0xf */
0x00001291      be10000000     mov esi, 0x10
0x00001296      ba00000000     mov edx, 0
0x0000129b      48f7f6         div rsi            /* rax = slen / 2 = 3 */
0x0000129e      486bc010       imul rax, rax, 0x10/* rax = 3 * 0x10 = 0x30 */
0x000012a2      4829c4         sub rsp, rax       /* allocate 0x30 stack for buf! */
0x000012a5      4889e0         mov rax, rsp       
```

Looks like `buf` is well-formed now, this werid code is just allocating `0x30` bytes
to it, which is exactly `6 * sizeof(long)`.

## I dnk math, but I know bruteforce

After resolving the decompliation myth. we can take a proper look at the checksum.
It first generates a buffer containing `6` magic numbers.

```c
buf = { 0x1, 0x83, 0x4309, 0x224d9b, 0x118db651, 0x228a4e1d };
```

Then, without using the first, it uses the rest to multiply **elementwisely**
with our input's ASCII code, then sum them up.

I genuinely dnk how to reverse this hash process, but we can esstially speed iterate
up to bruteforce out the checksum.

We can create a table storing the multiplication of `('1',..,'9') * buf`, totaling
`9 * 5 = 45` numbers as `'0'` and `buf[0]` is unused.

Then iterate all combination from `111111` to `999999` to sum different entries
in the previous table to crack it.

```python
#!/usr/bin/python3
from pprint import pprint
buf = [1]
for i in range(1, 6):
    buf.append((buf[i-1] * 0x83) % 0x3b9aca09)

for k in buf:
    print(hex(k))

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
        print(f'cracked, the code is {l}')
        break
```

The result is `231337`.
