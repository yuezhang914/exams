// （对应 not_official_subject，stdin 为空时返回非 0，输出只用 write）
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


/* filter_not_official.c — 对应 not_official_subject
 * 说明：这个版本满足更严格的 subject 要求：
 * - 如果 stdin 为空（没有任何输入字节），程序返回非 0（返回 1）
 * - 输出使用 write（不使用 printf），以满足 allowed functions 列表
 */

/* 下面说明所用头文件与用途：
 * <stdlib.h>  — malloc/free/realloc/exit 等基础函数。
 * <unistd.h>  — read/write/STDIN_FILENO 等 POSIX I/O。
 * <string.h>  — strlen, memcpy 等字符串/内存操作。
 * <errno.h>   — errno，用于判断 read 被 EINTR 等。
 */

#include <stdlib.h>   /* malloc/free/realloc/exit 等声明 */
#include <unistd.h>   /* read/write/STDIN_FILENO 等声明 */
#include <string.h>   /* strlen, memcpy 等声明 */
#include <errno.h>    /* errno 错误码声明 */

/* 判断字符是否为“单词字符”（字母或数字） */
int is_word_char(char c) /* 与版本 A 相同的实现 */
{
    int res;
    res = 0;
    if (c >= '0' && c <= '9')
        res = 1;
    if (c >= 'a' && c <= 'z')
        res = 1;
    if (c >= 'A' && c <= 'Z')
        res = 1;
    return res;
}

/* 循环写入所有字节，直到全部写完或出错
 * 返回 0 表示成功，-1 表示写入失败
 */
int write_all(int fd, const char *buf, size_t len)
{
    ssize_t w;           /* write 返回的值（可能部分写入） */
    size_t offset;       /* 已写入的偏移量 */

    offset = 0;          /* 初始化偏移量为 0 */
    while (offset < len) /* 循环直到写完所有字节 */
    {
        w = write(fd, buf + offset, len - offset); /* 写入剩余部分 */
        if (w == -1)    /* write 出错 */
        {
            if (errno == EINTR) /* 如果被信号中断，重试 */
                continue;
            return -1;   /* 否则返回 -1 表示错误 */
        }
        if (w == 0)     /* write 返回 0（通常不应发生）视为错误 */
        {
            return -1;
        }
        offset = offset + (size_t)w; /* 更新已写入字节数 */
    }
    return 0;            /* 全部写入成功 */
}

/* safe_append：将 chunk（长度 n）追加到 *buf 中，更新 *total */
int safe_append(char **buf, size_t *total, const char *chunk, ssize_t n)
{
    char *tmp;
    size_t old;
    size_t new_total;

    old = *total;         /* 记录旧长度 */
    new_total = old + (size_t)n; /* 计算新长度 */

    tmp = realloc(*buf, new_total + 1); /* 重新分配（多留一字节放 '\0'） */
    if (tmp == NULL)      /* realloc 失败 */
        return -1;

    *buf = tmp;           /* 更新外部 buf 指针 */
    memcpy(*buf + old, chunk, (size_t)n); /* 复制新数据到尾部 */
    (*buf)[new_total] = '\0'; /* 末尾放 NUL 字符，便于某些操作 */
    *total = new_total;   /* 更新已保存总长度 */
    return 0;             /* 成功返回 0 */
}

/* 就地替换函数：在 buf 中查找 pattern 并替换为 '*'（仅单词边界） */
void replace_inplace(char *buf, size_t len, const char *pat, size_t patlen)
{
    size_t i;

    i = 0;
    while (i + patlen <= len)
    {
        if (i == 0 || !is_word_char(buf[i - 1]))
        {
            size_t j;
            int match;
            match = 1;
            j = 0;
            while (j < patlen)
            {
                if (buf[i + j] != pat[j])
                {
                    match = 0;
                    break;
                }
                j = j + 1;
            }
            if (match)
            {
                if (i + patlen == len || !is_word_char(buf[i + patlen]))
                {
                    j = 0;
                    while (j < patlen)
                    {
                        buf[i + j] = '*';
                        j = j + 1;
                    }
                    i = i + patlen;
                    continue;
                }
            }
        }
        i = i + 1;
    }
}

/* 主程序：参数校验 -> 读取 stdin -> 若无输入返回错误 -> 替换 -> 用 write 输出 */
int main(int argc, char **argv)
{
    char readbuf[4096];   /* 每次从 stdin 读取的数据缓冲 */
    ssize_t n;            /* read 返回的字节数 */
    char *input;          /* 动态缓冲区，存放整个 stdin 内容 */
    size_t total;         /* 当前已保存的总长度 */
    size_t patlen;        /* pattern 长度 */
    int ret;              /* 用于保存 write_all 的返回结果 */

    if (argc < 2)         /* 参数检查：缺少要替换的单词 */
    {
        write(2, "usage: filter <word>\n", 21); /* 向 stderr 写简短用法提示 */
        return 1;         /* 返回错误码 1 */
    }

    input = NULL;         /* 初始化动态缓冲区为空 */
    total = 0;            /* 已读长度为 0 */

    /* 循环读取 stdin，把数据累加到 input 中 */
    while (1)
    {
        n = read(0, readbuf, sizeof(readbuf)); /* 从 stdin 读取 */
        if (n == -1)     /* read 出错 */
        {
            if (errno == EINTR) /* 被信号中断则重试 */
                continue;
            /* 其它 read 错误：写错误提示到 stderr 并退出 */
            write(2, "read error\n", 11);
            free(input);  /* 释放已分配的内存 */
            return 1;
        }
        if (n == 0)      /* EOF（正常结束） */
            break;       /* 退出读取循环 */

        /* 将读取到的数据追加到 input 缓冲区 */
        if (safe_append(&input, &total, readbuf, n) == -1)
        {
            write(2, "memory error\n", 13); /* 内存分配失败写错误提示 */
            free(input);
            return 1;
        }
    }

    /* 如果 stdin 没有任何输入字节（total == 0），按 not_official_subject 要求返回错误 */
    if (total == 0)
    {
        free(input);      /* 释放内存（若为 NULL safe） */
        return 1;         /* 返回非 0 表示错误 */
    }

    patlen = strlen(argv[1]); /* 计算 pattern 长度 */
    if (patlen > 0)        /* 若 pattern 非空则执行替换 */
        replace_inplace(input, total, argv[1], patlen);

    /* 用 write_all 将结果写到 stdout（fd=1），避免使用 printf */
    ret = write_all(1, input, total);
    free(input);           /* 释放动态缓冲区 */

    if (ret == -1)         /* 若写入失败则打印错误提示并返回 1 */
    {
        write(2, "write error\n", 12);
        return 1;
    }

    return 0;              /* 成功退出 */
}
