#include <stdio.h>   /* 提供 printf、NULL 等（调试/验证可用） */
#include <stdlib.h>  /* 提供 malloc、free、atoi 等 */

/* 检查从 argv[1] 到 argv[ac-1] 每个字符串是否为合法整数表示 */
int check_int(char **str, int ac)
{
    int j;          /* 外层索引：参数索引，从 1 到 ac-1 */

    j = 1;
    while (j < ac)  /* 遍历每个参数字符串 */
    {
        int i;      /* 字符串内部索引 */
        i = 0;
        /* 允许可选的前导符 '+' 或 '-'，若存在则跳过一位 */
        if (str[j][i] == '-' || str[j][i] == '+')
            i = i + 1;
        /* 去掉可选符号后必须至少有一位数字，否则视为非法 */
        if (str[j][i] == '\0')
            return 0; /* 非法：只有符号没有数字 */
        /* 检查剩余字符是否全部为数字字符 '0'..'9' */
        while (str[j][i])
        {
            if (str[j][i] < '0' || str[j][i] > '9')
                return 0; /* 非数字字符 -> 非法 */
            i = i + 1;
        }
        j = j + 1; /* 检查下一个参数 */
    }
    return 1; /* 所有参数都是合法整数表示 */
}

/* 回溯函数：枚举 nums 的子集并打印那些和为 target 的子集 */
void solve(int *nums, int size, int target, int *subset, int index, int sub_size, int curr_sum)
{
    if (index == size) /* 已经处理完所有元素 */
    {
        if (curr_sum == target) /* 当前子集和等于目标，打印它 */
        {
            int i;
            i = 0;
            while (i < sub_size) /* 逐个打印子集元素，用空格分隔 */
            {
                if (i > 0)
                    printf(" ");
                printf("%d", subset[i]);
                i = i + 1;
            }
            printf("\n"); /* 打印完一组解后换行 */
        }
        return; /* 返回上一层探索其它分支 */
    }
    /* 先探索不包含 nums[index] 的分支 */
    solve(nums, size, target, subset, index + 1, sub_size, curr_sum);
    /* 再探索包含 nums[index] 的分支 */
    subset[sub_size] = nums[index]; /* 把当前元素加入到子集中 */
    solve(nums, size, target, subset, index + 1, sub_size + 1, curr_sum + nums[index]);
}

/* 主程序：参数检查 -> 解析整数数组 -> 分配 subset -> 调用 solve -> 释放内存 */
int main(int ac, char **av)
{
    if (ac < 3)            /* 需要至少 target + 1 个数字作为候选 */
        return 1;

    if (!check_int(av, ac)) /* 检查参数均为整数格式 */
        return 1;

    int target;
    target = atoi(av[1]);  /* 解析目标和 */

    int n;
    n = ac - 2;            /* 候选数字的个数 */

    int *nums;
    nums = (int *)malloc(sizeof(int) * n); /* 分配 nums 数组 */
    if (!nums)             /* 分配失败则退出 */
        return 1;

    int i;
    i = 0;
    while (i < n)          /* 逐个把字符串参数转换为整数并写入 nums */
    {
        nums[i] = atoi(av[i + 2]);
        i = i + 1;
    }

    int *subset;
    subset = (int *)malloc(sizeof(int) * n); /* 为构造子集分配缓冲 */
    if (!subset)          /* 若分配失败，释放 nums 并退出 */
    {
        free(nums);
        return 1;
    }

    /* 调用回溯函数打印所有满足条件的子集 */
    solve(nums, n, target, subset, 0, 0, 0);

    /* 清理资源并退出 */
    free(nums);
    free(subset);
    return 0;
}
