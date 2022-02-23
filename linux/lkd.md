# Linux Kernel Development Chapter 2

这是 LKD 的读书笔记，希望能对自己以后在 Linux 下开发内核程序有所帮助。

## 内核配置

有多种手段对内核编译选项进行配置。

1. 最简单的 `make config` 会遍历询问每个选项的 `Y/N`，非常 tedious。
2. TUI 的 `make menuconfig` 是由 ncurse 支持的界面选项 （我一般用这个）。
3. GUI 的 `make gconfig`，这个我还没用过。

### 特殊配置命令

使用架构默认配置

```shell
make defconfig
```

检查并更新配置（建议每次配置完都运行）

```shell
make oldconfig
```

## 内核编程的特殊性

与用户程序相比，内核程序有着自己的特殊特点。

### 无法使用 C 库

在内核中我们无法使用标准库，而应当转用由内核提供的例如 `<linux/string.h>` 这类头文件。内核函数的命名方式也有所不同，例如 `printf -> printk`。

### 内联函数

当内核需要时间敏感的函数时，通常使用 `static inline` 来定义内联函数，例如。

```c
static inline void wolf(unsigned long tail_size)
```

这类函数声明必须在任何使用之前否则编译器无法进行内联，通常将声明放置于头文件之中，因为 `static` 函数并不会创建一个导出的函数（TODO: 没看懂，有空了解一下）。

### 分支标注

gcc 编译器提供了一个内建的功能来指明“更可能”运行到的分支，名为 `likely` 和 `unlikely`，其用法如下。

```c
if (unlikely(error)) {
    /* ... */
}
```

# LKD Chapter 3 Process 进程

