// Assignment name			: filter
// Expected files			: *.c *.h ???
// Allowed functions   : malloc, free, calloc, realloc, write, read, perror
// -------------------------------------------------------

// Write a program that reads from the standard input and replaces every occurrence
// of a given word (provided as an argument) with a sequence of * of the same length.

// If no argument is given or if the input is empty, the program should return an error.

// We will not test with an empty string as an argument.

// You will display the modified text, preserving the rest of the input.

// Example:
// $> echo "The secret word is hidden in this text." | ./filter secret  
// The ****** word is hidden in this text.

// $> echo "banana banana apple" | ./filter banana  
// ****** ****** apple

// Memory management is important: any allocated memory must be freed properly.


/* ft_scanf_simplified.c
 * 对应：not_official_subject.txt（简化版题目）
 * 功能：仅支持 %c 与 %d（题目说明说只测试正整数），实现稳健的空白处理
 */

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

/* 跳过输入空白并把第一个非空白放回，返回 1 成功，-1 表示 EOF */
int match_space(FILE *f)
{
    int c;
    c = fgetc(f);
    while (c != EOF && isspace(c))
        c = fgetc(f);
    if (c == EOF)
        return -1;
    ungetc(c, f);
    return 1;
}

/* 匹配一个字面字符，匹配返回 1，不匹配返回 0（并回退），EOF 返回 -1 */
int match_char(FILE *f, char c)
{
    int input;
    input = fgetc(f);
    if (input == EOF)
        return -1;
    if (input == (unsigned char)c)
        return 1;
    ungetc(input, f);
    return 0;
}

/* %c: 读取一个字符（不跳过空白），写入 char*，遇 EOF 返回 -1，否则 1 */
int scan_char(FILE *f, va_list ap)
{
    int c;
    char *ptr;
    c = fgetc(f);
    if (c == EOF)
        return -1;
    ptr = va_arg(ap, char *);
    *ptr = (char)c;
    return 1;
}

/* %d: 先跳过前导空白（match_space），然后读取连续数字（只处理正数或可带 +）。
 * 未读到数字返回 0，成功返回 1。
 */
int scan_int(FILE *f, va_list ap)
{
    int c;
    int r;
    int digits;
    int *ptr;

    if (match_space(f) == -1)   /* 跳过前导空白 */
        return -1;
    c = fgetc(f);
    if (c == EOF)
        return -1;
    r = 0;
    digits = 0;
    if (c == '+')              /* 允许可选的 + */
        c = fgetc(f);
    while (c != EOF && isdigit(c))
    {
        r = r * 10 + (c - '0');
        digits++;
        c = fgetc(f);
    }
    if (c != EOF)
        ungetc(c, f);
    if (digits == 0)
        return 0;
    ptr = va_arg(ap, int *);
    *ptr = r;
    return 1;
}

/* match_conv: 仅处理 c 和 d 两个转换符 */
int match_conv(FILE *f, const char **format, va_list ap)
{
    switch (**format)
    {
        case 'c':
            return scan_char(f, ap);
        case 'd':
            return scan_int(f, ap);
        default:
            return -1;
    }
}

/* ft_vfscanf: 主解析驱动（与官方版逻辑类似，但简化转换集） */
int ft_vfscanf(FILE *f, const char *format, va_list ap)
{
    int nconv = 0;
    int ret;
    int c;

    c = fgetc(f);             /* 初始 EOF 检测 */
    if (c == EOF)
        return EOF;
    ungetc(c, f);

    while (*format)
    {
        if (*format == '%')
        {
            format++;
            ret = match_conv(f, &format, ap);
            if (ret == -1)
                return (nconv == 0) ? EOF : nconv;
            if (ret == 0)
                return nconv;
            nconv++;
        }
        else if (isspace((unsigned char)*format))
        {
            if (match_space(f) == -1)
                return (nconv == 0) ? EOF : nconv;
        }
        else
        {
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
    return nconv;
}

/* ft_scanf: 外壳，调用 ft_vfscanf(stdin, format, ap) */
int ft_scanf(const char *format, ...)
{
    va_list ap;
    int ret;
    va_start(ap, format);
    ret = ft_vfscanf(stdin, format, ap);
    va_end(ap);
    return ret;
}
