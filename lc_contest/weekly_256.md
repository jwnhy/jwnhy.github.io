# [5854. 学生分数的最小差值](https://leetcode-cn.com/problems/minimum-difference-between-highest-and-lowest-of-k-scores/)

## Description

给你一个 下标从 `0` 开始 的整数数组 `nums` ，其中 `nums[i]` 表示第 `i` 名学生的分数。另给你一个整数 `k` 。

从数组中选出任意 `k` 名学生的分数，使这 `k` 个分数间 **最高分** 和 **最低分** 的 **差值** 达到 **最小化** 。

返回可能的 **最小差值** 。

## Solution

排序后遍历长度为 `k` 的序列，取序列头和序列尾插值最小的即可。

```c++
class Solution {
public:
    int minimumDifference(vector<int>& nums, int k) {
        if (nums.size() == 1)
            return 0;
        std::sort(nums.begin(), nums.end());
        auto minimal = 0xffffffffffffffff;
        for (int i = 0; i < nums.size() - k + 1; i++) {
            minimal = nums[i+k-1] - nums[i] < minimal ? nums[i+k-1] - nums[i] : minimal;
        }
        return minimal;
    }
};
```

# [5855. 找出数组中的第 K 大整数](https://leetcode-cn.com/problems/find-the-kth-largest-integer-in-the-array/)

## Description

给你一个字符串数组 `nums` 和一个整数 `k` 。`nums` 中的每个字符串都表示一个不含前导零的整数。

返回 `nums` 中表示第 `k` 大整数的字符串。

注意：重复的数字在统计时会视为不同元素考虑。例如，如果 `nums` 是 `["1","2","2"]`，那么 `"2"` 是最大的整数，`"2"` 是第二大的整数，`"1"` 是第三大的整数。

## Solution

可以利用 C++ 标准库中的 `nth_element` 函数配合一个自定义的比较函数 `cmp` 解决。

```c++
class Solution {
public:
    static bool cmp(string& a, string& b)
    {
        if (a.size() > b.size())
            return true;
        else if (a.size() < b.size())
            return false;
        for (int i = 0; i < a.size(); i++) {
            if (a.at(i) < b.at(i))
                return false;
            else if (a.at(i) > b.at(i))
                return true;
        }
        return false;
    }
    
    string kthLargestNumber(vector<string>& nums, int k) {
        std::nth_element(nums.begin(), nums.begin()+k-1, nums.end(), cmp);
        return nums.at(k-1);
    }
};
```

### 注意

在 C++ Reference 中，对于 `nth_element` 描述如下。

> More formally, nth_element partially sorts the range `[first, last)` in ascending order so that the condition `!(*j < *i)` or `comp(*j, *i) == false` is met for any i in the range `[first, nth)` and for any j in the range `[nth, last)`.

因此对于相同元素应该默认返回 `false`，也如题意所示，否则在一些情况下会导致 `nth_element` 越界。

# [5856. 完成任务的最少工作时间段](https://leetcode-cn.com/problems/minimum-number-of-work-sessions-to-finish-the-tasks/)

## Description

你被安排了 `n` 个任务。任务需要花费的时间用长度为 `n` 的整数数组 `tasks` 表示，第 `i` 个任务需要花费 `tasks[i]` 小时完成。一个 工作时间段 中，你可以 **至多** 连续工作 `sessionTime` 个小时，然后休息一会儿。

你需要按照如下条件完成给定任务：

如果你在某一个时间段开始一个任务，你需要在 **同一个** 时间段完成它。
完成一个任务后，你可以 **立马** 开始一个新的任务。
你可以按 **任意顺序** 完成任务。
给你 `tasks` 和 `sessionTime` ，请你按照上述要求，返回完成所有任务所需要的 **最少** 数目的 工作时间段 。

测试数据保证 **sessionTime** 大于等于 **tasks[i]** 中的 **最大值** 。