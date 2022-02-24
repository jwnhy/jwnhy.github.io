# LKD 读书笔记 Part 1

这是 LKD 的读书笔记，希望能对自己以后在 Linux 下开发内核程序有所帮助。

## Linux Kernel Development Chapter 2

第二章主要内容是内核基础知识例如如何编译等。

### 内核配置

有多种手段对内核编译选项进行配置。

1. 最简单的 `make config` 会遍历询问每个选项的 `Y/N`，非常 tedious。
2. TUI 的 `make menuconfig` 是由 ncurse 支持的界面选项 （我一般用这个）。
3. GUI 的 `make gconfig`，这个我还没用过。

#### 特殊配置命令

使用架构默认配置

```shell
make defconfig
```

检查并更新配置（建议每次配置完都运行）

```shell
make oldconfig
```

### 内核编程的特殊性

与用户程序相比，内核程序有着自己的特殊特点。

#### 无法使用 C 库

在内核中我们无法使用标准库，而应当转用由内核提供的例如 `<linux/string.h>` 这类头文件。
内核函数的命名方式也有所不同，例如 `printf -> printk`。

#### 内联函数

当内核需要时间敏感的函数时，通常使用 `static inline` 来定义内联函数，例如。

```c
static inline void wolf(unsigned long tail_size)
```

这类函数声明必须在任何使用之前否则编译器无法进行内联，通常将声明放置于头文件之中，因为 `static` 函数并不会创建一个导出的函数（TODO: 没看懂，有空了解一下）。

#### 分支标注

gcc 编译器提供了一个内建的功能来指明“更可能”运行到的分支，名为 `likely` 和 `unlikely`，其用法如下。

```c
if (unlikely(error)) {
    /* ... */
}
```

## LKD Chapter 3 Process 进程

在 Linux 中，进程和线程并没有明显区分，只是一部分进程“恰好”共享了一些资源（文件描述符，内存空间等）。
进程的生命周期如下。

1. 父进程调用 `fork()` 库函数，其返回两次，一次在父进程，另一次在子进程。
2. 子进程通常在 `fork()` 返回后立刻使用 `exec()` 来创建新的地址空间并加载程序。
3. 当一个程序通过 `exit()` 退出时，父进程可以调用 `wait4()` 来查询其状态，如果父进程不调用，该进程会被置于僵尸进程区。

### 进程描述符与任务结构体

内核使用**循环双向链表**来存储类型为 `struct task_struct` 的进程描述符，其定义在 `<linux/sched.sh>`。
这类型的描述符在 32 位机器上大小为 1.7 KB，还是比较大的，但它包含了所有内核需要的进程信息，例如进程的地址空间，挂起的信号，进程状态等。

![image.png](https://ae03.alicdn.com/kf/H7e8b990f689e449f92a2000f66e55167m.png)

### 分配进程描述符

为了便于利用 `sp` 获取进程描述符位置，通常将 `task_struct` 放于内核栈顶部[最低地址]（prior 2.4），
这样只需要一个寄存器 `sp` 以及一些计算，即可知道 `task_struct` 位置。
由于 `task_struct` 太大了，逐渐不适合直接放置于内核栈顶部，后来改为使用 `slab` 分配器分配 `task_struct` ，
以此允许使用对象重用（object reuse）和缓存染色（cache coloring）。
并使用 `thread_info` 替代 `task_struct` 放入内核栈顶部（2.6～4.8），`thread_info` 如下所示。

```c
struct thread_info {
  struct task_struct *task;
  struct exec_domain *exec_domain;
  __u32 flags;
  __u32 status;
  __u32 cpu;
  int preempt_count;
  mm_segment_t addr_limit;
  struct restart_block restart_block;
  void *sysenter_return;
  int uaccess_err;
};
```

再后来，由于 Linux 引入了 `percpu` 全局变量描述当前 CPU 上执行任务的信息，`thread_info` 内存储的
信息也逐渐减少，具体见该 [commit](https://github.com/torvalds/linux/commit/15f4eae70d365bba26854c90b6002aaabb18c8aa)。

### 存储进程描述符

系统通过 `pid_t pid;` 来识别每个进程，为了与 UNIX 兼容，其最大值仅为 `32,768`，
不过可以通过 `/proc/sys/kernel/pid_max` 修改。获取进程描述符的方式（prior 4.9），
可以通过将栈顶指针的低 13 位清零获得位于最低地址的进程描述符。

![image.png](https://ae02.alicdn.com/kf/H73462181c476422f9e90273ad2741217M.png)

```asm
movl $-8192, %eax
andl %esp, %eax
```

这些都是由 `current_thread_info()` 函数实现的，最后可以通过其 `task` 成员获得 `task_struct`。

```c
current_thread_info() -> task;
```


