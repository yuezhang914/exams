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

#include <errno.h> /* errno, EINTR：read 被信号打断时会用到 */
#include <stdio.h> /* printf, perror */
#include <stdlib.h> /* malloc, realloc, free */
#include <string.h> /* strlen */
#include <unistd.h> /* read */

	/*
	** safe_append（超级重点，指针最容易卡的地方）
	**
	** 目标：把 chunk[0..n-1] 这段新读到的数据，追加到动态字符串 *buf 的末尾。
	**
	** 为什么参数是 char **buf ？
	** - main 里有 char *input;
	** - 追加过程中会用 realloc：realloc 可能返回“新地址”，旧地址可能失效
	** - 你必须把“新地址”写回 main 的 input
	** - 所以必须传入 input 的地址：&input
	** - &input 的类型就是 char **，因此这里写 char **buf
	**
	** 参数解释：
	** - buf   ：指向 main 里的 input（也就是 “input 变量的地址”）
	**           buf 里面存的是 &input
	**           *buf 才是 input（字符串首地址）
	** - total ：指向 main 里的 total（也就是 “total 变量的地址”）
	**           *total 才是当前长度
	** - chunk ：这次 read 读到的临时数据（只读，不改它）
	** - n     ：chunk 有效字节数
	**
	** 返回值：
	** - 成功：0
	** - 失败：-1（通常是 realloc 失败）
	*/
	static int
	safe_append(char **buf, size_t *total, const char *chunk, ssize_t n)
{
	size_t	k;

	char *tmp;        /* 用来接 realloc 的返回值（先别直接写回 *buf，安全） */
	size_t old;       /* 追加前已有多少字节 */
	size_t new_total; /* 追加后总字节数 */
	/* old = 当前总长度
		total 是 size_t*，要取它的值用 *total */
	old = *total;
	/* n 是 ssize_t（可能是负数，但这里只有 n>0 时才会调用）
		这里把 n 转成 size_t（无符号）用于长度计算 */
	new_total = old + (size_t)n;
	/*
	** realloc 需要的是“旧内存块地址”
	** buf 是 char**，*buf 才是 char*（也就是 input）
	** 所以这里必须用 *buf
	**
	** 还要 +1：多留一个字节放 '\0'
	** 因为最后要用 printf("%s", input)，printf 需要以 '\0' 结尾的字符串
	*/
	tmp = (char *)realloc(*buf, new_total + 1);
	if (tmp == NULL)
		return (-1);
	/*
	** 关键：把新地址写回 main 的 input
	** buf 指向 main 的 input，所以 *buf 就是 main 的 input
	*/
	*buf = tmp;
	/*
	** 把 chunk 逐字节拷贝到追加位置：
	** 追加位置从 old 开始（old 是原长度）
	**
	** (*buf)[old + k]：
	** - *buf 是字符串首地址（char*）
	** - (*buf)[i] 就是第 i 个字符
	*/
	{
		k = 0;
		while (k < (size_t)n)
		{
			(*buf)[old + k] = chunk[k];
			k = k + 1;
		}
	}
	/* 追加完后补 '\0'，让它成为合法 C 字符串 */
	(*buf)[new_total] = '\0';
	/* 更新 main 里的 total，所以要写 *total */
	*total = new_total;
	return (0);
}

/*
** replace_inplace
**
** 目标：在 buf[0..len-1] 这段内容里，找到所有 pat，并把匹配到的那段字符全部改成 '*'
**（长度不变，替换成同长度星号）
**
** 为什么可以 “inplace（原地修改）”？
** - 因为替换前后长度相同：比如 "abc" -> "***"
** - 不需要挪动后面内容，所以直接改字符就行
**
** 参数解释：
** - buf    ：要修改的整段文本（可写）
** - len    ：buf 的有效长度（不是 strlen，而是我们累计的 total）
** - pat    ：要匹配的字符串（argv[1]）
** - patlen ：pat 的长度（strlen(argv[1])）
*/
static void	replace_inplace(char *buf, size_t len, const char *pat,
		size_t patlen)
{
	size_t	i;
	size_t	j;
	int		match;

	/* 如果 pat 为空、buf 为空、pat 比 buf 还长：直接不做 */
	if (patlen == 0 || len == 0 || patlen > len)
		return ;
	i = 0;
	/*
	** i + patlen <= len：保证从 i 开始往后至少还有 patlen 个字符能比较
	** 例如 len=10, patlen=3，那么 i 最大到 7（7,8,9 还能比 3 个）
	*/
	while (i + patlen <= len)
	{
		j = 0;
		match = 1;
		/* 尝试匹配 pat[0..patlen-1] */
		while (j < patlen)
		{
			if (buf[i + j] != pat[j])
			{
				match = 0;
				break ;
			}
			j = j + 1;
		}
		if (match)
		{
			/* 匹配成功：把 buf[i..i+patlen-1] 全部变成 '*' */
			j = 0;
			while (j < patlen)
			{
				buf[i + j] = '*';
				j = j + 1;
			}
			/*
			** 很重要：i 直接跳过 patlen
			** 因为这一段已经替换了，不需要从 i+1 重新匹配
			** 这也符合 sed 的常见“非重叠匹配”效果
			*/
			i = i + patlen;
		}
		else
		{
			/* 没匹配上：向右走一步继续试 */
			i = i + 1;
		}
	}
}

/*
** main：程序入口
**
** 目标：
** 1) 检查参数：必须恰好一个非空参数 s（即 argv[1]）
** 2) 用 read(0, ...) 从 stdin 读入所有内容，拼成一整块 input
** 3) 把 input 里每次出现的 s 替换成同长度 '*'
** 4) 输出到 stdout
** 5) 读失败 / 内存失败：perror("Error")，返回 1
*/
int	main(int argc, char **argv)
{
	char readbuf[4096]; /* 每次 read 临时装数据的桶 */
	ssize_t n;          /* read 返回读到的字节数：-1 错误，0 EOF，>0 正常 */
	char *input;        /* 动态拼出来的“全部输入内容” */
	size_t total;       /* input 当前一共装了多少字节 */
	size_t patlen;      /* argv[1] 的长度 */
	
	input = NULL;
	total = 0;
	/*
	** 参数检查：
	** argc 必须等于 2（程序名 + 一个参数）
	** argv[1][0] == '\0' 表示空字符串参数
	*/
	if (argc != 2 || argv[1][0] == '\0')
		return (1);
	/* 反复读取 stdin，直到 EOF */
	while (1)
	{
		/*
		** read(0, ...)：
		** 0 是 stdin 的文件描述符
		** readbuf 是数组名，会自动变成“首元素地址”，所以直接传 readbuf
		** sizeof(readbuf) 是最多读多少字节
		*/
		n = read(0, readbuf, sizeof(readbuf));
		if (n == -1)
		{
			/* EINTR：被信号打断，不算真失败，继续读 */
			if (errno == EINTR)
				continue ;
			/* 真错误：题目要求 "Error: <message>" 到 stderr */
			perror("Error");
			free(input);
			return (1);
		}
		if (n == 0)
			break ; /* EOF：读完了 */
		/*
		** 重点：这里为什么传 &input 和 &total？
		**
			- safe_append 里面会 realloc，可能改变 input 的地址，所以要让它能改 main 的 input → 传 &input
		** - safe_append 里面会更新 total → 传 &total
		*/
		if (safe_append(&input, &total, readbuf, n) == -1)
		{
			perror("Error");
			free(input);
			return (1);
		}
	}
	/* 没输入内容：直接成功退出（不输出也行） */
	if (total == 0)
	{
		free(input);
		return (0);
	}
	/* 计算匹配串长度并替换 */
	patlen = strlen(argv[1]);
	replace_inplace(input, total, argv[1], patlen);
	/* 输出最终结果 */
	printf("%s", input);
	free(input);
	return (0);
}

// #include <errno.h> /* errno, EINTR */
// #include <stdio.h> /* printf, perror */
// #include <stdlib.h> /* malloc, realloc, free */
// #include <string.h> /* strlen */
// #include <unistd.h> /* read */

// 	/* 函数说明（高中生能懂）：
// 		* safe_append：把新读到的一段数据追加到动态缓冲区里。
// 		* 如果内存分配失败返回 -1，成功返回 0。
// 		*/
// 	static int
// 	safe_append(char **buf, size_t *total, const char *chunk, ssize_t n)
// {
// 	char *tmp;        /* 临时指针，保存 realloc 返回值 */
// 	size_t old;       /* 原来缓冲里面的字节数 */
// 	size_t new_total; /* 追加后的总字节数 */
// 	/* 记录旧长度 */
// 	old = *total;
// 	/* 计算新总长度 */
// 	new_total = old + (size_t)n;
// 	/* 重新分配内存，新长度加 1 用来放终止符 '\0' */
// 	tmp = (char *)realloc(*buf, new_total + 1);
// 	if (tmp == NULL) /* realloc 失败 */
// 		return (-1); /* 返回错误标志 */
// 	*buf = tmp;      /* 更新外部 buf 指针 */
// 	/* 将新读取的数据逐字复制到 buf 的尾部（不能用 memcpy） */
// 	{
// 		size_t k = 0;         /* 复制索引从 0 开始 */
// 		while (k < (size_t)n) /* 复制 n 字节 */
// 		{
// 			(*buf)[old + k] = chunk[k]; /* 逐字赋值 */
// 			k = k + 1;                  /* k++ */
// 		}
// 	}
// 	/* 添加字符串终止符，方便 printf 使用 */
// 	(*buf)[new_total] = '\0';
// 	/* 更新外部记录的总长度 */
// 	*total = new_total;
// 	return (0); /* 成功 */
// }

// // /* 函数说明（高中生能懂）：
// //  * replace_inplace：在 buf（长度 len）中查找所有 pat，并就地把每个匹配替换为 '*'。
// //  * 替换行为与 sed 's/pat/***/g' 类似；匹配后跳过已替换段（不在替换后的 '*' 上再次匹配）。

// static void	replace_inplace(char *buf, size_t len, const char *pat,
// 		size_t patlen)
// {
// 	size_t i; /* 主扫描索引 */
// 	/* 若模式为空或缓冲为空或模式比缓冲长，则直接返回 */
// 	if (patlen == 0 || len == 0 || patlen > len)
// 		return ;
// 	i = 0;                    /* 从缓冲起点开始扫描 */
// 	while (i + patlen <= len) /* 只要剩余长度足够匹配 就继续 */
// 	{
// 		size_t j = 0;  /* 模式匹配时的内部索引 */
// 		int match = 1; /* 先假设匹配成功 */
// 		/* 逐字符比较 buf[i + j] 与 pat[j] */
// 		while (j < patlen)
// 		{
// 			if (buf[i + j] != pat[j]) /* 一旦有不相等 */
// 			{
// 				match = 0; /* 标记为不匹配 */
// 				break ;     /* 退出比较循环 */
// 			}
// 			j = j + 1; /* j++ */
// 		}
// 		if (match) /* 如果匹配成功 */
// 		{
// 			/* 就地把匹配段替换成 '*' */
// 			j = 0;
// 			while (j < patlen)
// 			{
// 				buf[i + j] = '*'; /* 把每个字符替换为星号 */
// 				j = j + 1;
// 			}
// 			/* 跳过已替换区域，继续扫描后面的内容 */
// 			i = i + patlen;
// 		}
// 		else
// 		{
// 			/* 当前不是匹配，向右移动一个字符继续扫描 */
// 			i = i + 1;
// 		}
// 	}
// }

// /* 函数说明（高中生能懂）：
//  * main：程序入口。检查参数；从 stdin 读取全部内容（用 read），
//  * 把内容累积到动态缓冲；执行替换；把结果打印到 stdout。
//  * 遇到 read 或 内存分配 错误，调用 perror("Error") 输出错误信息并返回 1。
//  */
// int	main(int argc, char **argv)
// {
// 	char readbuf[4096]; /* 每次 read 时的临时缓冲 */
// 	ssize_t n;          /* read 返回的字节数 */
// 	char *input = NULL; /* 动态缓冲，保存全部输入 */
// 	size_t total = 0;   /* 已保存的字节数 */
// 	size_t patlen;      /* 模式长度 */
// 	/* 参数检查：必须且仅且非空的一个参数 */
// 	if (argc != 2 || argv[1][0] == '\0')
// 		/* 题目要求这种情况返回 1；不使用 fprintf（未允许） */
// 		return (1);
// 	/* 循环读取 stdin，直到 EOF 或出现错误 */
// 	while (1)
// 	{
// 		/* 从文件描述符 0（stdin）读取最多 sizeof(readbuf) 字节 */
// 		n = read(0, readbuf, sizeof(readbuf));
// 		if (n == -1) /* read 出错 */
// 		{
// 			if (errno == EINTR) /* 如果是被信号中断，重试读取 */
// 				continue ;
// 			/* 其它错误：按题目要求输出 "Error: <errmsg>" 到 stderr */
// 			perror("Error");
// 			free(input); /* 释放可能已经分配的内存 */
// 			return (1);  /* 返回错误码 1 */
// 		}
// 		if (n == 0) /* EOF（没有更多数据） */
// 			break ;  /* 跳出读取循环 */
// 		/* 把这次读取到的数据追加到动态缓冲 input 中 */
// 		if (safe_append(&input, &total, readbuf, n) == -1)
// 		{
// 			/* 内存分配失败（realloc 返回 NULL）时，perror 输出错误 */
// 			perror("Error");
// 			free(input);
// 			return (1);
// 		}
// 	}
// 	/* 如果没有任何输入（空输入），按题目允许情况下返回 0（成功） */
// 	if (total == 0)
// 	{
// 		free(input); /* free(NULL) 是安全的 */
// 		return (0);
// 	}
// 	/* 计算模式长度并执行替换（若模式非空） */
// 	patlen = strlen(argv[1]);
// 	if (patlen > 0)
// 		replace_inplace(input, total, argv[1], patlen);
// 	/* 使用 printf 输出最终结果（题目允许 printf） */
// 	printf("%s", input);
// 	/* 清理内存并正常退出 */
// 	free(input);
// 	return (0);
// }