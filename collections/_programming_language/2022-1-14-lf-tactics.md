---
title: 常见 More Basic Tactics
---

## The `apply` Tactic

1. 需要证明的 goal 与之前假设完全一致时可以使用。

2. 形如 $H:A\rightarrow B$ 的条件假设，将 goal 中需要证明的 $B$ 替换成 $A$ 因为根据 $H$ 如果 $A$ 成立，则 $B$ 也成立。

## The `apply with` Tactic

在证明传递性时，可能会出现下面的情况，我们已经有定理 `trans_eq`，现在希望利用它证明一个简单的例子 `trans_eq_example`。

```coq
Theorem trans_eq : ∀ (X:Type) (n m o : X),
  n = m → m = o → n = o.
Proof.
  intros X n m o eq1 eq2. rewrite → eq1. rewrite → eq2.
  reflexivity. Qed.

Example trans_eq_example' : ∀ (a b c d e f : nat),
     [a;b] = [c;d] →
     [c;d] = [e;f] →
     [a;b] = [e;f].
Proof.
  intros a b c d e f eq1 eq2.
  apply trans_eq with (m:=[c;d]).
  apply eq1. apply eq2.   Qed.
```

Coq 会智能的实例化 `trans_eq` 中的变量，例如 `n:=[a;b]`，`o:=[e;f]`，但对于 `m` 而言，Coq 无法判断。（为什么？）
Coq 会提示用户 `Unable to find an instance for the variable m.` 这时需要将 `apply trans_eq.` 改为 `apply trans_eq with (m:=[c;d])` 即可。

Coq 也提供了这类传递性通用的 tactic 成为 `transitivity` 它也需要提供实例化的变量。

## The `injection` and `discriminate` Tactic

注意到对于一个 `Inductive` 类型，存在下面两个性质。

1. 构造函数是 injective 的，既 $S n = S m\rightarrow n = m$ 。
2. 不同的构造函数是 disjoint 的，既 $0 \neq S n \forall n$ 。

Coq 提供了 `injection` 这个 tactic 来将类型“解构”。
例如，`injection H as H1 H2.` 提供了下面的功能。

$$\begin{align}
H&: (a,b)=(x,y)\\
H_1&: a=x\\
H_2&: b=y
\end{align}$$

`discriminate` 则是当假设中出现形同 $H: S n = 0$ 这种不可能出现的情况时，直接处理掉 goal （反证法）。

## Using Tactics in Hypothesis

tactics 是同样可以修改假设，使用 `tactics ... in H.` 的形式即可。

## Varying the Induction Hypothesis

`intros x y z.` 是将变量从假设中提取到语境中，含义通常为 *For a particular $x$*。
在进行数学归纳法时，可能会出现 $n$ 和 $m$ 两个可以归纳的值，若我们想对 $m$ 进行归纳，会不得不先引入 $n$。
导致出现 *For a particular $n$ and $m$* 这会导致证明卡死，因为两个都是 *particular* 的，我们对其一无所知。
然后进行归纳时，会出现我们试图证明对于所有的 $m$ 都有对于某个 *particular* 的 $n$ 成立的性质，这是不可能的。

因此 Coq 提供了 `generalize dependent n.` 用于将 `intros n.` 提取出去。

## Unfolding Definitions

`unfold function.` 可以将函数展开成原来的样子便于处理，通常可以处理 `simpl.` 无法处理的情况或者用于观察需要 `destruct` 的值。

## `destruct` on Compound Expressions

对于不动点迭代等，经常出现
```coq
func x = match (func x) with
| ... => ...
| ... => ...
end
```

这时我们可以直接采用 `destruct (func x).` 来对 `func x` 可能出现的结果进行证明。
