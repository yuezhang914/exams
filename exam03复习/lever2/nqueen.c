/* n_queens_annotated.c
 * 与 subject_n_queens.txt 对齐的逐行中文注释版本
 */

// Assignement name : n_queens

// Expected files : *.c *.h

// Allowed functions : atoi, fprintf, write

// -------------------------------------------------------------------------

// Write a program that will print all the solutions to the n queens problem
// for a n given as argument.
// We will not test with negative values.
// The order of the solutions is not important.

// You will display the solutions under the following format :
// <p1> <p2> <p3> ... \n
// where pn are the line index of the queen in each colum starting from 0.

// For example this should work :
// $> ./n_queens 2 | cat -e

// $> ./n_queens 4 | cat -e
// 1 3 0 2$
// 2 0 3 1$

// $> ./n_queens 7 | cat -e
// 0 2 4 6 1 3 5$
// 0 3 6 2 5 1 4$

/*
 * n_queens.c
 *
 * 说明：
 * - 题目允许的函数：atoi, fprintf, write
 * - 本实现未使用 malloc/printf/for，全部循环使用 while，
 *   输出使用 fprintf(stdout, ...)（允许）。
 * - 使用可变长数组 (VLA) int positions[n] 存放每列的皇后行号，
 *   这样无需动态分配堆内存。
 *
 * 直接编译示例：
 * gcc -std=c99 -Wall -Wextra -o n_queens n_queens.c
 */

/* 包含需要的头文件：
 * - stdio.h 提供 fprintf (用于输出到 stdout/stderr)
 * - stdlib.h 提供 atoi (用于解析命令行参数)
 */
#include <stdio.h>
#include <stdlib.h>

/* is_safe:
 * 作用（高中生能懂）：
 *   判断在列 current_col 放置皇后到行 current_row 是否与之前已放的皇后冲突。
 *   如果与之前任一皇后同一行、同一主对角或同一副对角，返回 0（不安全）。
 *   否则返回 1（安全）。
 *
 * 参数：
 *   positions: 整数数组，positions[c] 表示第 c 列上皇后放在的行号
 *   current_col: 当前要放皇后的列索引（0-based）
 *   current_row: 尝试放皇后的行索引（0-based）
 */
int	is_safe(int *positions, int current_col, int current_row)
{
	int	prev_col;
	int	prev_row;

	/* prev_col 用来遍历之前已放置皇后的列，从 0 到 current_col - 1 */
	prev_col = 0;
	/* 只需检查之前放置的列 */
	while (prev_col < current_col)
	{
		/* 读取 prev_col 列上皇后所在的行 */
		prev_row = positions[prev_col];
		/* 检查是否同一行：
			* 如果 prev_row == current_row，说明两个皇后在同一行，会互相攻击 */
		if (prev_row == current_row)
			return (0); /* 冲突，返回不安全 */
		/* 检查主对角线冲突（行-列相等）：prev_row - prev_col == current_row - current_col */
		if (prev_row - prev_col == current_row - current_col)
			return (0); /* 冲突，返回不安全 */
		/* 检查副对角线冲突（行+列相等）：prev_row + prev_col == current_row + current_col */
		if (prev_row + prev_col == current_row + current_col)
			return (0); /* 冲突，返回不安全 */
		/* 检查下一个已放置的皇后 */
		prev_col = prev_col + 1;
	}
	/* 没有发现冲突，返回安全 */
	return (1);
}

/* solve:
 * 作用（高中生能懂）：
 *   使用回溯法在第 col 列放置皇后并递归处理后续列。
 *   当 col == n 时说明所有列都放好了，打印一组解。
 *
 * 参数：
 *   positions: 存放每列皇后行索引的数组（长度 n）
 *   col: 当前正在处理的列索引
 *   n: 棋盘大小（列数/行数）
 */
void	solve(int *positions, int col, int n)
{
	int	row;

	/* 如果 col == n，说明所有列都放置完成，打印当前解 */
	if (col == n)
	{
		int i = 0; /* 用来遍历 positions 数组 */
		/* 逐个输出 positions[i]，数字之间用空格分隔 */
		while (i < n)
		{
			if (i > 0)
			{
				/* 在每个数字前输出一个空格（第一个数字前不输出） */
				/* 使用 fprintf 输出到 stdout（allowed） */
				fprintf(stdout, " ");
			}
			/* 输出第 i 列皇后所在的行索引（0-based） */
			fprintf(stdout, "%d", positions[i]);
			i = i + 1;
		}
		/* 当前解输出完毕，换行 */
		fprintf(stdout, "\n");
		return ; /* 返回上一层继续寻找其它解 */
	}
	/* 在当前列 col 逐行尝试放置皇后（从行 0 到 n-1） */
	row = 0;
	while (row < n)
	{
		/* 先判断在 (row, col) 放置是否安全 */
		if (is_safe(positions, col, row))
		{
			/* 记录当前列的放置行 */
			positions[col] = row;
			/* 递归尝试放置下一列 */
			solve(positions, col + 1, n);
			/* 回溯：不需要显式清除 positions[col]，下次会被覆盖 */
		}
		/* 尝试下一行 */
		row = row + 1;
	}
}

/* main:
 * 作用（高中生能懂）：
 *   解析命令行参数 n（使用 atoi），若参数合法则初始化 positions 数组并
 *   调用 solve(…, 0, n) 来开始回溯求解，最后返回 0。
 *   参数不合法时返回 1。
 */
int	main(int ac, char **av)
{
	int	n;
	int	positions[n];

	/* 参数检查：必须且只有一个参数，并且参数非空 */
	if (ac != 2 || av[1][0] == '\0')
	{
		/* 参数格式不对，返回错误码 1 */
		return (1);
	}
	/* 把参数字符串转换为整数 n */
	n = atoi(av[1]);
	/* 如果 n <= 0，则视为非法（题目说明不会给负值，但我们做防护） */
	if (n <= 0)
		return (1);
	/* 使用 VLA（变量长度数组）在栈上分配 positions 数组，长度为 n
		* 这样避免使用 malloc（题目允许的函数列表中没有 malloc）
		*/
	/* 从第 0 列开始回溯求解 */
	solve(positions, 0, n);
	/* 正常结束，返回 0 */
	return (0);
}
