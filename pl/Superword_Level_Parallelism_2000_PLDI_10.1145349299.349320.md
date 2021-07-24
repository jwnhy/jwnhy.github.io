# [PLDI 2020] Exploiting Superword Level Parallelism with Multimedia Instruction Sets

这一篇博文是阅读 **PLDI** 文章系列的一个开始，希望自己能够坚持下去，第一篇是关于如何利用多媒体支持指令来实现通用程序加速的。

这一篇应该是这一领域的第一篇文章，利用其称为的 `superword` 指令来打包（pack）相似的指令，以减少程序的指令数量。

## 前置知识

这部分介绍编译原理中一些基本概念，也帮助我复习一些早已遗忘的知识（XD。

### 基本块（Basic Block）

**基本块定义：** 

- 在任何位置的任意一条指令总是先于在它之后的指令执行。
- 在两条连续的指令之间没有其它指令会被执行。

该定义允许了分支指令出现在基本块之中，只要它是一条无条件指向下一条语句的分支指令。

### 使用-定义链（Use-Define Chain）

**变量 Define：**

- 当一个变量 $v$​ 出现在赋值语句左侧时即为变量定义，每个变量至少会被其初始化定义一次。

**变量 Use：**

- 当一个变量 $v$ 出现在语句右侧时即为变量使用，若出现在 $s(j)$，且存在定义 $s(i), i<j, i=argmin_i(j-i)$，则 $s(i)$ 为该变量的定义。

**生成过程：**

该论文只考虑了基本块内部的语句打包过程，简单的依靠定义生成即可，不需要考虑跨基本块的 U-D、D-U 链问题。

## 算法过程

算法整体流程分为下面几步，对于一个基本块 $B$ 与 空集合 $P=\empty$，我们进行下面的操作。

1. 查找相邻内存引用：`P=find_adj_ref(B,P)`
2. 利用 U-D、D-U 链扩展已有打包集合：`P=extend_packlist=(B,P)`
3. 打包指令：`P=combine_packs(P)`
4. 指令重排：`B=schedule(B,[],P)`

### 查找相邻内存引用

这个函数比较简单，遍历基本块内的所有语句组合，找到所有可能能够打包的指令。

```c++
PackSet find_adj_ref(BasicBlock B, PackSet P) {
    for (auto s1 : B) {
        for (auto s2 : B) {
            if (s1.has_mem_ref() && s2.has_mem_ref() && adjacent(s1, s2)) {
                int align = get_alignment(s1);
                if (stmt_can_pack(B, P, s1, s2, align))
                    P.add(new pair(s1, s2));
            }
        }
    }
    return P;
}
```

### 检查可否打包

这个函数用于判断两条指令是否可以打包，其判断条件有下面几点。

- 两条指令同构，有着相同的形式。
- 两条指令独立，不存在依赖关系。【如何判断？】
- 左侧指令 $s_1$ 未已经出现在左侧。
- 右侧指令 $s_2$ 未 已经出现在右侧。
- 指令操作数对齐一致。
- 打包后估计的运行时间小于打包前的【避免负优化】。

```c++
bool stmts_can_pack(BasicBlock B, PackSet P, Stmt s1, Stmt s2, int align) {
    if (isomorphic(s1, s2) && independent(s1, s2)) {
        if (not_at_left(P, s1) && not_at_right(P, s2)) {
            int align1 = s1.align();
            int align2 = s2.align();
            if (align1 == NULL || align1 == align) {
                if (align2 == NULL || align2 == align + s2.data_size())
                    return true;
            }
        }
    }
    return false;
}
```

### 扩展可打包指令

这个函数依据 D-U 链与 U-D 链不断扩展已有的打包集合。

```c++
PackSet extend_packlist(BasicBlock B, PackSet P) {
	do {
        PackSet Pprev = P;
        for (auto p: P) {
            P = follow_use_defs(B, P, p);
            P = follow_def_uses(B, P, p);
        }
    } while (Pprev != P)
    return P;
}

PackSet follow_use_defs(BasicBlock B, PackSet P, Pack p) {
    auto [s1, s2] = p;
    int align = s1.align();
    auto [x0 := f(x1,...,xm), y0 := f(y1,...,ym)] = [s1, s2];                // used xj, yj
    for (int j = 1; j <= m; j++) {
        if ( t1 = [xj := f(...)] in B && t2 = [yj := f(...)] in B) {         // find where they were defined
            if (stme_can_pack(B, P, t1, t2, align) && est_saving(t1, t2, P) > 0) {
                P.add(new pair(t1, t2));
                set_alignment(t1, t2, align);
            }
        }
    }
}

PackSet follow_def_uses(BasicBlock B, PackSet P, Pack p) {
    auto [s1, s2] = p;
    auto [x0 := f(x1,...,xm), y0 := f(y1,...,ym)] = [s1, s2];                // defined x0, y0
    int align = s1.align();
    int saving = -1;
    for (auto t1 = [... := g(..., x0, ...)]) {
        for (auto t1 != t2 = [... := h(..., y0, ...)]) {                     // find where they were used
            if (stmt_can_pack(B, P, t1, t2, align) && est_saving(t1, t2, P) > saving) {
                auto u1 = t1, u2 = t2;
                saving = est_saving(t1, t2, P);
            }
        }
    }
    if (saving > 0) {
        P.add(new pair(u1, u2));
        set_alignment(s1, s2, align);
    }
}
```

### 合并包

在扩展过程中，可能会出现一个包 $s_1$ 的末尾正好是另一个包 $s_2$ 的开头，此时便需要执行包合并，以提升性能并避免该指令被执行两次。

```cpp
PackSet combine_packs(PackSet P) {
    do {
        PackSet Pprev = P;
        for (auto p1 = [s1,...,sn]: P) {
            for (auto p2 = [l1,...,ln]: P) {
                if (sn == l1)
                    P = P.remove([p1, p2]);
                	P = P.add([s1,...,sn,l2,...,ln]);
            }
        }
    } while (Pprev != P)
    return P;
}
```

### 语句调度

语句调度比较简单，直接 DAG 搜索语句间依赖关系即可。

## 疑问

1. 原文编译到的目标平台为 AltiVec，且使用的为 SUIF 编译器框架，对于商用 CPU 而言，如何确保可移植性？
2. 对齐分析（Alignment Analysis）到底做了什么工作？
3. 跨基本块的存活分析能否提升该文章的性能？
