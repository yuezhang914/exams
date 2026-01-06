// 对应文件：subject_ft_scanf.txt）
// Assignment name  : ft_scanf
// Expected files   : ft_scanf.c
// Allowed functions: fgetc, ungetc, ferror, feof, isspace, isdigit, stdin,
	va_start, va_arg, va_copy, va_end
// --------------------------------------------------------------------------------

// Write a function named `ft_scanf` that will mimic the real scanf with the following constraints:

// - It will manage only the following conversions: s, d, and c
// - You don't have to handle the options *, m and '
// - You don't have to handle the maximum field width
// - You don't have to handle the types modifier characters (h, hh, l, etc.)
// - You don't have to handle the conversions beginning with %n$

// Your function must be declared as follows:

// int ft_scanf(const char *, ... );

// You will find in this directory a file containing a part of the code you will need,
	// you just have to complete it.

// To test your program compare your results with the real scanf.

// Hint : You may need to read the man of scanf.

/* ft_scanf_official.c
 * 对应：subject_ft_scanf.txt（官方题目，允许使用 fgetc/ungetc/isspace/isdigit 等）
 * 功能：实现 ft_scanf 的子集，支持 %s, %d, %c
 */

/* 引入需要的头文件：
 *  <stdarg.h>  - 可变参数宏 va_start/va_arg/va_end
 *  <stdio.h>   - FILE, fgetc, ungetc, stdin, ferror
 *  <ctype.h>   - isspace, isdigit
 */
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

/* match_space: 跳过输入流中的空白字符（空格/换行/制表等）。
 *  如果读到 EOF 返回 -1（表示无法匹配/输入结束）。
 *  如果读到非空白字符则将其放回流（ungetc）并返回 1。
 */
int	match_space(FILE *f)
{
	int c; /* 当前读到的字符（作为 int 表示以容纳 EOF） */
	c = fgetc(f);                  /* 读取一个字符 */
	while (c != EOF && isspace(c)) /* 跳过连续空白 */
		c = fgetc(f);
	if (c == EOF)  /* 如果到达 EOF，没有非空白字符 */
		return (-1); /* 返回 -1 表示失败 / EOF */
	ungetc(c, f);  /* 把第一个非空白字符放回流，供后续读取 */
	return (1);      /* 返回 1 表示成功匹配并跳过空白 */
}

/* match_char: 从输入流读一个字符并与 c 对比。
 * 返回 1 表示匹配、0 表示不匹配（并把读到的字符放回）、-1 表示 EOF。
 */
int	match_char(FILE *f, char c)
{
	int input; /* 存放 fgetc 的返回值 */
	input = fgetc(f); /* 读取一个字符 */
	if (input == EOF) /* EOF 情况 */
		return (-1);
	if (input == (unsigned char)c) /* 字面匹配（注意转换） */
		return (1);                  /* 匹配成功 */
	ungetc(input, f);              /* 不匹配则回退 */
	return (0);                      /* 返回 0 表示未匹配但非严重错误 */
}

/* scan_char: 处理 %c —— 读取一个字符（不跳过空白），写入 va_list 提供的 char*。
 * 返回 1 成功；-1 表示 EOF（失败）。
 */
int	scan_char(FILE *f, va_list ap)
{
	int c;     /* 保存读取的字符 */
	char *ptr; /* 指向调用者提供的目标缓冲 */
	c = fgetc(f); /* 读取一个字符（%c 不跳空白） */
	if (c == EOF)
		return (-1);
	ptr = va_arg(ap, char *); /* 取得下一个可变参数，应为 char * */
	*ptr = (char)c;           /* 存储字符 */
	return (1);                 /* 成功 */
}

/* scan_int: 处理 %d —— 假设前导空白已通过 match_space 跳过（由 match_conv 控制）。
 * 读取可选 + 或 -，然后一连串数字；遇到第一个非数字字符就 ungetc 回退。
 * 返回 1 表示成功赋值，返回 0 表示匹配失败（比如没有数字），返回 -1 表示 EOF（若实现中想区分）。
 */
int	scan_int(FILE *f, va_list ap)
{
	int sign;          /* 符号标志 */
	int digit_scanned; /* 是否至少读到一位数字 */
	int c;             /* 当前字符 */
	int r;             /* 累积的结果 */
	int *ptr;          /* 存放要写入的 int* */

	sign = 1;
	digit_scanned = 0;
	r = 0;

	c = fgetc(f); /* 读取第一个字符（前导空白假定已跳过） */
	if (c == EOF)
		return (-1);

	if (c == '+' || c == '-') /* 处理可选符号 */
	{
		if (c == '-')
			sign = -1;
		c = fgetc(f); /* 读符号后的下一个字符 */
	}

	while (c != EOF && isdigit(c))
	{
		r = r * 10 + (c - '0'); /* 累加数字 */
		digit_scanned++;
		c = fgetc(f); /* 继续读 */

		if (c != EOF) /* 将第一个非数字字符放回流 */
			ungetc(c, f);

		if (digit_scanned == 0) /* 若没有读到任何数字，说明匹配失败 */
			return (0);           /* 返回 0 表示未匹配（非 EOF 错误） */

		ptr = va_arg(ap, int *); /* 取得 int* 并写入结果 */
		*ptr = r * sign;
		return (1); /* 成功 */
	}

	/* scan_string: 处理 %s —— 前导空白应由 match_space 跳过（由 match_conv 控制）。
		* 读取直到遇到空白或 EOF，把字符写入调用者提供的缓冲区（调用者需确保缓冲区足够大）。
		* 返回 1 表示成功赋值，返回 0 表示未匹配（无字符），返回 -1 表示 EOF。
		*/
	int scan_string(FILE * f, va_list ap)
	{
		char *str;         /* 目标缓冲 */
		int c;             /* 当前字符 */
		int chars_scanned; /* 计数器 */

		str = va_arg(ap, char *); /* 取得目标缓冲指针 */
		c = fgetc(f);             /* 读取首字符（假设前导空白已跳过） */
		if (c == EOF)
			return (-1);

		chars_scanned = 0;
		while (c != EOF && !isspace(c))
		{
			str[chars_scanned] = (char)c; /* 存字符 */
			chars_scanned++;
			c = fgetc(f); /* 读下一个字符 */
		}

		if (c != EOF) /* 如果因空白停止，将该空白放回流 */
			ungetc(c, f);

		str[chars_scanned] = '\0'; /* 以 NUL 结尾 */
		if (chars_scanned == 0)    /* 如果没有读到任何字符，匹配失败 */
			return (0);
		return (1); /* 成功 */
	}

	/* match_conv: 根据 format 指针所指字符决定调用哪个 scan_* 函数。
		* 为了符合 scanf 行为，%d 与 %s 在调用对应 scan 函数前要先调用 match_space 跳过输入空白。
		*/
	int match_conv(FILE * f, const char **format, va_list ap)
	{
		switch (**format)
		{
		case 'c':
			return (scan_char(f, ap)); /* %c 不跳白，由 scan_char 直接读 */
		case 'd':
			if (match_space(f) == -1) /* 跳过前导空白，若 EOF 则传播错误 */
				return (-1);
			return (scan_int(f, ap)); /* 读取整数 */
		case 's':
			if (match_space(f) == -1) /* 跳过前导空白 */
				return -1;
			return scan_string(f, ap); /* 读取字符串 */
		case EOF:
			return -1;
		default:
			return -1;
		}
	}

	/* ft_vfscanf: 按 format 解析并从 FILE* f 中读取。返回成功赋值的字段数，或 EOF（-1）在输入前即失败。
		* 行为说明：
		*  - 遇 '%' 则调用 match_conv
		*  - 遇 format 中的空白则调用 match_space 去跳过输入空白
		*  - 遇其他字母/符号则调用 match_char 要求逐字匹配
		*/
	int ft_vfscanf(FILE * f, const char *format, va_list ap)
	{
		int nconv = 0; /* 成功赋值字段计数 */
		int c;         /* 临时变量用于初始 EOF 检测 */
		int ret;       /* 用于接收各个子函数的返回值 */

		/* 初始快速检测：如果输入一开始就是 EOF，则直接返回 EOF（题目框架原本就有） */
		c = fgetc(f);
		if (c == EOF)
			return EOF;
		ungetc(c, f);

		while (*format)
		{
			if (*format == '%')
			{
				format++;
				ret = match_conv(f, &format, ap);
				if (ret == -1) /* -1 表示在没有成功赋值前遇到 EOF/严重错误 */
					return (nconv == 0) ? EOF : nconv;
				if (ret == 0) /* 0 表示该转换未匹配到任何东西（比如 %d 没读到数字） */
					return nconv;
				nconv++; /* 成功赋值一个字段 */
			}
			else if (isspace((unsigned char)*format))
			{
				/* format 中的空白意味着要跳过输入中的空白 */
				if (match_space(f) == -1)
					return (nconv == 0) ? EOF : nconv;
			}
			else
			{
				/* 非空白的字面字符需要和输入逐个匹配 */
				ret = match_char(f, *format);
				if (ret == -1)
					return (nconv == 0) ? EOF : nconv;
				if (ret == 0)
					return nconv;
			}
			format++;
		}

		if (ferror(f))
			return EOF;
		return nconv; /* 返回成功赋值的字段数 */
	}

	/* ft_scanf: 外壳，初始化 va_list 并调用 ft_vfscanf(stdin, format, ap) */
	int ft_scanf(const char *format, ...)
	{
		va_list ap;
		int ret;

		va_start(ap, format);
		ret = ft_vfscanf(stdin, format, ap);
		va_end(ap);
		return ret;
	}
