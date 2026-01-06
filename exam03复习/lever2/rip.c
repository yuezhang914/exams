/* rip.c
 * 题目：rip
 * 说明：只使用允许的函数 puts/write（此处使用 puts）。
 *       将最少数量的括号替换为空格，使表达式平衡，并输出所有可能结果。
 */
// Assignment name: rip
// Expected files: *.c *.h
// Allowed functions: puts, write
// --------------------------------------------------------------------------------
// Write a program that will take as argument a string containing only parenthesis.
// If parenthesis are unbalanced (for example "())") your program shall remove the
// minimum number of parenthesis for the expression to be balanced.
// By removing we mean replacing by spaces.
// You will print all the solutions (can be more than one).
// The order of the solutions is not important.
// For example this should work:
// (For readability reasons the '_' means space and the spaces are for readability only.)
// $> ./rip '( ( )' | cat -e
// _ ( ) $
// ( _ ) $
// $> ./rip '( ( ( ) ( ) ( ) ) ( ) )' | cat -e
// ( ( ( ) ( ) ( ) ) ( ) ) $
// $> ./rip '( ) ( ) ) ( )' | cat -e
// ( ) ( ) _ ( ) $
// ( ) ( _ ) ( ) $
// ( _ ( ) ) ( ) $
// $> ./rip '( ( ) ( ( ) (' | cat -e
// ( ( ) _ _ ) _ $
// ( _ ) ( _ ) _ $
// ( _ ) _ ( ) _ $
// _ ( ) ( _ ) _ $
// _ ( ) _ ( ) _ $

#include <stdio.h> /* 提供 puts */

/* 计算字符串中未匹配的括号数量（忽略空格）：
 *  opened -- 当前未匹配的 '(' 个数
 *  closed -- 出现但没有可匹配的 ')' 个数
 * 返回 opened + closed（最少需要删除/替换的括号数）
 */
int	is_val(const char *s)
{
	int	opened;
	int	closed;
	int	i;

	opened = 0;
	closed = 0;
	i = 0;
	/* 遍历字符串直到 NUL 终止 */
	while (s[i] != '\0')
	{
		if (s[i] == '(') /* 遇到左括号，记录为未匹配的左括号 */
			opened = opened + 1;
		else if (s[i] == ')') /* 遇到右括号 */
		{
			if (opened > 0) /* 如果有未匹配左括号，匹配它 */
				opened = opened - 1;
			else /* 否则这是一个多余的右括号 */
				closed = closed + 1;
		}
		/* 空格或其它字符忽略（我们将用空格表示被移除的位置） */
		i = i + 1;
	}
	return (opened + closed); /* 返回需要移除的括号数量之和 */
}

/* 递归枚举删除组合：将字符串中某些括号替换为空格并在删除数量达到 must_fix 时检查合法性并输出。
 * s        : 可修改的字符串缓冲（替换时用 ' '）
 * must_fix : 最少需要删除的括号数量（由 is_val 计算）
 * n_fix    : 当前已删除的括号数量
 * pos      : 下一次尝试删除位置的起点索引（确保组合索引递增，避免重复）
 * len      : 字符串长度
 */
void	rip_rec(char *s, int must_fix, int n_fix, int pos, int len)
{
	int	i;

	/* 如果已删除的数量达标，则检查是否平衡并输出 */
	if (n_fix == must_fix)
	{
		if (is_val(s) == 0) /* 若替换后字符串平衡 */
			puts(s);        /* 输出（puts 会在末尾加换行） */
		return ;
	}
	/* 否则，从 pos 开始尝试每一个位置的括号删除（替换为空格） */
	i = pos;
	while (i < len)
	{
		if (s[i] == '(' || s[i] == ')') /* 只有括号才是候选删除位置 */
		{
			char save = s[i]; /* 保存原字符以便回溯 */
			s[i] = ' ';       /* 把该位置替换为空格（表示删除） */
			rip_rec(s, must_fix, n_fix + 1, i + 1, len);
			/* 递归，下一次只考虑 i+1 之后的位置 */
			s[i] = save; /* 回溯：恢复原字符 */
		}
		i = i + 1;
	}
}

/* 程序主入口：
 * - 检查参数数量（需要恰好一个字符串参数）
 * - 复制参数到本地缓冲（避免修改 argv）
 * - 计算最少删除数 must_fix
 * - 若 must_fix == 0，直接输出原字符串
 * - 否则调用 rip_rec 开始枚举并输出所有解
 */
int	main(int ac, char **av)
{
	int	i;
	int	must_fix;

	/* 题目要求接收单个字符串参数 */
	if (ac != 2)
		return (1);
	/* 使用固定大小缓冲复制输入，避免使用 malloc（题目禁止） */
	{
		char s[1024]; /* 假定输入长度 < 1024，题目范围通常适合 */
		i = 0;
		/* 复制 av[1] 到本地缓冲 s（包括 NUL 终止符） */
		while (av[1][i] != '\0' && i < 1023)
		{
			s[i] = av[1][i];
			i = i + 1;
		}
		s[i] = '\0';
		/* 计算最小需要删除（替换为空格）的括号数量 */
		{
			must_fix = is_val(s);
			int len = i; /* 字符串长度 */
			/* 如果不需要删除（已经平衡），直接输出一次即可 */
			if (must_fix == 0)
			{
				puts(s);
				return (0);
			}
			/* 否则递归枚举所有恰好删除 must_fix 个括号的组合并输出平衡结果 */
			rip_rec(s, must_fix, 0, 0, len);
		}
	}
	return (0);
}
