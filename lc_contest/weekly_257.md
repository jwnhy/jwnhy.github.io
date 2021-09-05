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