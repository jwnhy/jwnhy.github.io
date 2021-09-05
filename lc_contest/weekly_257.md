# Leetcode 257 次周赛

## [5863. 统计特殊四元组](https://leetcode-cn.com/problems/count-special-quadruplets/)

### Description

给你一个 下标从 `0` 开始 的整数数组 `nums` ，返回满足下述条件的 不同 四元组 `(a, b, c, d)` 的 数目

- `nums[a] + nums[b] + nums[c] == nums[d]`
- `a < b < c < d`

### Solution

注意数据范围，直接暴力。

```python
class Solution {
public:
    int countQuadruplets(vector<int>& nums) {
        int ans = 0;
        for (int i = 0; i < nums.size(); i++)
            for (int j = i + 1; j < nums.size(); j++)
                for (int k = j + 1; k < nums.size(); k++)
                    for (int l = k + 1; l < nums.size(); l++) {
                        int a = nums[i], b = nums[j], c = nums[k], d = nums[l];
                        if (a + b + c == d)
                            ans++;
                    }
        return ans;
    }
};
```

#### 注意

此处 `a,b,c,d` 是下标，不是元素的值，需要看清题意。

## [5864. 游戏中弱角色的数量](https://leetcode-cn.com/problems/the-number-of-weak-characters-in-the-game/)

### Description

你正在参加一个多角色游戏，每个角色都有两个主要属性：**攻击** 和 **防御** 。给你一个二维整数数组 `properties` ，其中 `properties[i] = [attack_i, defense_i]` 表示游戏中第 i 个角色的属性。

如果存在一个其他角色的攻击和防御等级 都严格高于 该角色的攻击和防御等级，则认为该角色为 弱角色 。更正式地，如果认为角色 `i` 弱于 存在的另一个角色 `j` ，那么 `attack_j > attack_i` 且 `defense_j > defense_i` 。

返回 弱角色 的数量。

### Solution I 排序 `O(nlog(n))`

对第一个元素降序，第二个元素升序，既

$$\{(a_i,b_i)\},\forall i<j,a_i>a_j\vee (a_i = a_j\wedge b_i<=b_j)$$

从前向后遍历，维护 $b_{max}=max_{1..k}(b_k)$，若出现 $a_k<a_0 \wedge b_k > b_{max}$，则为一个有效的解。
分开看两个条件，

- $a_k<a_0$: 跳过所有攻击力和最大元素相等的。
- $b_k>b_{max}$: 假设出现无效解，既 $\exists i$ 使得 $a_i=a_k,b_i>=b_k$。
这与排序的结果相悖，所有满足 $a_i = a_j,b_i>=b_k$ 的元素都在当前元素 $k$ 之后，因此 $b_{max}$ 一定属于一个 $a_i > a_j$ 的元素。

```c++
class Solution {
public:
    int numberOfWeakCharacters(vector<vector<int>>& properties) {
        sort(properties.begin(), properties.end(), [](auto& a, auto& b) -> bool { return a[0] > b[0] || (a[0] == b[0] && a[1] <= b[1]); });
        int atk_max = properties[0][0], def_max = properties[0][1];
        int ans = 0;
        for(auto prop: properties) {
            if (prop[1] < def_max && prop[0] < atk_max)
                ans++;
            else
                def_max = prop[1];
        }
        return ans;
    }
};
```

### Solution II 查表 `O(n)`

制作一张长度为攻击力取值范围 `N` 的表格，其中每个元素为大于或等于该攻击力下的最高防御力，既

$$T_a=max_{a>=a'}(D_{a'}),a',a\in \{1..N\}$$

$D_{a'}$ 意思为攻击力为 $a'$ 的防御力的值的集合。
判断某角色 $(a,d)$ 是否为弱角色，只需要判断是否 $T_{a+1}>d$ 即可。

```c++
#define N 100000
class Solution {
public:
    int numberOfWeakCharacters(vector<vector<int>>& properties) {
        int ans = 0;
        int table[N+1] = {0};
        // 放入最大防御力
        for (auto prop: properties) {
            table[prop[0]] = max(table[prop[0]], prop[1]);
        }
        int tmp = 0;
        // 填充项与项之间的空隙
        for (int i = N; i > 0; i--) {
            tmp = max(tmp, table[i]);
            table[i] = tmp;
        }
        // 判断
        for (auto prop: properties) {
            if (prop[0] < N && prop[1] < table[prop[0] + 1]) {
                ans++;
            }
        }
        return ans;
    }
};
```

## [5865. 访问完所有房间的第一天](https://leetcode-cn.com/problems/first-day-where-you-have-been-in-all-the-rooms/)

### Description

你需要访问 `n` 个房间，房间从 `0` 到 `n - 1` 编号。同时，每一天都有一个日期编号，从 `0` 开始，依天数递增。你每天都会访问一个房间。

最开始的第 `0` 天，你访问 `0` 号房间。给你一个长度为 `n` 且 下标从 `0` 开始 的数组 `nextVisit` 。在接下来的几天中，你访问房间的 次序 将根据下面的 规则 决定：

假设某一天，你访问 `i` 号房间。
如果算上本次访问，访问 `i` 号房间的次数为 奇数 ，那么 第二天 需要访问 `nextVisit[i]` 所指定的房间，其中 `0 <= nextVisit[i] <= i` 。
如果算上本次访问，访问 `i` 号房间的次数为 偶数 ，那么 第二天 需要访问 `(i + 1) mod n` 号房间。
请返回你访问完所有房间的第一天的日期编号。题目数据保证总是存在这样的一天。由于答案可能很大，返回对 $10^9 + 7$ 取余后的结果。

### Solution

动态规划，设 $f(i)$ 为第一次抵达第 $i$ 号房间所需要的天数。抵达第 $i$ 号房间，我们必须先抵达 $i+1$ 个房间**两次**。

这取决于 $next(i-1)$ 的值，若 $next(i-1)=i-1$，则第一次抵达第 $i-1$ 号房间时，直接会访问自己然后到达第 $i$ 号房间。

$$ f(i) = f(i-1) + 2\mbox{ if }next(i-1)=i-1$$

否则，由于 $next(i-1)\in [0,i-1)$ 会跳回之前的房间里（这里发生了一次跳跃），这时可以观察到一个现象。

> 若第一次抵达 $i-1$ 号房间，第 $[0,i-1)$ 号房间一定被访问过**偶数**次(包括 $0$ 次，可看成 $0$ 次)。

那跳回第 $next(i-1)\in [0, i-1)$ 号房间时一定是奇数，现在要处理的问题就是第一次访问 $next(i-1)$ 再回到 $i-1$ 所需要的时间。

那其实就是 $f(i-1)-f(next(i-1))$，是之前循环的一部分，然后再跳一次到第 $i$ 号房间。

那么总的状态转移方程就是

$$f(i)=\begin{cases}
f(i-1) + 2&\mbox{if }next(i-1)=i-1\\
f(i-1) + 2 + f(i-1) - f(next(i-1)) &\mbox{if }next(i-1)<i-1
\end{cases}$$

代入 $next(i-1)=i-1$，简化为
$$f(i) = 2 + 2*f(i-1) -  f(next(i-1))$$

```c++
class Solution {
public:
    int firstDayBeenInAllRooms(vector<int>& nextVisit) {
        long long dp[100002] = {0};
        long long MOD = 1e9 + 7;
        for (int i = 1; i < nextVisit.size(); i++) {
            dp[i] = (2 + 2 * dp[i-1] + MOD - dp[nextVisit[i-1]]) % MOD;
        }
        return (int)dp[nextVisit.size()-1];
    }
};
```