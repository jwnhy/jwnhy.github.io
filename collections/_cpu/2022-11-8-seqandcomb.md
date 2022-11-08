---
title: 搭建自己的 CPU / 组合逻辑与时序逻辑
---

## 组合逻辑

组合逻辑指那些**不**包含存储能力的电路。由于它们并不具有存储功能，因此可以被表示
成布尔表达式的形式。

```scala
out := in_a & in_b
```

### 解码器

解码器将一个 $n$ 位的输入转换成一个 $m$ 位的信号输出，其中 $m\leq 2^n$。
下表为一个 4 位解码器的输入与对应输出。

| 输入 | 输出 |
|:----:|:----:|
|  00  | 0001 |
|  01  | 0010 |
|  10  | 0100 |
|  11  | 1000 |

一个解码器可以用下面的 Chisel 代码描述

```scala
/*sel: 输入
  result: 输出*/

/* Verbose Version */
val result = 0.U
switch (sel) {
  is ("b00".U) { result := "b0001".U}
  is ("b01".U) { result := "b0010".U}
  is ("b10".U) { result := "b0100".U}
  is ("b11".U) { result := "b1000".U}
}
/* Simple Version */
result := 1.U << sel
```

解码器可以与 AND 门组合构成 Mux 选择器，也可以在处理器与内存的通信时用于解码地址
信号。

### 编码器

编码器执行与解码器相反的操作，用于将一个 $m$ 位的信号输入编码为一个 $n$ 位的数字输出。
下表位一个 4 位编码器的真值表。

| 输入 | 输出 |
|:----:|:----:|
| 0001 |  00  |
| 0010 |  01  |
| 0100 |  10  |
| 1000 |  11  |
| ???? |  ??  |

对于小的编码器，我们可以用与解码器类似的思路进行硬编码，然而对于大规模的编码器，
这样显然不可取，因此我们可以利用 Scala 的循环来生成编码器。

```scala
/*hotIn: 输入
  v: 临时存储
  encOut: 输出*/
val v = Wire(Vec (16, UInt (4.W)))
v(0) := 0.U
for (i <- 1 until 16) {
  v(i) := Mux(hotIn(i), i.U, 0.U) | v(i - 1)
}
val encOut = v(15)
```

上面的式子中，由于 `hotIn` 为 one-hot 编码，只有一位非零，导致结果 `v` 中也只有
一个元素非零，我们只要将 `v` 的各个元素以 OR 连接即可找到该非零元素。

$$
\exists! i, hotIn[i]\neq 0 \Rightarrow \exists! v[i]\neq 0
$$

### 裁决器

裁决器负责将一个 $n$ 位信号源转换成 one-hot 编码，一个公平的裁决器需要使用寄存器
来记住自己上次选择的信号，在这里，我们假定**低位**信号具有优先权。

对于小型的裁决器，我们可以利用类似的硬编码思路进行编程。

```scala
val grant = WireDefault ("b0000".U(3.W))
switch (request) {
  is ("b000".U) { grant := "b000".U}
  is ("b001".U) { grant := "b001".U}
  is ("b010".U) { grant := "b010".U}
  is ("b011".U) { grant := "b001".U}
  is ("b100".U) { grant := "b100".U}
  is ("b101".U) { grant := "b001".U}
  is ("b110".U) { grant := "b010".U}
  is ("b111".U) { grant := "b001".U}
}
```

对于大型裁决器，就需要用到循环来生成裁决器的电路了。

```scala
/*grant(i) == true: i 获得了权限
  notGranted(i) == true: 权限还未给予 0~i 种任何一个*/
val grant = VecInit.fill(n)(false.B)
val notGranted = VecInit.fill(n)(false.B)
grant (0) := request (0)
notGranted (0) := !grant (0)
for (i <- 1 until n) {
  grant(i) := request(i) && notGranted (i -1)
  notGranted (i) := !grant(i) && notGranted (i -1)
}
```

> 似乎这里可以 `val notGranted = false.B`，因为我们每次只需要记录是否已被 grant，在
> 生成的电路层面应当两者等价，但程序会更清晰？

## 组合逻辑：数码管输出
