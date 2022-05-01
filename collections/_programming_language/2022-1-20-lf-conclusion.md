---
title: 逻辑基础小结
---

从第一次了解到 Software Foundation 开始，已经过去了好几个年头，而我终于把它的第一卷 Logic Foundation 读完了。

现在看来其实 LF 也并没有那么长，只是我的耐心不够罢了。

下面是一些小小的总结和感受

## Logic in Coq

在 Coq 中，留待证明的对象也是具有类型的，并且属于 `first class value` ，这种类型被称之为 `Prop`。所有的 `Prop` 无论可否证明，都在 Coq 里属于合法的对象。

与一般数学里的 **命题** 不同的是，Coq 中的 `Prop` 是允许携带参数的，也就是 `parameterized proposition`。

```coq
Definition is_three (n: nat) : Prop :=
    n = 3.
Definition is_three : nat -> Prop :=
    fun n:nat => n=3.
(* Even "=" is a parameterized proposition *)
Check @eq : forall A: Type, A -> A -> Prop
```

## 证明中的存在量词

在证明中，常常会出现 $\exists$ 存在量词，它出现的位置可能有两种，目标或者假设中。

#### 出现在目标中的存在量词

对于出现在目标中的 $\exists$ ，我们直接提供 **存在** 的对象即可。

```coq
Definition Even x := exists n : nat, x = double n.
Lemma four_is_even : Even 4.
Proof.
  unfold Even. 
  exists 2. 
  reflexivity.
Qed.
```

#### 出现在假设中的存在量词

对于出现在假设中的 $\exists$，我们使用 `destruct` 来获得一个满足条件的对象。

```coq
Theorem exists_example_2 : forall n,
  (exists m, n = 4 + m) ->
  (exists o, n = 2 + o).
Proof.
  intros n [m Hm]. (* note implicit destruct here *)
  ∃ (2 + m).
  apply Hm. Qed.
```

## 定理的实例化

对于含有全称量词 $\forall$ 的定理，我们可以传入参数或者使用 `specialize` 使其实例化为我们需要的值。

```coq
Lemma in_not_nil_42_take4 :
  forall l : list nat, In 42 l -> l <> [].
Proof.
  intros l H.
  apply (in_not_nil nat 42). (* here *)
  apply H.
Qed.
```

## Inductive Proposition

Coq 允许归纳定义命题，一般用于构造递归的 **命题** 或者 **关系**。

例如

```coq
Inductive le (n: nat) : nat -> Prop :=
| le_n: n <= n
| le_S: forall m: nat, n <= m -> n <= S m.
```

## Proof Objects

根据 Curry-Howard Correspondence，我们可以将 **命题** 看成 **类型**。将 **正确的命题** 看成 **合法的类型**，即可以被构造出来的类型。

而 **证明** 可以被看成 **数据**，当我们尝试证明一个东西的时候，我们实际上在构造一个 **证明树**，来构造出我们想要的证明。

例如

```coq
Inductive ev: nat -> Prop
| ev_0: ev 0 (* ev_0 is a proof of (ev 0) *)
| ev_SS: forall n, ev n -> ev (S (S n)).
```

`ev` 是一个归纳定义的类型，首先其规定，`ev_0` 是一个具有 `ev 0` **类型** 的对象。其中 `ev 0` 是一个 `Prop` 也就是命题。

那么很多时候，我们的证明方式可以从 Coq 一般的**证明** 变成直接构造出具有想要证明的命题的类型的数据。

## Imp

Imp 章节为整卷做了很好的总结，非常 Amazing！
