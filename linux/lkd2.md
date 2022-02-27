# LKD 读书笔记 Part 2

## LKD Chapter 3 进程调度

### I/O 密集与计算密集

进程分为 I/O 密集与计算密集。

|      | I/O 密集 | 计算密集|
|---------------------------|
|时间片| 短       | 长      |
|主任务| 处理 I/O | 计算    |
|中断  | 密集     | 几乎没有|
|例子  | 文本编辑 | MATLAB  |

### 进程优先级

Linux 中进程优先级分为两个值 `nice` 值与 `rtprio` 值。
前者决定了普通进程的优先级，越大的 `nice` 代表进程优先级越低（对其他进程越 `nice`）。
后者决定了实时进程的优先级，越大的 `rtprio` 代表进程优先级越高。

`nice` 的取值范围是 `-20～+19`，`rtprio` 的取值范围是 `0～99`。

两者分开调度，实时进程总有比普通进程高的优先级。

查看实时优先级

```bash
ps -eo state, uid, pid, ppid, rtprio, time, comm
```

### 时间片

与一般操作系统不同，Linux 不直接对进程分配时间片，而是使用 CFS 确保进程运行时间的**占用比例**。
而 `nice` 值则是他们的占用比例的权重。
例如，有一个文本编辑器和一个视频编码器在同时运行，有着相同的 `nice` 值。
那么他们各自占用 `50%` 的处理器，而文本编辑器只需要在需要的时候才会响应，因此在 CFS 看来，
它总是**未完全使用**自己的时间，因此总是会优先响应它，而视频编码器也可以在文本编辑器未运行的时候充分利用 CPU 资源。

### 传统 UNIX 分配的缺点

传统 UNIX 分配的方式也是通过 `nice` 值决定进程优先级以及时间片的大小，但它有下面的缺点。

1. 每一个 `nice` 值对应的绝对时间片难以确定。
    例如，有两个进程分别有 `0` 和 `+20` 的 `nice` 值，那么他们分配的时间是 `100 ms` 和 `5 ms`。
    但如果是同时有两个进程有 `0` 的 `nice` 值，那么他们分配的时间就会变成 `100 ms` 和 `100 ms`。
    相似的，如果同时有两个进程有 `+20` 的 `nice` 值，那么他们分配的时间就会变成 `5 ms` 和 `5 ms`。
    前者导致响应时间长，后者导致频繁的上下文切换。

2. `nice` 值之间的差值不一致，`nice` 值为 `0` 和 `1` 的进程分配的时间片可能是 `100 ms` 和 `95 ms`。
    相差无几，但 `nice` 值为 `18` 和 `19` 的可能是 `10 ms` 和 `5 ms` ，相差一倍。

3. 时间片必须是时钟的整数倍。

4. 对于刚唤醒进程的处理，系统常常会使得**刚唤醒**的进程有更高的优先级，即使它们的时间片已经用光。
   这对交互式程序的体验有好处，但无疑影响了公平性。

### CFS 调度

一个理想的调度算法，在不考虑上下文切换的开销的情况下，会希望**所有**任务在很小的 $\epsilon$ 时间内都运行一遍。
当然这在现实是不可能做到的，因为存在上下文开销。但是 CFS 会尝试接近这个理想模型，CFS 设置了一个 **targeted latency**。
也就是上面说的 $\epsilon$，当存在 $n$ 个任务时，每个任务所占用的时间是 $\frac{\epsilon}{n}$。值得注意的是，
如果 $n\rightarrow \infty$，会导致上下文切换的次数趋于无穷。因此 CFS 设置了一个最小粒度，默认为 `1 ms`。
而 `nice` 值在 CFS 算法中被用来计算每个任务应当占用的时间权重，CFS 会不断试图接近**理想**的分配方案，
它总是选择已经占用最少的任务进行执行。

> 与[这个问题](https://jwnhy.github.io/misc/ad_problem.html)十分类似。

### CFS 调度实现

CFS 算法分为下面几个部分。

- 时间统计
- 进程选择
- 调度器入口
- 休眠与唤醒

#### 时间统计

##### 调度对象结构体

CFS 并没有时间片的概念，但它仍然需要记录每个进程运行的时间来确保它们只运行公平的时间。
CFS 在 `<linux/sched.h>` 中引入了 `struct sched_entity`

```c
struct sched_entity {
  struct load_weight load;
  struct rb_node run_node;
  struct list_head group_node;
  unsigned int on_rq;
  u64 exec_start;
  u64 sum_exec_runtime;
  u64 vruntime; // accounting the current running time
  u64 prev_sum_exec_runtime;
  u64 last_wakeup;
  u64 avg_overlap;
  u64 nr_migrations;
  u64 start_runtime;
  u64 avg_wakeup;
  /* many stat variables elided, enabled only if CONFIG_SCHEDSTATS is set */
}
```

该结构体直接嵌入 `task_struct` 中，成员名为 `se`。

##### `vruntime`

`vruntime` 指当前进程运行的时间经过 `nice` 值作为权重标准化之后的值。
更新它的函数是定义在 `<linux/sched_fair.c>` 中的 `update_curr(struct cfs_rq *cfs_rq)`。
它的步骤如下。

- 获取当前时间

  ```c
  u64 now = rq_of(cfs_rq)->clock;
  ```

- 计算差值

  ```c
  delta_exec = (unsigned long) (now - curr -> exec_start);
  ```

- 更新 `vruntime`

  ```c
  __update_curr(cfs_rq, curr, delta_exec);
  ```

- 更新开始时间

  ```c
  curr->exec_start = now;
  ```

具体加权求值的方式在 `__update_curr` 中。

```c
static inline void
__update_curr(struct cfs_rq *cfs_rq, struct sched_entity *curr,
unsigned long delta_exec)
{
  unsigned long delta_exec_weighted;

  schedstat_set(curr->exec_max, max((u64)delta_exec, curr->exec_max));

  curr->sum_exec_runtime += delta_exec;
  schedstat_add(cfs_rq, exec_clock, delta_exec);
  delta_exec_weighted = calc_delta_fair(delta_exec, curr);

  curr->vruntime += delta_exec_weighted;
  update_min_vruntime(cfs_rq);
}
```

#### 进程选择

之前讨论过，在一个理想的多任务系统中，每个进程的 `vruntime` 都应该是一样的。
CFS 做不到这一点，因此它采用了一种很简单的方法。

> 每次挑选 `vruntime` 最小的进程。

CFS 使用红黑树 `rbtree` 来管理可运行的进程列表，能够快速的寻找到 `vruntime` 最小的进程。

进程选择的代码如下，值得注意的是，Linux 并没有真的遍历整颗树，而是缓存了**最左节点**。

```c
static struct sched_entity *__pick_next_entity(struct cfs_rq *cfs_rq)
{
  struct rb_node *left = cfs_rq->rb_leftmost;
  if (!left)
    return NULL;
  return rb_entry(left, struct sched_entity, run_node);
}
```
