// filter:
// Allowed functions: read, strlen, malloc, calloc, realloc, free, printf,
perror
// ------------------------------------------------------------------------------

// Write a programm taht will take one and only one argument s.

// Your programm will then read from stdin and write all the content read in stdout
// except that every occurence of s must be replaced by '*'.

// For example :

// ./filter bonjour
// will behave the same way as:
// sed 's/bonjour/*******/g'

// in case of error during read or a malloc you must write "Error: " followed by
// the error message in stderr and return 1.

// $> echo 'abcdefgaaaabcdefabc' | ./filter abc | cat -e
// ***defgaaa***def***

// If the program is called whitout argument or with an empty argument or with
// multiples arguments it must return 1.

/* filter.c — 题目允许函数：
 * read, strlen, malloc, calloc, realloc, free, printf, perror
 *
 * 功能：从 stdin 读取全部内容，把每次出现的 argv[1] 替换为 '*'（同长度的星号），
 * 并把结果写到 stdout。参数必须且仅且非空，否则返回 1。
 *
 * 下面的实现：
 * - 只使用允许的函数；
 * - 所有循环均使用 while（没有 for）；
 * - 每个函数前有简单说明，代码逐行中文注释，便于高中生理解。
 */

#include <errno.h> /* errno, EINTR */
#include <stdio.h> /* printf, perror */
#include <stdlib.h> /* malloc, realloc, free */
#include <string.h> /* strlen */
#include <unistd.h> /* read */

	/* 函数说明（高中生能懂）：
		* safe_append：把新读到的一段数据追加到动态缓冲区里。
		* 如果内存分配失败返回 -1，成功返回 0。
		*/
	static int
	safe_append(char **buf, size_t *total, const char *chunk, ssize_t n)
{
	char *tmp;        /* 临时指针，保存 realloc 返回值 */
	size_t old;       /* 原来缓冲里面的字节数 */
	size_t new_total; /* 追加后的总字节数 */
	/* 记录旧长度 */
	old = *total;
	/* 计算新总长度 */
	new_total = old + (size_t)n;
	/* 重新分配内存，新长度加 1 用来放终止符 '\0' */
	tmp = (char *)realloc(*buf, new_total + 1);
	if (tmp == NULL) /* realloc 失败 */
		return (-1); /* 返回错误标志 */
	*buf = tmp;      /* 更新外部 buf 指针 */
	/* 将新读取的数据逐字复制到 buf 的尾部（不能用 memcpy） */
	{
		size_t k = 0;         /* 复制索引从 0 开始 */
		while (k < (size_t)n) /* 复制 n 字节 */
		{
			(*buf)[old + k] = chunk[k]; /* 逐字赋值 */
			k = k + 1;                  /* k++ */
		}
	}
	/* 添加字符串终止符，方便 printf 使用 */
	(*buf)[new_total] = '\0';
	/* 更新外部记录的总长度 */
	*total = new_total;
	return (0); /* 成功 */
}

// /* 函数说明（高中生能懂）：
//  * replace_inplace：在 buf（长度 len）中查找所有 pat，并就地把每个匹配替换为 '*'。
//  * 替换行为与 sed 's/pat/***/g' 类似；匹配后跳过已替换段（不在替换后的 '*' 上再次匹配）。

static void	replace_inplace(char *buf, size_t len, const char *pat,
		size_t patlen)
{
	size_t i; /* 主扫描索引 */
	/* 若模式为空或缓冲为空或模式比缓冲长，则直接返回 */
	if (patlen == 0 || len == 0 || patlen > len)
		return ;
	i = 0;                    /* 从缓冲起点开始扫描 */
	while (i + patlen <= len) /* 只要剩余长度足够匹配 就继续 */
	{
		size_t j = 0;  /* 模式匹配时的内部索引 */
		int match = 1; /* 先假设匹配成功 */
		/* 逐字符比较 buf[i + j] 与 pat[j] */
		while (j < patlen)
		{
			if (buf[i + j] != pat[j]) /* 一旦有不相等 */
			{
				match = 0; /* 标记为不匹配 */
				break ;     /* 退出比较循环 */
			}
			j = j + 1; /* j++ */
		}
		if (match) /* 如果匹配成功 */
		{
			/* 就地把匹配段替换成 '*' */
			j = 0;
			while (j < patlen)
			{
				buf[i + j] = '*'; /* 把每个字符替换为星号 */
				j = j + 1;
			}
			/* 跳过已替换区域，继续扫描后面的内容 */
			i = i + patlen;
		}
		else
		{
			/* 当前不是匹配，向右移动一个字符继续扫描 */
			i = i + 1;
		}
	}
}

/* 函数说明（高中生能懂）：
 * main：程序入口。检查参数；从 stdin 读取全部内容（用 read），
 * 把内容累积到动态缓冲；执行替换；把结果打印到 stdout。
 * 遇到 read 或 内存分配 错误，调用 perror("Error") 输出错误信息并返回 1。
 */
int	main(int argc, char **argv)
{
	char readbuf[4096]; /* 每次 read 时的临时缓冲 */
	ssize_t n;          /* read 返回的字节数 */
	char *input = NULL; /* 动态缓冲，保存全部输入 */
	size_t total = 0;   /* 已保存的字节数 */
	size_t patlen;      /* 模式长度 */
	/* 参数检查：必须且仅且非空的一个参数 */
	if (argc != 2 || argv[1][0] == '\0')
		/* 题目要求这种情况返回 1；不使用 fprintf（未允许） */
		return (1);
	/* 循环读取 stdin，直到 EOF 或出现错误 */
	while (1)
	{
		/* 从文件描述符 0（stdin）读取最多 sizeof(readbuf) 字节 */
		n = read(0, readbuf, sizeof(readbuf));
		if (n == -1) /* read 出错 */
		{
			if (errno == EINTR) /* 如果是被信号中断，重试读取 */
				continue ;
			/* 其它错误：按题目要求输出 "Error: <errmsg>" 到 stderr */
			perror("Error");
			free(input); /* 释放可能已经分配的内存 */
			return (1);  /* 返回错误码 1 */
		}
		if (n == 0) /* EOF（没有更多数据） */
			break ;  /* 跳出读取循环 */
		/* 把这次读取到的数据追加到动态缓冲 input 中 */
		if (safe_append(&input, &total, readbuf, n) == -1)
		{
			/* 内存分配失败（realloc 返回 NULL）时，perror 输出错误 */
			perror("Error");
			free(input);
			return (1);
		}
	}
	/* 如果没有任何输入（空输入），按题目允许情况下返回 0（成功） */
	if (total == 0)
	{
		free(input); /* free(NULL) 是安全的 */
		return (0);
	}
	/* 计算模式长度并执行替换（若模式非空） */
	patlen = strlen(argv[1]);
	if (patlen > 0)
		replace_inplace(input, total, argv[1], patlen);
	/* 使用 printf 输出最终结果（题目允许 printf） */
	printf("%s", input);
	/* 清理内存并正常退出 */
	free(input);
	return (0);
}
