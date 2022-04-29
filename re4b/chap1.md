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

## Note 函数参数

对于 CPU 而言，函数参数的传递方式是 **不可知** 的，不论是寄存器传递还是栈传递都是可以接受的，
完全以编译器决定。

## Exercise 51, 52 & 53

### 51 [https://challenges.re/51/](https://challenges.re/51/)

```c
#include <stdio.h>

int main()
{
  printf ("%d, %d, %d\n");

  return 0;
};
```

### Solution 51

msvc

```asm
$SG5198 DB        '%d, %d, %d', 0aH, 00H

main    PROC
$LN3:
        sub     rsp, 40                             ; 00000028H
        lea     rcx, OFFSET FLAT:$SG5198
        call    printf
        xor     eax, eax
        add     rsp, 40                             ; 00000028H
        ret     0
main    ENDP
```

gcc

```asm
.LC0:
        .string "%d, %d, %d\n"
main:
        push    rbp
        mov     rbp, rsp
        mov     edi, OFFSET FLAT:.LC0
        mov     eax, 0
        call    printf
        mov     eax, 0
        pop     rbp
        ret
```

他们都输出了一些垃圾信息，是保存在栈上的本该交给 `main()` 的参数，例如 `argc` 和 `argv` 等。

### 52 [https://challenges.re/52/](https://challenges.re/52/)

```asm
$SG3103 DB '%d', 0aH, 00H
_main PROC
  push 0
  call DWORD PTR __imp___time64
  push edx
  push eax
  push OFFSET $SG3103 ; '%d'
  call DWORD PTR __imp__printf
  add esp, 16
  xor eax, eax
  ret 0
_main ENDP
```

### Solution 52

1. 将 `0(NULL)` 作为参数传给函数 `time64()`。
2. 将 `time64()` 的返回值 `eax` 与格式化字符串 `"%d"` 作为参数传给函数传给 `printf`
3. 清空用于传参的栈空间，清空 `eax`，返回

### 53 [https://challenges.re/53/](https://challenges.re/53/)

```c
#include <string.h>
#include <stdio.h>

void alter_string(char *s)
{
        strcpy (s, "Goodbye!");
        printf ("Result: %s\n", s);
};

int main()
{
        alter_string ("Hello, world!\n");
};
```

### Solution 53

gcc 与 msvc 对于字符串常量的处理方式不同。
gcc 将只读数据存储在 `.rodata` 段，在加载时其页表会被设为只读（触发栈错误）。
msvc 将字符串存储在 `.data` 段，可读可写，因此不会触发段错误。

## Note 全局变量 v.s. 局部变量

全局变量存储在 `.bss` 段，会由操作系统清零，而局部变量存储在栈上，不会被清零。
因此将全局变量改为未初始化的局部变量可能出现难以发觉的 bug 。

## Note 结构体作为返回值

当返回值不足以存入寄存器时，编译器会传入一个**隐藏参数**作为指向结构体的指针。

```asm
$T3853 = 8 ; size = 4
_a$ = 12 ; size = 4
?get_some_values@@YA?AUs@@H@Z PROC ; get_some_values
mov ecx, DWORD PTR _a$[esp-4]     ; 函数序言被省略，直接使用 ESP 访问参数
mov eax, DWORD PTR $T3853[esp-4]  ; 函数序言被省略，直接使用 ESP 访问自动生成的指针 $T3853
lea edx, DWORD PTR [ecx+1]        ; 利用 lea 指令完成计算，edx = ecx + 1
mov DWORD PTR [eax], edx          ; 将 edx 中的值放入 DWORD PTR [eax] 指向的地址
lea edx, DWORD PTR [ecx+2]        ; 同上，利用 lea 指令完成计算
add ecx, 3
mov DWORD PTR [eax+4], edx
mov DWORD PTR [eax+8], ecx
ret 0
?get_some_values@@YA?AUs@@H@Z ENDP ; get_some_values
```

## Note `lea` 指令用于计算

`lea` 指令指 `Load Effective Address` 实际上就是直接计算 `[complicate formula]` 方括号内的内容。
其可以用于计算所有能够放入地址计算方式的算数表达式，例如

```asm
lea ecx, [edx * edx+1] ; ecx = edx * edx + 1
```

## Note 条件跳转

x86 与 Arm 上的条件跳转利用 Flag 寄存器来判断是否执行跳转，可以看成下面的两步组合。

x86

```asm
  cmp reg, ret/val
  Jcc true
false:
  ;...
true:
  ;...
```

Arm

```asm
  cmp reg, ret/val
  Bcc true
false:
  ;...
true:
  ;...
```

而对于 MIPS，RISC-V 等架构，其提供了不同的指令来进行不同条件的跳转，`beq/bltz/...`。

> `switch` 条件语句，
> 在 `case` 不多的情况下，`switch` 会生成多条条件跳转语句。
> 在 `case` 较多的情况下，`switch` 会生成跳转表以减小二进制文件大小。
> 在 Fall-Through 的情况下，跳转表内会有多个**相同**的项指向同一个地址。

## Note 循环语句

伟大的 x86 提供了用于循环的 `loop` 指令，其检查寄存器 `ecx` 的值，如果不为 `0` 就减少 `ecx`
并跳转到对应的 `<label>`，但因为过于不好用而不再有编译器生成这类指令。

对于一般的 `for` 循环而言，下面是常见的结构。

```c
for (init; condition; action per iter) {
  loop_body;
}
```

其在**开始**就会检查条件来决定是否进入 `for` 循环，然而对编译器而言，通常会将检查的代码生成在**最后**。

```asm
  mov [counter], <init_val>
  jmp check
body:
  ; loop body
  add [counter] 1
check:
  cmp [counter], <fini_val>
  jle body
```

> [counter] 可以被一个专属寄存器替代（如有空余寄存器）。
> 当编译器确定第一次循环一定为真时，可能生成类似 `do-while` 结构的代码而非先检查的 `for` 结构。

## Exercise 56 & 57

### 56 [https://challenges.re/56/](https://challenges.re/56/)

```asm

$SG2795  DB  '%d', 0aH, 00H

_main  PROC
  push  esi   ; 保存将被用到的寄存器 esi & edi
  push  edi   ; 
  mov  edi, DWORD PTR __imp__printf ; 使用 edi 存储 printf 函数地址
  mov  esi, 100                     ; 初始化 esi 作为 [counter]
  npad  3 ; align next label        ; 对齐
$LL3@main:
  push  esi                         ; 压入第二个参数 100..1
  push  OFFSET $SG2795 ; '%d'       ; 压入第一个参数 "%d"
  call  edi                         ; 调用 printf
  dec  esi                          ; 减小 esi，counter--
  add  esp, 8                       ; 栈平衡，释放两个参数占用的空间
  test  esi, esi                    ; 判断 esi 是否为 0
  jg  SHORT $LL3@main               ; 条件检查
  pop  edi                          ; 恢复保存的 edi
  xor  eax, eax                     ; 设置返回值为 0
  pop  esi                          ; 恢复保存的 esi
  ret  0                            ; 返回
_main  ENDP
```

### Solution 56

输出 `100..1`，细节见代码注释。

### 57 [https://challenges.re/57/](https://challenges.re/57/)

一个类似的例子只是输出变为 `1..100` 步长为 `3`。

## Note 一些新指令

1. `test reg_1, reg_2` 将两个操作数进行 `and` 并设置相应的标志，可以用来判断寄存器是否为 `0`
2. `movsx reg_1, addr` 符号扩展移动

## Exercise 59

### 59 [https://challenges.re/59/](https://challenges.re/59/)

```asm
_a$ = 8
_f  PROC
  mov  ecx, DWORD PTR _a$[esp-4]  ; ecx = arg
  lea  eax, DWORD PTR [ecx*8]     ; eax = ecx * 8
  sub  eax, ecx                   ; eax = eax - ecx
  ret  0
_f  ENDP
```

### Solution 59

返回传入参数 `arg` 的七倍，`arg * 7`。
