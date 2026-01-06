/* permutations_fixed.c
 * 对原始两个版本的改进与合并（以第二版为基础，做了拷贝排序、内存检查与重复检测）
 * 仅使用被允许的函数（malloc, calloc, free, write），移除了未使用的 stdio 头文件
 */
// Assignment name : permutations

// Expected files : *.c *.h
// Allowed functions : puts, malloc, calloc, realloc, free, write
// ---------------------------------------------------------------

// Write a program that will print all the permutations of a string given as argument.

// The solutions must be given in alphabetical order.

// We will not try your program with strings containing duplicates (eg: 'abccd')

// For example this should work:
// $> ./permutations a | cat -e
// a$
// $> ./permutations ab | cat -e
// ab$
// ba$
// $> ./permutations abc | cat -e
// abc$
// acb$
// bac$
// bca$
// cab$
// cba$
/*
 * permutations.c
 *
 * 功能说明（总体）：
 *   接收一个字符串参数（不含重复字符），打印该字符串的所有排列，
 *   并按字典（字母）顺序输出，每个排列一行。
 *
 * 允许使用的函数：puts, malloc, calloc, realloc, free, write
 *
 * 我对你的原实现做了小幅修正：
 * - 为结果缓冲设置 NUL 终止符，从而使用 puts 输出更方便；
 * - 当 len == 0 时返回 1（可按需改成输出空行）；
 * - 保持所有循环使用 while，不使用 for；
 * - 添加逐函数功能说明与逐行注释，便于理解与维护。
 */

#include <stdio.h>  /* puts 函数声明（允许使用） */
#include <stdlib.h> /* malloc, calloc, free */
#include <unistd.h> /* write 函数声明（保留，以防需要） */

/* has_dup:
 * 作用：检查传入字符串是否包含重复字符。
 * 返回值：若存在重复返回 1；否则返回 0。
 * 说明：题目保证不会有重复字符，此函数只是保守检查，可选保留。
 */
int	has_dup(const char *str)
{
	int	i;
	int	j;

	i = 0;                 /* 外层索引从 0 开始 */
	while (str[i] != '\0') /* 遍历每个字符 */
	{
		j = i + 1;             /* 内层从 i+1 开始比较 */
		while (str[j] != '\0') /* 比较 i 与后续所有字符 */
		{
			if (str[i] == str[j]) /* 若发现相等则存在重复 */
				return (1);
			j = j + 1; /* 内层索引递增 */
		}
		i = i + 1; /* 外层索引递增 */
	}
	return (0); /* 没有发现重复 */
}

/* ft_strlen:
 * 作用：计算字符串长度（等价于 strlen）。
 * 返回值：字符串长度（不包含终止符）。
 */
int	ft_strlen(const char *str)
{
	int	len;

	len = 0;                 /* 初始化计数为 0 */
	while (str[len] != '\0') /* 循环直到 NUL 终止符 */
		len = len + 1;       /* 递增计数 */
	return (len);            /* 返回长度 */
}

/* sort_string:
 * 作用：就地对字符串 s 进行升序排序（字典序），使用交换排序（嵌套循环）。
 * 说明：使用 while 循环实现，不使用 for。
 */
void	sort_string(char *s)
{
	int		i;
	int		j;
	char	tmp;

	if (!s) /* 保护性检查：若 s 为 NULL 则直接返回 */
		return ;
	i = 0;
	while (s[i] != '\0') /* 外层 i 遍历每个位置 */
	{
		j = i + 1;           /* j 从 i+1 开始比较 */
		while (s[j] != '\0') /* 内层 j 用来找到比 s[i] 更小的字符并交换 */
		{
			if (s[i] > s[j]) /* 如果 s[i] 大于 s[j] 则交换两者 */
			{
				tmp = s[i];
				s[i] = s[j];
				s[j] = tmp;
			}
			j = j + 1; /* j++ */
		}
		i = i + 1; /* i++ */
	}
}

/* generate:
 * 作用：回溯生成所有排列并输出。
 *
 * 参数说明：
 * - src: 已排序的源字符串（只读）
 * - res: 结果缓冲（长度至少为 len + 1），函数保证 res[len] = '\0'
 * - used: 标志数组，长度为 len，0 表示未用，1 表示已用
 * - len: 字符串长度
 * - depth: 当前构造深度（从 0 到 len）
 *
 * 工作流程：
 * - 若 depth == len，则 res 已构成一个完整排列，使用 puts 输出（自动换行）；
 * - 否则遍历 src 的每个位置 i，若 used[i] == 0，
 *   则把 src[i] 放到 res[depth]，标记 used[i] = 1，递归处理 depth + 1，回溯时将 used[i] = 0。
 */
void	generate(const char *src, char *res, int *used, int len, int depth)
{
	int	i;

	if (depth == len) /* 递归终点：已构造完整排列 */
	{
		/* res 事先保证以 NUL 结尾，因此可以使用 puts 输出并自动换行 */
		puts(res);
		return ; /* 返回上一层以继续生成其他排列 */
	}
	i = 0;
	while (i < len) /* 遍历 src 中的每个字符位置 */
	{
		if (used[i] == 0) /* 若该字符尚未被使用 */
		{
			used[i] = 1;         /* 标记为已用 */
			res[depth] = src[i]; /* 将字符放到当前深度的位置 */
			/* 递归生成下一深度的排列 */
			generate(src, res, used, len, depth + 1);
			/* 回溯：恢复 used 标志为未用，供后续排列使用 */
			used[i] = 0;
		}
		i = i + 1; /* i++，继续尝试下一个字符 */
	}
}

/* main:
 * 作用：主程序入口，负责参数检查、内存分配、排序、调用生成函数、清理资源。
 *
 * 步骤：
 * - 检查参数数量，若不为 2（程序名 + 字符串）则返回 1；
 * - 可选地检查重复字符（此题不必检查，因为题目保证无重复）；
 * - 计算长度 len，若 len == 0 则返回 1（可根据需求修改）；
 * - 分配 src_copy、res、used 等缓冲，并初始化必要元素（如 res[len] = '\0'）；
 * - 对 src_copy 进行排序，调用 generate 生成并输出所有排列；
 * - 释放内存并返回 0。
 */
int	main(int ac, char **av)
{
	int		len;
	char	*src_copy;
	char	*res;
	int		*used;
	int		i;

	/* 参数检查：程序需接收且仅接收一个参数 */
	if (ac != 2)
		return (1);
	/* 可选：检测重复字符；若发现重复则视为错误并返回 1 */
	if (has_dup(av[1]))
		return (1);
	/* 计算字符串长度 */
	len = ft_strlen(av[1]);
	/* 若字符串为空（len == 0），返回 1（按题目可选行为） */
	if (len == 0)
		return (1);
	/* 分配 src_copy，并把 argv[1] 拷贝到 src_copy 中以便就地排序 */
	src_copy = (char *)malloc((len + 1) * sizeof(char));
	if (!src_copy)
		return (1);
	i = 0;
	while (i < len)
	{
		src_copy[i] = av[1][i];
		i = i + 1;
	}
	src_copy[len] = '\0'; /* NUL 终止符 */
	/* 排序源字符串，确保字典序的起点是最小排列 */
	sort_string(src_copy);
	/* 为结果缓冲分配空间，并设置终止符，方便使用 puts 输出 */
	res = (char *)malloc((len + 1) * sizeof(char));
	if (!res)
	{
		free(src_copy);
		return (1);
	}
	res[len] = '\0'; /* 预置 NUL 终止符 */
	/* 分配并初始化 used 标志数组（calloc 会将内存置 0） */
	used = (int *)calloc(len, sizeof(int));
	if (!used)
	{
		free(src_copy);
		free(res);
		return (1);
	}
	/* 调用回溯生成所有排列并输出 */
	generate(src_copy, res, used, len, 0);
	/* 释放资源并返回成功 */
	free(src_copy);
	free(res);
	free(used);
	return (0);
}
