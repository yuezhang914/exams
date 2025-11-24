// filter:对应 subject_filter（允许 printf，空输入不报错）
// Allowed functions: read, strlen, malloc, calloc, realloc, free, printf, perror
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


/* filter_subject.c — 对应 subject_filter
 * 说明：这个版本允许使用 printf 输出，且当 stdin 为空时程序返回 0（不视为空输入为错误）。
 */

/* 下面是包含的头文件及其用途：
 * <stdlib.h>  — 提供 malloc/free/realloc/exit 等内存与退出接口。
 * <unistd.h>  — 提供 read/write/STDIN_FILENO 等 POSIX I/O 接口。
 * <string.h>  — 提供 strlen/memcpy 等字符串/内存操作函数。
 * <stdio.h>   — 提供 printf/perror 等标准 I/O 函数（本版本允许使用 printf）。
 * <errno.h>   — 提供 errno 常量，用于判断 read 被中断等错误情况。
 */

#include <stdlib.h>   /* malloc/free/realloc/exit 等函数声明 */
#include <unistd.h>   /* read/write/STDIN_FILENO 等 POSIX I/O 声明 */
#include <string.h>   /* strlen, memcpy 等字符串/内存操作声明 */
#include <stdio.h>    /* printf, perror 等标准输入输出声明 */
#include <errno.h>    /* errno 及错误码定义 */

 /* 判断字符是否为“单词字符”（我定义为字母或数字） */
int is_word_char(char c) /* 函数：判断 c 是否是字母或数字，返回 1 表示是 */
{
    int res;              /* 存放判断结果的临时变量 */
    res = 0;              /* 初始化为 0（不是单词字符） */
    if (c >= '0' && c <= '9') /* 如果 c 在 '0'..'9' 范围内 */
        res = 1;          /* 标记为单词字符 */
    if (c >= 'a' && c <= 'z') /* 如果 c 在 'a'..'z' 范围内 */
        res = 1;          /* 标记为单词字符 */
    if (c >= 'A' && c <= 'Z') /* 如果 c 在 'A'..'Z' 范围内 */
        res = 1;          /* 标记为单词字符 */
    return res;           /* 返回判断结果 */
}

/* 将 read 得到的 chunk（长度 n）追加到动态缓冲区 *buf，更新 *total */
int safe_append(char **buf, size_t *total, const char *chunk, ssize_t n)
{
    char *tmp;            /* 用于暂时保存 realloc 返回的新指针 */
    size_t old;           /* 旧的总长度 */
    size_t new_total;     /* 扩容后的新长度 */

    old = *total;         /* 读取当前已存总长度 */
    new_total = old + (size_t)n; /* 计算追加后的新总长度 */

    /* realloc 成 new_total + 1 字节，最后留 1 字节放 '\0' */
    tmp = realloc(*buf, new_total + 1); /* 重新分配内存（可能增长） */
    if (tmp == NULL)      /* 如果 realloc 失败 */
        return -1;        /* 返回 -1 表示错误 */

    *buf = tmp;           /* 更新外部 buf 指针为 realloc 返回值 */

    /* 把新读取的数据复制到 buf 的尾部 */
    memcpy(*buf + old, chunk, (size_t)n); /* 将 chunk 数据拷贝到新空间 */

    /* 在字符串末尾放置 '\0' 以方便 printf 等函数使用 */
    (*buf)[new_total] = '\0'; /* 以 NUL 终止字符串 */

    *total = new_total;    /* 更新外部记录的总长度 */
    return 0;              /* 返回 0 表示成功 */
}

/* 在 buf 中查找 pattern 并就地替换为 '*'（仅在单词边界处替换） */
void replace_inplace(char *buf, size_t len, const char *pat, size_t patlen)
{
    size_t i;             /* 主扫描索引 */

    i = 0;                /* 从头开始扫描 */
    /* 当还有足够的字符可以与 pattern 比对时继续 */
    while (i + patlen <= len)
    {
        /* 如果当前位置左侧是缓冲区起点或左侧不是单词字符，可能是单词起点 */
        if (i == 0 || !is_word_char(buf[i - 1]))
        {
            size_t j;     /* 内部匹配索引 */
            int match;    /* 标记是否匹配 */
            match = 1;    /* 先假设匹配成功 */

            j = 0;        /* 从 pattern 的第 0 个字符开始比较 */
            while (j < patlen) /* 逐字符比较 pattern 与 buf 子串 */
            {
                if (buf[i + j] != pat[j]) /* 如果任一字符不相等 */
                {
                    match = 0; /* 标记为不匹配 */
                    break;     /* 提前退出比较循环 */
                }
                j = j + 1;   /* 比较下一个字符 */
            }

            /* 如果 pattern 全部匹配，再检查右侧是否为单词边界 */
            if (match)
            {
                if (i + patlen == len || !is_word_char(buf[i + patlen]))
                {
                    /* 左右都为单词边界，执行替换为 '*' */
                    j = 0;
                    while (j < patlen)
                    {
                        buf[i + j] = '*'; /* 就地替换每个匹配字符 */
                        j = j + 1;
                    }
                    /* 跳过已替换的部分继续扫描，避免重复从已替换处开始 */
                    i = i + patlen;
                    continue; /* 继续主循环 */
                }
            }
        }
        /* 若未替换或不符合条件，向后移动一位继续扫描 */
        i = i + 1;
    }
}

/* 主程序：读取 stdin，累积到内存，替换后用 printf 输出（本 subject 允许） */
int main(int argc, char **argv)
{
    char readbuf[4096];   /* 临时读取缓冲区，用于每次 read */
    ssize_t n;            /* read 返回的字节数 */
    char *input;          /* 动态缓冲区，保存全部 stdin 内容 */
    size_t total;         /* input 当前已保存的总字节数 */
    size_t patlen;        /* pattern 的长度 */

    /* 参数检查：必须提供要替换的单词 */
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <word>\n", argv[0]); /* 打印用法到 stderr */
        return 1; /* 参数错误返回 1 */
    }

    input = NULL;         /* 初始时没有分配缓冲区 */
    total = 0;            /* 已读字节数为 0 */

    /* 循环读取 stdin，直到 EOF 或错误 */
    while (1)
    {
        n = read(0, readbuf, sizeof(readbuf)); /* 从 fd 0（stdin）读取数据 */
        if (n == -1)      /* read 出错 */
        {
            if (errno == EINTR) /* 如果被信号中断 */
                continue; /* 重试读取 */
            perror("read"); /* 其它错误打印 perror 并退出 */
            free(input);    /* 释放已分配的内存 */
            return 1;
        }
        if (n == 0)       /* EOF（正常结束） */
            break;        /* 退出读取循环 */

        /* 将这次读取的数据追加到动态缓冲区 input 中 */
        if (safe_append(&input, &total, readbuf, n) == -1)
        {
            perror("realloc"); /* 内存分配失败时打印 perror */
            free(input);
            return 1;
        }
    }

    /* 如果没有任何输入（total == 0），本 subject 允许直接成功返回 0（不视为空输入为错误） */
    if (total == 0)
    {
        free(input);      /* 释放可能为 NULL 的指针（safe） */
        return 0;         /* 返回 0 表示成功 */
    }

    patlen = strlen(argv[1]); /* 计算 pattern 的长度（用于替换函数） */
    if (patlen > 0)       /* 若 pattern 非空才执行替换 */
        replace_inplace(input, total, argv[1], patlen); /* 就地替换 */

    printf("%s", input);  /* 使用 printf 输出结果（本版本允许使用 printf） */

    free(input);         /* 释放动态缓冲区 */
    return 0;            /* 正常退出 */
}

