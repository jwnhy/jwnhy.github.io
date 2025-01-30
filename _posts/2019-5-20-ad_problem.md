---
title: 一道有趣的面试题，广告推送问题
---

**描述** 页面上有两个广告位，有 4 个广告需要展示（\\(abcd\\)），要实现一个算法输出需要展示的一对广告：
要求：

1. 输出的两个广告不能相同
2. 多次调用，最后平均而言输出四个广告的概率为 \\(a:b:c:d = 1:2:3:4\\)

## Solution I

利用机器学习的思路，多退少补，分为三个步骤。

1. Init: \\(exist = [0,0,0,0], ideal = [.1,.2,.3,.4]\\)
2. Draw: \\(k_1 = argmax_k(exist[k]-ideal[k]), k_2 = argmax_k(exist[k]-ideal[k]),\text{except }k_1\\)
3. Update: \\(exist[k_{1,2}] = (exist[k_{1,2}] * total + 1)/(total + 1)\\)

这种方法直觉上是收敛的，也确实收敛，应该证明挺简单的。

## Solution II

构造法，我们构造一个 \\(S = \\{(s_1, s_2)\in \\{a,b,c,d\\}^2|s_1\neq s_2\\}\\) 且出现的 \\(a:b:c:d=1:2:3:4\\)。
每次从池子中均匀抽取一个广告对即可。

基本思路，我们考虑满足该比例的池子，例如，\\(P = \\{a,b,b,c,c,c,d,d,d,d\\}\\)。我们如果需要从中构造 \\(S\\) 只需要将其不重复的两两配对即可。
可以通过每次取出出现次数最少的广告和出现次数最多的广告进行配对，即可在尽可能防止重复的条件下满足频率约束。

**Remark**
注意到该算法需要出现次数最多的元素的数量小于其他元素的数量的和。否则一定会出现重复。

$$|s| < \sum_{p\in P,p\neq s}|p|,s = argmax(|s|)$$

一个显然的构造是 \\(\\{ad,bc,bd,cd,cd\\}\\)，当然也有其他的，但注意到该构造形式没有满足出现所有类型的组合，例如 \\(ac\\) 就没有出现。
如果要满足所有组合，可以考虑下面的算法。

1. Init: 构造六个 \\(P = \\{a,b,b,c,c,c,d,d,d,d\\}\\)，分别从中提前取出需要的全排列既 \\(\\{ab,ac,ad,bc,bd,cd\\}\\)。
2. Pair: 对已经构造好全排列的六个池子分别配对，在尽可能防止重复的条件下满足频率约束。
3. Aggregate: 将六个池子合并，并加入全排列。

代码如下：

```python
#!/usr/bin/python
pool = list('abbcccdddd')
combs = ['ab', 'ac', 'ad', 'bc', 'bd', 'cd']
final_pool = []
for comb in combs:
    tp = pool.copy()
    tp.remove(comb[0])
    tp.remove(comb[1])
    print(tp, comb)
    unique = []
    tp = {x: tp.count(x) for x in tp}
    while max(tp.values()) > 0:
        maxi = max(filter(lambda x: x[1] != 0,tp.items()), key=lambda x: x[1])
        tp[maxi[0]] -= 1
        mini = min(filter(lambda x: x[1] != 0 and x[0] != maxi[0],tp.items()), key=lambda x: x[1])
        tp[mini[0]] -= 1
        print(maxi, mini)
        unique += [f"{maxi[0]}{mini[0]}"]
    final_pool = final_pool + unique + [comb]
print(final_pool)
```
