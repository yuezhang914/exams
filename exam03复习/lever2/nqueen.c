/* n_queens_annotated.c
 * 与 subject_n_queens.txt 对齐的逐行中文注释版本
 */

#include <stdio.h>   /* 提供 printf 函数声明 */
#include <stdlib.h>  /* 提供 malloc, free, atoi 等 */

 /* is_safe: 检查在 current_col 列放在 current_row 行是否安全 */
int is_safe(int *positions, int current_col, int current_row)
{
	for (int prev_col = 0; prev_col < current_col; prev_col++) /* 遍历之前已放置的每一列 */
	{
		int prev_row = positions[prev_col]; /* 读取之前在 prev_col 上放的行 */

		/* 若在同一行或在主/副对角线上则冲突 */
		if (prev_row == current_row || /* 同一行 */
			prev_row - prev_col == current_row - current_col || /* 主对角线冲突 */
			prev_row + prev_col == current_row + current_col)   /* 副对角线冲突 */
			return 0; /* 不安全，返回 0 表示冲突 */
	}
	return 1; /* 没有冲突，安全 */
}

/* solve: 回溯函数，在第 col 列放置皇后（0-based 列索引） */
void solve(int *positions, int col, int n)
{
	if (col == n) /* 所有列已放置完毕，输出当前解 */
	{
	   for (int i = 0; i < n; i++) /* 逐列输出对应的行索引 */
		{
			if (i > 0) /* 第一个数字前不输出空格，之后每个数字前输出空格 */
				printf(" ");
			printf("%d", positions[i]); /* 输出 0-based 的行索引（题目要求） */
		}
		printf("\n"); /* 一组解后换行 */
		return;        /* 返回上一层继续搜索其他解 */
	}
	for (int row = 0; row < n; row++) /* 在当前列尝试每一行 */
	{
		if (is_safe(positions, col, row)) /* 如果该位置安全 */
		{
			positions[col] = row;  /* 记录放置位置 */
			solve(positions, col + 1, n); /* 递归放置下一列 */
			/* 回溯时不需要清除 positions[col]，下一次会覆盖 */
		}
	}
}

/* main: 参数解析、内存分配、调用回溯并释放资源 */
int main(int ac, char **av)
{
	if (ac == 2 && av[1][0] != '\0') /* 需要一个非空参数 */
	{
		int n = atoi(av[1]);         /* 把参数解析为整数 n */
		if (n <= 0)                  /* n 必须为正整数 */
			return 1;
		int *positions = malloc(sizeof(int) * n); /* 分配 positions 数组 */
		if (!positions)              /* 检查 malloc 是否成功 */
			return 1;
		solve(positions, 0, n);      /* 从第 0 列开始回溯求解 */
		free(positions);             /* 释放分配的内存 */
	}
	return 0;                        /* 程序正常结束 */
}
