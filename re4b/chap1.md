# RE4B Chap 1 Notes

## Exercise 48 & 49

出现在 1.5.6 节

### 48 [https://challenges.re/48/](https://challenges.re/48/)

```asm
main:
    push 0xFFFFFFFF
    call MessageBeep
    xor  eax,eax
    retn
```

### Solution 48

1. 向栈中压入 `0xFFFFFFFF`
2. 调用 `MessageBeep`
3. 清空 `eax` 返回值寄存器
4. 返回上一层

### 49 [https://challenges.re/49/](https://challenges.re/49/)

```asm
main:
        pushq   %rbp
        movq    %rsp, %rbp
        movl    $2, %edi
        call    sleep
        popq    %rbp
        ret
```

### Solution 49

1. 向栈中压入栈基地址 `rbp`
2. 将当前栈顶设为栈基地址
3. 将 `2` 通过 `edi` 寄存器作为函数参数
4. 调用 `sleep` 函数
5. 弹出栈基地址 `rbp`

## Note 函数序言与结语

### 函数序言

```asm
push ebp
mov ebp, esp
sub esp, X
```

1. 压入栈基地址，保存栈帧
2. 将 callee 的栈基地址设为当前栈顶
3. 在栈上为 callee 局部变量分配空间

> 栈基地址被用于访问当前函数的局部变量等，根据架构的不同也可被称为 `frame pointer`。
> `esp` 也可以用来访问局部变量，例如 `rust` 支持省略栈基地址。

### 函数结语

```asm
mov esp, ebp
pop ebp
ret 0
```

1. 将栈顶恢复（栈基地址在 callee 调用过程中始终保持不变）
2. 弹出栈基地址，恢复栈帧
3. 返回上一层

## Note 栈的作用

### 储存函数返回地址

在 x86 上，函数调用是由 `call/ret` 指令组合实现，其中 `call` 指令相当于

1. `push <address after call>(pc+4)`
2. `jmp <function>`

在 Arm 上，存在一个 LR 寄存器，用于存储返回地址，但当函数调用多于一层时，仍然需要使用栈存储返回地址。

### 传递函数参数

```asm
push arg3
push arg2
push arg1
call f
add esp, 12 ; 4 * 3 = 12 release arg space
```

栈在函数进入前的状态

| stack | usage            |
|-------|------------------|
| ESP   | return address   |
| ESP+4 | arg#1, arg_0 IDA |
| ESP+8 | arg#2, arg_4 IDA |
| ESP+C | arg#3, arg_8 IDA |

函数参数也可以通过寄存器方式进行传递。

> `alloc()` 函数，该函数直接在栈上分配空间（而非像 `malloc()` 在堆上）。

### 常见栈结构

| stack | usage            |
|-------|------------------|
| ESP-C | localvar#2, var_8|
| ESP-8 | localvar#1, var_4|
| ESP-4 | saved EBP        |
| ESP   | return address   |
| ESP+4 | arg#1, arg_0 IDA |
| ESP+8 | arg#2, arg_4 IDA |
| ESP+C | arg#3, arg_8 IDA |
