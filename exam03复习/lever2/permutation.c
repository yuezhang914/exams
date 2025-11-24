/* permutations_fixed.c
 * 对原始两个版本的改进与合并（以第二版为基础，做了拷贝排序、内存检查与重复检测）
 * 仅使用被允许的函数（malloc, calloc, free, write），移除了未使用的 stdio 头文件
 */

#include <unistd.h> /* write 函数声明 */
#include <stdlib.h> /* malloc, calloc, free */

/* 检测字符串中是否存在重复字符
 * 返回 1 表示有重复，返回 0 表示没有重复
 */
int has_dup(const char *str)
{
    int i;
    int j;

    i = 0;                         /* 外层索引从 0 开始 */
    while (str[i] != '\0')         /* 遍历每个字符 */
    {
        j = i + 1;                 /* 内层从 i+1 开始比较 */
        while (str[j] != '\0')     /* 比较 i 与后续所有字符 */
        {
            if (str[i] == str[j])  /* 如果发现相等则存在重复 */
                return 1;
            j = j + 1;             /* 内层索引递增 */
        }
        i = i + 1;                 /* 外层索引递增 */
    }
    return 0;                      /* 没有发现重复 */
}

/* 计算字符串长度（简单的实现，等价于 strlen） */
int ft_strlen(const char *str)
{
    int len;

    len = 0;                       /* 初始化计数为 0 */
    while (str[len] != '\0')       /* 直到遇到 NUL 终止符 */
        len = len + 1;
    return len;                    /* 返回长度 */
}

/* 就地排序字符串 s（升序），使用简单的交换排序实现 */
void sort_string(char *s)
{
    int i;
    int j;
    char tmp;

    if (!s)                        /* 保护性检查：若 s 为 NULL 则直接返回 */
        return;
    i = 0;
    while (s[i] != '\0')           /* 外层循环 i 从 0 到 len-1 */
    {
        j = i + 1;                 /* j 从 i+1 开始 */
        while (s[j] != '\0')       /* 内层循环比较并在必要时交换 */
        {
            if (s[i] > s[j])       /* 若 s[i] 大于 s[j] 则交换 */
            {
                tmp = s[i];
                s[i] = s[j];
                s[j] = tmp;
            }
            j = j + 1;             /* j++ */
        }
        i = i + 1;                 /* i++ */
    }
}

/* 回溯生成排列并输出
 * src: 已排序的源字符串（只读）
 * res: 构造结果缓冲，长度至少为 len + 1
 * used: 标志数组，len 个 int，0 表示未用，1 表示已用
 * len: 字符串长度
 * depth: 当前递归深度（从 0 到 len）
 */
void generate(const char *src, char *res, int *used, int len, int depth)
{
    int i;

    if (depth == len)              /* 如果构建完成一个排列 */
    {
        /* 输出当前排列（写 len 字节），然后输出换行 */
        write(1, res, len);
        write(1, "\n", 1);
        return;
    }

    i = 0;
    while (i < len)                /* 遍历每个可能的位置 i */
    {
        if (used[i] == 0)          /* 如果 src[i] 尚未被使用 */
        {
            used[i] = 1;           /* 标记为已使用 */
            res[depth] = src[i];   /* 将该字符放到当前结果位置 */
            generate(src, res, used, len, depth + 1); /* 递归处理下一个位置 */
            used[i] = 0;           /* 回溯：恢复标记为未使用 */
        }
        i = i + 1;                 /* i++ */
    }
}

/* 主程序：参数检查 -> 重复检测 -> 内存分配 -> 拷贝与排序 -> 生成排列 -> 释放资源 */
int main(int ac, char **av)
{
    int len;
    char *src_copy;
    char *res;
    int *used;
    int i;

    /* 参数检查：必须只有一个参数（程序名之外） */
    if (ac != 2)
        return 1;

    /* 对原始 argv[1] 进行重复字符检查（如果题目要求拒绝重复则返回错误） */
    if (has_dup(av[1]))
        return 1;

    /* 计算长度并分配内存 */
    len = ft_strlen(av[1]);

    /* 申请 src_copy（len + 1）用于保存源字符串的可修改副本（便于排序，不修改 argv） */
    src_copy = (char *)malloc((len + 1) * sizeof(char));
    if (!src_copy)
        return 1;

    /* 拷贝字符串到 src_copy */
    i = 0;
    while (i < len)
    {
        src_copy[i] = av[1][i];
        i = i + 1;
    }
    src_copy[len] = '\0';          /* 终止符 */

    /* 对副本进行就地排序，保证输出按字典序 */
    sort_string(src_copy);

    /* 为结果缓冲申请内存（len + 1），并为 used 标志数组分配内存并初始化为 0（使用 calloc） */
    res = (char *)malloc((len + 1) * sizeof(char));
    if (!res)
    {
        free(src_copy);
        return 1;
    }

    used = (int *)calloc(len, sizeof(int));
    if (!used)
    {
        free(src_copy);
        free(res);
        return 1;
    }

    /* 调用回溯生成排列并输出 */
    generate(src_copy, res, used, len, 0);

    /* 释放内存资源并返回成功 */
    free(src_copy);
    free(res);
    free(used);
    return 0;
}
