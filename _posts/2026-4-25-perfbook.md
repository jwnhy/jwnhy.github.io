---
title: 性能分析方法的书
---

# 底层性能调优 & 并发编程 资料导航

> 以下均为领域内公认的经典资料，非原创整理。

---

## 微架构 & 底层调优 — 必读资料

### 📄 What Every Programmer Should Know About Memory

- **作者：** Ulrich Drepper, 2007 · 114 页
- **类型：** 免费 PDF
- **链接：** <https://people.freebsd.org/~lstewart/articles/cpumemory.pdf>
- **标签：** `Cache` `TLB` `Prefetch` `NUMA`
- **简评：** 内存子系统最权威的入门文章。从 DRAM 物理原理讲到 cache 层级、TLB、预取器、NUMA，每个 section 都有可复现的 benchmark 代码。**必读。**
- **阅读建议：** 重点精读 §3（CPU Cache）和 §4（Virtual Memory），§2 DRAM 原理可快速扫读。

### 📄 Agner Fog Optimization Manuals（5 卷）

- **作者：** Agner Fog · 持续更新至 2025
- **类型：** 免费 PDF
- **链接：** <https://www.agner.org/optimize/>
- **标签：** `Branch Prediction` `Pipeline` `SIMD` `Instruction Latency`
- **简评：** x86 优化的圣经。Vol.1 C++ 优化；Vol.3 Intel/AMD 微架构详解（流水线、分支预测、乱序执行）；Vol.4 指令延迟/吞吐量表格。
- **阅读建议：** Vol.3 microarchitecture.pdf 是理解 BTB、分支预测器的权威来源，结合 Godbolt 验证汇编食用。

### 📚 Computer Systems: A Programmer's Perspective (CS:APP)

- **作者：** Bryant & O'Hallaron · 第 3 版
- **类型：** 书
- **链接：** <http://csapp.cs.cmu.edu/3e/labs.html（配套实验）>
- **标签：** `Cache` `Loop Optimization` `ILP`
- **简评：** 第 5 章"优化程序性能"和第 6 章"存储层次结构"是最佳系统性入门。配套 datalab/cachelab 可直接上手实验。

### 📚 Performance Analysis and Tuning on Modern CPUs

- **作者：** Denis Bakhvalov · 2023
- **类型：** 书（部分免费）
- **链接：** <https://book.easyperf.net/perf_book>
- **标签：** `Top-Down` `PMU` `perf` `VTune`
- **简评：** 最新的现代调优书。涵盖 Top-Down 性能分析方法论、PMU 计数器实战、perf/VTune 使用，以及 Cache Miss、Branch Mispredict 的系统性修复流程。**强烈推荐。**

### 📚 Systems Performance（2nd ed）

- **作者：** Brendan Gregg · 2020
- **类型：** 书
- **链接：** <https://www.brendangregg.com/systems-performance-2nd-edition-book.html>
- **标签：** `perf` `BPF` `Flamegraph` `Linux`
- **简评：** Linux 系统性能分析的权威书。CPU 调度、内存、磁盘、网络全覆盖。perf、BPF、Flamegraph 工具链的最佳实战指南。

### 🌐 CPU.Land

- **作者：** Lexi Mattick & Hack Club · 2023
- **类型：** 免费网站
- **链接：** <https://cpu.land>
- **标签：** `Pipeline` `Cache` `Syscall`
- **简评：** 图文并茂的 CPU 工作原理交互式教程，从机器码到流水线、缓存、系统调用，可视化极佳，适合建立整体认知。

---

## 并发、关键区 & 无锁数据结构 — 必读资料

### 📚 C++ Concurrency in Action（2nd ed）

- **作者：** Anthony Williams · Manning, 2019
- **类型：** 书
- **链接：** <https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition>
- **标签：** `Memory Model` `atomic` `Lock-Free` `ABA`
- **简评：** C++ 并发最权威的书。Ch.5 内存模型与 atomic；Ch.6 有锁数据结构；Ch.7 无锁数据结构（lock-free stack/queue）；Ch.8 并发代码设计与性能。代码全部可编译运行。**无锁必读。**

### 📚 The Art of Multiprocessor Programming（2nd ed）

- **作者：** Herlihy, Shavit, Luchangco, Spear · 2020
- **类型：** 书
- **链接：** <https://www.elsevier.com/books/the-art-of-multiprocessor-programming/herlihy/978-0-12-415950-1>
- **标签：** `Linearizability` `Spinlock` `Lock-Free` `Wait-Free`
- **简评：** 并发理论圣经。线性化、互斥证明、Spinlock 家族、无锁栈/队列/哈希表、Hazard Pointer、RCU 全覆盖。代码用 Java 但原理完全通用。

### 📄 Is Parallel Programming Hard, And If So, What Can You Do About It?

- **作者：** Paul E. McKenney · 持续更新
- **类型：** 免费 PDF
- **链接：** <https://mirrors.edge.kernel.org/pub/linux/kernel/people/paulmck/perfbook/perfbook.html>
- **标签：** `RCU` `Memory Barrier` `Linux Kernel`
- **简评：** RCU (Read-Copy-Update) 的权威参考书，Linux 内核并发模型深度讲解。内存屏障、无锁原语、性能 vs 正确性的工程权衡。

### 🌐 1024cores.net — Dmitry Vyukov's Blog

- **作者：** Dmitry Vyukov（Google, ThreadSanitizer 作者）
- **类型：** 免费网站
- **链接：** <https://www.1024cores.net>
- **标签：** `MPMC Queue` `Hazard Pointer` `SPSC`
- **简评：** 无锁编程实战圣地。SPSC/MPMC Queue、Bounded/Unbounded 实现、内存序实践、Memory Reclamation 方案的代码+分析。**直接有可复现代码。**

### 📄 x86-TSO: A Rigorous and Usable Programmer's Model

- **作者：** Sewell et al. · CACM 2010
- **类型：** 免费论文
- **链接：** <https://www.cl.cam.ac.uk/~pes20/weakmemory/cacm.pdf>
- **标签：** `Memory Model` `TSO` `acquire/release`
- **简评：** 想真正搞清楚 x86 内存序（为什么 acquire/release 足够，seq_cst 何时必要），这篇论文是最严谨的形式化描述。

### 📄 Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms

- **作者：** Michael & Scott · PODC 1996
- **类型：** 论文
- **链接：** <https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf>
- **标签：** `MS Queue` `CAS` `ABA Problem`
- **简评：** Michael-Scott Queue 原始论文，工业界最广泛使用的无锁队列（Java ConcurrentLinkedQueue 即此实现）。CAS + 哑节点的经典设计。

---

## 视频讲座 — 最值得看的演讲

| 标题 | 作者 | 链接 | 要点 |
|---|---|---|---|
| atomic<> Weapons (1 & 2) | Herb Sutter · C++ and Beyond 2012 | [YouTube](https://herbsutter.com/2013/02/11/atomic-weapons-the-c-memory-model-and-modern-hardware/) | C++ 内存模型最好的讲解，happens-before 关系 |
| Live Lock-Free or Deadlock (1 & 2) | Fedor Pikus · CppCon 2015 | [YouTube](https://www.youtube.com/watch?v=lVBvHbJsg5Y) | 无锁编程实战，ABA 问题与解决方案 ⭐ |
| CPU Caches and Why You Care | Scott Meyers · code::dive 2014 | [YouTube](https://www.youtube.com/watch?v=WDIkqP4JbkE) | Cache line、false sharing、预取，直观易懂 |
| C++ atomics, from basic to advanced | Fedor Pikus · CppCon 2017 | [YouTube](https://www.youtube.com/watch?v=ZQFzMfHIxng) | atomic 实现细节，memory_order 选择 |
| Linux Performance Tools | Brendan Gregg · LISA 2014 | [YouTube](https://www.youtube.com/watch?v=FJW8nGV4jxY) | perf、Flamegraph、ftrace 系统性实战 |

---

## 工具 & 可复现代码库

| 名称 | 链接 | 用途 |
|---|---|---|
| Google Benchmark | <https://github.com/google/benchmark> | 标准 microbenchmark 框架，防止编译器优化掉测试代码 |
| Compiler Explorer (Godbolt) | <https://godbolt.org> | 在线查看 C++/Rust 汇编，验证 cmov/inline/SIMD 生成 |
| perf + Flamegraph | <https://www.brendangregg.com/perf.html> | PMU 计数器量化 cache-miss / branch-miss，火焰图定位热点 |
| rigtorp/awesome-lockfree | <https://github.com/rigtorp/awesome-lockfree> | 无锁资源 awesome-list + 高质量 SPSC/MPMC Queue 实现 |
| Facebook Folly | <https://github.com/facebook/folly> | 生产级无锁数据结构参考实现（MPMCQueue, AtomicHashMap 等）|
| Intel VTune Profiler | <https://www.intel.com/content/www/us/en/developer/tools/oneapi/vtune-profiler.html> | Top-Down 微架构分析，可视化 Front-End/Back-End Bound |

---

## 推荐学习路径

```
Step 1：建立硬件直觉
  → Drepper 论文 §3-§4（Cache + TLB）
  → Scott Meyers 视频（CPU Caches and Why You Care）

Step 2：量化问题
  → Denis Bakhvalov 书（Top-Down 方法论 + perf/VTune 实战）
  → 工具：perf stat -e cache-misses,branch-misses

Step 3：理解并发基础
  → Herb Sutter atomic<> Weapons 视频
  → Anthony Williams Ch.5（内存模型 + atomic）

Step 4：无锁数据结构
  → Anthony Williams Ch.6-7（有锁 → 无锁）
  → Fedor Pikus CppCon 2015 视频
  → Michael-Scott Queue 原始论文

Step 5：验证与实践
  → Google Benchmark 写对比测试
  → Godbolt 确认汇编
  → perf 验证 PMU 计数器下降
```

---

## TODO List

### 微架构 & 性能调优

- [ ] 读完 Drepper 论文 §3（CPU Cache）
- [ ] 读完 Drepper 论文 §4（Virtual Memory / TLB）
- [ ] 扫读 Drepper 论文 §6（What Programmers Can Do）
- [ ] 看完 Scott Meyers "CPU Caches and Why You Care" 视频
- [ ] 读 Agner Fog Vol.3 microarchitecture.pdf（重点：BTB、分支预测器结构）
- [ ] 安装并试用 Google Benchmark，跑一个 cache miss 对比实验
- [ ] 用 `perf stat -e cache-misses,branch-misses` 跑自己的代码
- [ ] 用 Godbolt 验证一段 branchless 代码是否生成了 cmov
- [ ] 读 Denis Bakhvalov 书的 Top-Down 分析章节

### 并发 & 无锁

- [ ] 看完 Herb Sutter "atomic<> Weapons" 两部视频
- [ ] 读完 Anthony Williams C++ Concurrency in Action Ch.5（内存模型）
- [ ] 读完 Anthony Williams Ch.6（有锁数据结构）
- [ ] 读完 Anthony Williams Ch.7（无锁数据结构）
- [ ] 看完 Fedor Pikus "Live Lock-Free or Deadlock" CppCon 2015
- [ ] 读 Michael-Scott Queue 原始论文，手写一遍实现
- [ ] 阅读 1024cores.net 的 MPMC Queue 实现
- [ ] 理解 Hazard Pointer 和 Epoch-Based Reclamation 的区别

### 工具熟悉

- [ ] 配置 perf + Flamegraph 工作环境
- [ ] 跑一次 `perf record` + `perf report` 分析一个真实程序
- [ ] 尝试 Intel VTune（或 AMD uProf）的 Top-Down 分析视图

### 进阶（有时间再说）

- [ ] 读 Herlihy & Shavit "The Art of Multiprocessor Programming"
- [ ] 读 McKenney "Is Parallel Programming Hard" RCU 相关章节
- [ ] 读 x86-TSO 论文（理解内存序形式化定义）
- [ ] 了解 AUTOSAR Classic OS 的任务模型和中断分类（Cat.1 / Cat.2）
- [ ] 看 Zephyr RTOS 调度器源码 `kernel/sched.c`
