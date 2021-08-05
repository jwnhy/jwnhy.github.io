# Untyped Lambda Calculus

在 1960 年，Peter Landin 发现大多数复杂编程语言可以被理解成一个极小的演算方式。这种被称为 λ-演算的形式化方法于 1920 年代被Church 提出。与之类似的还有由 Milner, Parrow, Walker 提出的 Pi-演算等不同的演算系统。

## 语法

λ-演算的语法非常简单，只由三种基础结构构成。
\[
\begin{align}
t::=&\\\\
    &x\\\\
    &\lambda x.t\\\\
    &t\ t
\end{align}
\]


分别代表变量，函数抽象与函数调用。

## 变量作用域

在 λ-演算中一个非常重要的概念就是变量作用域，当一个变量 $x$ 出现在函数抽象 $\lambda x.t$ 中时，我们称其被该抽象 **绑定 bound**。同样的我们可以称 $\lambda x$ 是一个具有 $t$​ 作用域的 **binder** 。

一个没有被绑定的变量就是**自由 free**的，一个不包含任何自由变量的 *term* 是 **闭合 closed** ，例如 $\text{id}=\lambda x.x$

## 求值方式

λ-演算具有不同的求值方式，在求值过程中，可以被进一步简化的式子被称为 **可简化表达式 reducible expression**。

- *full beta-reduction*: any redex may be redex at anytime，任何可简化表达式都可以在任何时候被简化
  1. $\text{id}(\text{id}(\lambda z. \text{id}z))$​
  2. $\text{id}(\text{id}(\lambda z. z))$​
  3. $\text{id}(\lambda z. z)$
  4. $\lambda z. z$
- *normal order*: leftmost, outermost redex is always reduced first，最左侧，最外侧的可简化表达式总是先被简化。
  1. $\text{id}(\text{id}(\lambda z. \text{id}z))$​
  2. $\lambda z. \text{id}z$​
  3. $\lambda z.z$
- *call by name*: no reduction inside abstraction，不允许函数抽象内部的可简化表达式被简化。
  1. $\text{id}(\text{id}(\lambda z. \text{id}z))$​
  2. $\lambda z. \text{id}z$
- *call by value*: only outermost redex is reduced and a redex is reduced only when its *rhs* has already been reduced to a *value*，只允许最外侧的可简化表达式被简化且其右侧的表达式已经无法进一步被简化。
  1. $\text{id}(\text{id}(\lambda z. \text{id}z))$​
  2. $\lambda z. \text{id}z$

## 利用 λ-演算 编程

尽管 λ-演算 的定义极为简单，但是它可以被发展成一个及其强大的编程语言。

### 多参数函数

$f = \lambda(x,y)\Leftrightarrow f= \lambda x.\lambda y. s$​

将一个具有多个参数的函数转换成一个接受一个参数返回一个接受一个参数的函数的高阶函数的技巧被称为 **柯里化 currying**

### 布尔值

在 λ-演算 中，布尔值可以被表示为
$$
\text{tru} = \lambda t.\lambda f. t\\
\text{fls} = \lambda t.\lambda f. f
$$
可以利用下面的 $\text{test}$ 抽象实现与 `if` 类似的效果。
$$
\text{test} = \lambda l.\lambda m.\lambda n. l\ m\ n
$$
例如
$$
\text{test tru v w} = \text{v}
$$
类似的布尔值演算还有
$$
\text{and}=\lambda b. \lambda c. b\ c\ \text{fls}\\
\text{or}=\lambda b. \lambda c. b\ \text{tru}\ c\\
\text{not}=\lambda b. \text{fls tru}
$$

### 二元组 Pair

有了布尔值之后我们就可以设计二元组结构。
$$
\text{pair} = \lambda f.\lambda s.\lambda b.b\ f\ s\\
\text{fst} = \lambda p.p\ \text{tru}\\
\text{snd} = \lambda p.p\ \text{fls}
$$

### 丘奇数 Church Numeral

自然数可以利用一个函数的施加次数表示，这被称为丘奇数。
$$
c_0 = \lambda s.\lambda z.z \\
c_1 = \lambda s.\lambda z.s\ z \\
c_2 = \lambda s.\lambda z.s\ s\ z
$$
常见计算有
$$
\begin{align}
\text{scc}  &= \lambda n.\lambda s.\lambda z.s(n\ s\ z)\\
     &= \lambda n.\lambda s.\lambda z.n(s\ (s\ z))\\
\text{plus} &= \lambda m.\lambda n. \lambda s. \lambda z.m\ s\ (n\ s\ z)\\
\text{times}&= \lambda m. \lambda n. m\ (plus\ n)\ c_0\\
     &= \lambda m.\lambda n. \lambda s. \lambda z. m\ (n\ s\ z)\ z\\
\text{exp}  &= \lambda m.\lambda n. n\ m\\
\text{iszro}&= \lambda m.m(\lambda x.\text{fls})\ \text{tru}
\end{align}
$$
比较复杂的是丘奇数的前缀函数 $pred$ ，既返回前一个数，其实现如下
$$
\begin{align}
\text{zz}&=\text{pair}\ c_0\ c_0\\
\text{ss}&=\lambda p.\text{pair}\ (\text{snd}\ p)(\text{plus}\ c_1(\text{snd}\ p))\\
\text{prd}&=\lambda m.\text{fst}(m\ \text{ss}\ \text{zz})
\end{align}
$$
原理，由 $\text{zz}$ 开始，两个参数 $c_0$​ 分别递增，其中 $fst$ 比 $snd$ 小一，当执行了 $m$ 次之后，第一个参数为 $m-1$，第二个参数为 $m$，此时取出第一个参数即可。

### 递归

不能进一步求值的表达式被称为 *normal form*，而存在一种表达式，其无法被求值到 *normal form*，他们是不收敛的。
$$
\text{omega} = (\lambda x.x\ x)\ (\lambda x.x\ x)
$$
我们可以进一步利用这一点，定义出**不动点结合子 fixed-point combinator**。
$$
\text{fix} = \lambda f.(\lambda x.f\ (\lambda y.x\ x\ y))\ (\lambda x.f\ (\lambda y.x\ x\ y))
$$
由此我们可以定义出递归求阶乘的函数。
$$
\begin{align}
\text{g}&=\lambda \text{fct}.\lambda n. \text{iszro n}\ c_1\ (\text{times}\ n\ \text{fct}(\text{prd}\ n))\\
\text{factorial} &= \text{fix g}
\end{align}
$$
求值过程
$$
\begin{align}
&\ \ \ \ \ \text{factorial }c_3\\
&=\text{fix g }c_3\\
&=(\lambda x.\text{g}\ (\lambda y.x\ x\ y))\ (\lambda x.\text{g}\ (\lambda y.x\ x\ y))\ c_3\\
&=\text{h h } c_3\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \text{we evaluate leftmost h}\\
&=\text{g }(\lambda y.h\ h\ y)\ c_3
\end{align}
$$
可以看出已经完成了自复制。

## 形式化语法

TODO.