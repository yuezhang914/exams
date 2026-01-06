// Assignment name : powerset
// Expected files : *.c *.h
// Allowed functions : atoi, printf, malloc, calloc, realloc, free.
// ---------------------------------------------------------------

// Write a program that will take as argument an integer n follow by a set of integers,
// your program should display all the subsets of whose sum of elements is n.

// In case of a malloc error your program will exit with the code 1.
// We will not test invalid test(for example '1 1 2')
// hint: the empty set is a subset of anyset.

// For example this should work:
// $> ./powerset 3 1 0 2 4 5 3 | cat -e
// 3$
// 0 3$
// 1 2$
// 1 0 2$
// $> ./powerset 12 5 2 1 8 4 3 7 11 | cat -e
// 8 4$
// 1 11$
// 1 4 7$
// 1 8 3$
// 2 3 7$
// 5 4 3$
// 5 7$
// 5 2 1 4$
// $> ./powerset 7 3 8 2| cat -e
// $
// $> ./powerset 0 1 -1| cat -e
// 1 -1$

// The order of lines is not important,
but the order of the element in a subset is.
// You must not have any duplicates (for example 2 1, 1 2)
// $> ./powerset 5 1 2 3 4 5| cat -e
// valid:
// 1 4$
// 2 3$
// 5$

// or:
// 2 3$
// 5$
// 1 4$
// not valid:
// 4 1$
// 3 2$
/*
 * powerset_printf.c
 *
 * 功能说明：
 *   列出从命令行第二个参数开始到最后一个参数构成集合中，
 *   所有元素和等于第一个参数（目标）的子集。子集中元素在输出中
 *   保持与输入相对顺序。若目标为 0，则需要输出空子集（即一行空行）。
 *
 * 允许使用的函数（本实现使用）：atoi, printf, malloc, calloc, free
 *
 * 所有循环使用 while，不使用 for。
 */

#include <stdio.h> /* printf, NULL */
#include <stdlib.h> /* malloc, calloc, free, atoi */

	/* check_int
		* 作用：检查 argv 数组中从下标 1 到 ac-1 的每个字符串是否为合法整数表示。
		* 说明：合法形式为可选的 '+' 或 '-' 开头，后面至少一位数字。
		* 返回：1 表示全是合法整数表示；0 表示存在非法字符串。
		*/
	int
	check_int(char **str, int ac)
{
	int j; /* 参数索引，从 1 到 ac-1 */
	j = 1;
	while (j < ac) /* 遍历每个命令行参数（包括目标和候选） */
	{
		int i; /* 字符串内部索引 */
		i = 0;
		/* 允许前导的 '+' 或 '-' */
		if (str[j][i] == '-' || str[j][i] == '+')
			i = i + 1;
		/* 如果只有符号，没有数字，则非法 */
		if (str[j][i] == '\0')
			return (0);
		/* 检查剩余字符是否全为数字 */
		while (str[j][i] != '\0')
		{
			if (str[j][i] < '0' || str[j][i] > '9')
				return (0);
			i = i + 1;
		}
		j = j + 1;
	}
	return (1);
}

/* print_subset
 * 作用：把当前子集 subset 中的元素按题目要求格式打印到 stdout。
 * 规则：
 *   - 如果 sub_size == 0（空子集），仅打印一个换行；
 *   - 否则按 "a b c\n" 的格式打印（元素间单个空格，行尾无多余空格）。
 *
 * 参数：
 *   subset: 整数数组，包含当前子集元素
 *   sub_size: 子集长度
 */
void	print_subset(int *subset, int sub_size)
{
	int	idx;

	/* 空子集：输出空行 */
	if (sub_size == 0)
	{
		printf("\n");
		return ;
	}
	idx = 0;
	while (idx < sub_size) /* 逐个输出元素 */
	{
		if (idx > 0)
			printf(" ");           /* 第一个元素前不加空格，后面每个元素前加空格 */
		printf("%d", subset[idx]); /* 打印整数 */
		idx = idx + 1;
	}
	printf("\n"); /* 行尾换行 */
}

/* solve (回溯)
 * 作用：递归枚举 nums[index .. size-1] 的所有子集，
 *      当前已选子集存在 subset[0..sub_size-1] 中，当前和为 curr_sum。
 *
 * 逻辑：
 *  - 若 index == size：已对每个元素做出选择，若 curr_sum == target 则打印子集；
 *  - 否则先递归“不选 nums[index]”分支，再递归“选 nums[index]”分支。
 *
 * 参数：
 *   nums: 候选整数数组（长度 size，若 size==0 可为 NULL）
 *   size: 候选个数
 *   target: 目标和
 *   subset: 当前构造的子集缓冲
 *   index: 当前考虑的 nums 索引
 *   sub_size: 当前子集长度
 *   curr_sum: 当前子集的和
 */
void	solve(int *nums, int size, int target, int *subset, int index,
		int sub_size, int curr_sum)
{
	/* 处理递归终点 */
	if (index == size)
	{
		if (curr_sum == target) /* 满足目标则打印当前子集 */
		{
			print_subset(subset, sub_size);
		}
		return ;
	}
	/* 1) 不选当前元素 nums[index] */
	solve(nums, size, target, subset, index + 1, sub_size, curr_sum);
	/* 2) 选择当前元素 nums[index]：将其加入子集中并递归 */
	subset[sub_size] = nums[index]; /* 把该元素放到子集末尾 */
	solve(nums, size, target, subset, index + 1, sub_size + 1, curr_sum
		+ nums[index]);
	/* 回溯时不需要显式清除 subset[sub_size]，下次会被覆盖 */
}

/* main
 * 作用：程序入口，解析并校验参数，构造 nums 数组与子集缓冲，调用回溯并清理资源。
 *
 * 行为要点：
 *  - 接受形式 ./a.out target val1 val2 ...
 *  - 允许仅提供 target（此时 size == 0）；若 target == 0 会输出空行（空子集）。
 */
int	main(int ac, char **av)
{
	int	i;

	int target;  /* 目标和 */
	int n;       /* 候选数字个数 */
	int *nums;   /* 候选数字数组（若 n==0 可为 NULL） */
	int *subset; /* 子集构造缓冲 */
	/* 至少需要一个参数（目标） */
	if (ac < 2)
		return (1);
	/* 检查所有参数是否为合法整数字符串 */
	if (!check_int(av, ac))
		return (1);
	/* 解析目标和（argv[1]） */
	target = atoi(av[1]);
	/* 候选数字个数为剩余参数数目（可能为 0） */
	n = ac - 2;
	/* 若存在候选项则分配并填充 nums 数组 */
	if (n > 0)
	{
		nums = (int *)malloc(sizeof(int) * n); /* 申请存放 n 个 int 的空间 */
		if (!nums)
			return (1); /* 分配失败则退出 */
		i = 0;
		while (i < n)
		{
			nums[i] = atoi(av[i + 2]); /* 把字符串转换为整数并存入 */
			i = i + 1;
		}
	}
	else
	{
		nums = NULL; /* 无候选元素时置 NULL */
	}
	/* 为子集缓冲分配空间；至少分配 1 个 int 避免 malloc(0) */
	subset = (int *)malloc(sizeof(int) * (n > 0 ? n : 1));
	if (!subset)
	{
		if (nums)
			free(nums);
		return (1);
	}
	/* 调用回溯：从索引 0 开始，当前子集大小 0，当前和 0 */
	solve(nums, n, target, subset, 0, 0, 0);
	/* 清理资源后退出 */
	if (nums)
		free(nums);
	free(subset);
	return (0);
}
