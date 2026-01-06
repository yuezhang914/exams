/* 完整的 tsp 实现（适用于题目给出的 tsp_give.c 框架）
 * 说明：
 * - 实现函数：float tsp(float (*array)[2], ssize_t size)
 * - 算法：预计算距离矩阵 + 回溯（DFS）+ 剪枝
 * - 使用到的库函数均在题目允许列表中（sqrtf, malloc, calloc, free 等）
 *
 * 编译时链接数学库（若使用 gcc）请加 -lm，例如：
 *   gcc -Wall -Wextra -Werror tsp_give.c -o tsp -lm
 */

#include <math.h>      /* sqrtf */
#include <stddef.h>    /* size_t*/
#include <stdlib.h>    /* malloc, calloc, free */
#include <sys/types.h> /* for ssize_t */

/* distance 函数在题目给定文件里已经存在，但若单独使用此实现需要它： */
/* float distance(float a[2], float b[2]) { ... }  已由题目文件提供 */

/* 递归辅助函数：深度优先搜索并做剪枝。
 * 参数说明：
 *  - current: 当前城市索引
 *  - visited_count: 已访问城市数量
 *  - current_dist: 当前累计路径长度
 *  - n: 城市总数
 *  - d: 指向 n * n 的距离矩阵（d[i*n + j] 表示 i->j 的距离）
 *  - visited: 指向 n 字节的已访问标志数组（0/1）
 *  - best: 指向当前最短回路长度（递归过程中可能更新）
 */
static void	dfs_tsp(int current, int visited_count, float current_dist, int n,
		float *d, unsigned char *visited, float *best)
{
	int		j;
	float	total;

	/* 如果已经访问了所有城市，则闭合回路回到起点(0)并更新最优值 */
	if (visited_count == n)
	{
		/* 闭合回路：从 current 回到 0 */
		total = current_dist + d[current * n + 0];
		if (total < *best)
			*best = total; /* 更新最优值 */
		return ;
	}
	/* 遍历所有城市，尝试访问尚未访问的城市 */
	j = 0;
	while (j < n)
	{
		if (!visited[j])
		{
			float nd = current_dist + d[current * n + j]; /* 预计的新距离 */
			/* 剪枝：如果当前长度加上到 j 的距离已经不优，则跳过 */
			if (nd < *best)
			{
				visited[j] = 1; /* 标记为已访问 */
				dfs_tsp(j, visited_count + 1, nd, n, d, visited, best);
				visited[j] = 0; /* 回溯：恢复标记 */
			}
		}
		j++;
	}
}

/* 主函数：计算最短闭合路径长度并返回（符合题目 tsp() 接口） */
float	tsp(float (*array)[2], ssize_t size)
{
	int		n;
	float	best;
	int		i;
	int		j;

	float *d = NULL;               /* 距离矩阵，平铺为一维数组，长度 n*n */
	unsigned char *visited = NULL; /* 已访问标志数组（用 unsigned char 节省内存） */
	/* 边界情况处理：如果城市数量小于等于 1，路径长度为 0 */
	if (size <= 1)
		return (0.0f);
	/* 将 ssize_t 转为 int，题目保证 size 不会超过 11，所以安全 */
	n = (int)size;
	/* 分配距离矩阵（n * n floats） */
	d = (float *)malloc((size_t)(n * n) * sizeof(float));
	if (!d)
		return (0.0f); /* 内存分配失败则返回 0（也可按需返回大值或 errno） */
	/* 预计算每对城市之间的距离，减少重复计算开销 */
	{
		i = 0;
		while (i < n)
		{
			j = 0;
			while (j < n)
			{
				/* 使用题目提供的 distance() 计算欧氏距离 */
				d[i * n + j] = distance(array[i], array[j]);
				j++;
			}
			i++;
		}
	}
	/* 分配 visited 数组并初始化为 0 （calloc 将内存置零） */
	visited = (unsigned char *)calloc((size_t)n, sizeof(unsigned char));
	if (!visited)
	{
		free(d);
		return (0.0f);
	}
	/* 初始化最优值为一个很大的浮点数 */
	best = 1e38f;
	/* 从城市 0 出发，标记已访问并开始回溯搜索 */
	visited[0] = 1;
	dfs_tsp(0, 1, 0.0f, n, d, visited, &best);
	/* 释放分配的内存 */
	free(d);
	free(visited);
	/* 返回找到的最短闭合路径长度 */
	return (best);
}
