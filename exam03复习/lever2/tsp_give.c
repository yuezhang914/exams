#include <stdio.h>    /* FILE, fopen, fclose, fprintf, stdin, stdout, stderr, printf */
#include <errno.h>    /* errno */
#include <stdlib.h>   /* malloc, calloc, free, EXIT_* */
#include <math.h>     /* sqrtf */
#include <string.h>   /* (可用于可选的字符串操作) */
#include <stdbool.h>  /* bool, true, false（题目允许） */
#include <sys/types.h>/* ssize_t */

/* compute the distance between two points
 * 参数：a、b: 各为长度为 2 的 float 数组，分别表示 (x,y)
 * 返回：两点间欧氏距离（float）
 */
float distance(float a[2], float b[2])
{
    return sqrtf((b[0] - a[0]) * (b[0] - a[0]) + (b[1] - a[1]) * (b[1] - a[1]));
}

/* tsp:
 * 这是题目要求你实现的函数。在给出的框架中它是空壳（返回 best_distance）。
 * 函数签名：
 *   float tsp(float (*array)[2], ssize_t size)
 * 参数说明：
 *   - array: 大小为 size 的二维数组，array[i][0] 为第 i 个城市的 x，array[i][1] 为 y
 *   - size: 城市数量（题目会保证 <= 11）
 * 返回值：
 *   - 应返回覆盖所有城市的最短闭合路径长度（float）
 *
 * 你需要在这里实现你的算法（例如回溯 + 剪枝、或其它启发式/精确解法）
 */
float tsp(float (*array)[2], ssize_t size)
{
    float best_distance;
    // ...   /* 在这里实现核心算法并返回结果 */
    return (best_distance);
}

/* file_size:
 * 统计文件中的行数（使用 getline 逐行读取计数），并在结束时把文件指针重置到文件开头。
 * 参数：FILE *file （需处于可读模式）
 * 返回：读取到的行数（ssize_t >= 0），若出错返回 -1
 *
 * 说明：
 * - getline 会为 buffer 分配/扩展内存，因此使用后要 free(buffer)
 * - 函数使用 errno 判断 I/O 错误；若 errno 被置位或 fseek 失败则返回 -1
 */
ssize_t file_size(FILE *file)
{
    char *buffer = NULL;
    size_t n = 0;
    ssize_t ret;

    errno = 0; /* 清 errno */

    /* 使用 getline 读取所有行以统计行数；
       getline 会在需要时为 buffer 分配内存 */
    for (ret = 0; getline(&buffer, &n, file) != -1; ret++)
        ; /* 空循环体，ret 计数递增 */

    free(buffer); /* 释放 getline 分配的缓冲 */

    /* 如果在读取过程中发生错误或无法把文件指针回到开头，返回 -1 */
    if (errno || fseek(file, 0, SEEK_SET))
        return -1;
    return ret; /* 返回行数 */
}

/* retrieve_file:
 * 从 file 中按行读取坐标数据并存入 array
 * 每行期望格式为 "%f, %f\n"（x, y）
 * 返回 0 表示成功；若某行格式非法或发生 I/O 错误返回 -1（并设置 errno）
 */
int retrieve_file(float (*array)[2], FILE *file)
{
    int tmp;
    /* 逐行调用 fscanf 解析 float, float，直到遇到 EOF */
    for (size_t i = 0; (tmp = fscanf(file, "%f, %f\n", array[i] + 0, array[i] + 1)) != EOF; i++)
        if (tmp != 2) /* 若不是成功读取两个 float，则输入格式非法 */
        {
            errno = EINVAL;
            return -1;
        }
    if (ferror(file)) /* 若发生 I/O 错误 */
        return -1;
    return 0; /* 成功读取所有数据 */
}

/* main:
 * 主程序流程：
 *  - 如果命令行给了文件名则打开文件，否则使用 stdin
 *  - 使用 file_size 统计行数（即城市数）
 *  - 为 array 分配内存并调用 retrieve_file 读取坐标
 *  - 调用 tsp(array, size) 计算最短闭合路径长度并 printf 输出 "%.2f\n"
 *  - 清理资源并返回退出码
 */
int main(int ac, char **av)
{
    char *filename = "stdin"; /* 默认显示名 */
    FILE *file = stdin;       /* 默认输入流 */

    /* 如果提供了文件名，尝试打开 */
    if (ac > 1)
    {
        filename = av[1];
        file = fopen(filename, "r");
    }

    /* 打开失败则打印错误信息并退出 */
    if (!file)
    {
        fprintf(stderr, "Error opening %s: %m\n", filename);
        return 1;
    }

    /* 统计文件行数（城市数量） */
    ssize_t size = file_size(file);
    if (size == -1)
    {
        fprintf(stderr, "Error reading %s: %m\n", filename);
        fclose(file);
        return 1;
    }

    /* 分配数组：size 个元素，每个元素是 float[2] */
    float (*array)[2] = calloc(size, sizeof (float [2]));
    if (!array)
    {
        fprintf(stderr, "Error: %m\n");
        fclose(file);
        return 1;
    }

    /* 从文件中读取坐标到 array 中 */
    if (retrieve_file(array, file) == -1)
    {
        fprintf(stderr, "Error reading %s: %m\n", av[1]);
        fclose(file);
        free(array);
        return 1;
    }

    /* 如果打开的是文件则关闭；若是 stdin 则不要关闭 */
    if (ac > 1)
        fclose(file);

    /* 调用你要实现的 tsp 函数并输出格式化结果（两位小数） */
    printf("%.2f\n", tsp(array, size));

    /* 释放动态分配的数组并退出 */
    free(array);
    return (0);
}
