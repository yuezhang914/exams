/* get_next_line_single.c
 * 将原本分散的实现合并为单文件版本，文件最前面包含所需头文件与 BUFFER_SIZE 定义。
 * 每行均含中文注释，便于学习与审阅。
 */

/* 标准库包含与用途说明：
 * <stdlib.h> — malloc, free, realloc, NULL 等。
 * <unistd.h> — read, ssize_t 等 POSIX I/O。
 * <string.h> — 用到 strlen、memcpy 时可用，但此实现使用自实现 gnl_strlen 等。
 * 这里保留 minimal 头文件，确保类型与函数声明可用。
 */
#include <stdlib.h> /* malloc, free, NULL, size_t 等 */
#include <string.h> /* （可选）如果编译器需要 strlen 等，这里包含了它 */
#include <unistd.h> /* read, ssize_t 等 POSIX I/O */

/* 如果外部没有定义 BUFFER_SIZE，则给出默认值 42 */
#ifndef BUFFER_SIZE
# define BUFFER_SIZE 42
#endif

/* ------------------------ 工具函数：字符串相关 ------------------------ */

/* 计算字符串长度，如果 s 为 NULL 则返回 0 */
size_t	gnl_strlen(const char *s)
{
	size_t i; /* 索引/计数器 */
	i = 0;    /* 从 0 开始计数 */
	if (!s)   /* 若传入 NULL，无法计算长度，返回 0 */
		return (0);
	while (s[i]) /* 遍历直到遇到 '\0' 为止 */
		i++;
	return (i); /* 返回计算得到的长度 */
}

/* 在字符串 s 中查找字符 c 的第一次出现位置，若找到返回指针，否则返回 NULL */
char	*gnl_strchr(const char *s, int c)
{
	int i;  /* 使用 int 保存索引（和原实现一致） */
	i = 0;  /* 从索引 0 开始扫描 */
	if (!s) /* 若 s 为 NULL，直接返回 NULL */
		return (NULL);
	while (s[i]) /* 遍历直到遇到 '\0' */
	{
		if (s[i] == (char)c)        /* 如果当前位置字符等于 c */
			return ((char *)&s[i]); /* 返回指向该位置的指针（去除 const） */
		i++;
	}
	/* 循环结束后 i 指向终止符位置，判断终止符是否也是目标字符（例如查找 '\0'） */
	if (s[i] == (char)c)
		return ((char *)&s[i]);
	return (NULL); /* 未找到，返回 NULL */
}

/* 复制字符串 s1，分配新内存并返回指针；s1 为 NULL 时返回 NULL */
char	*gnl_strdup(const char *s1)
{
	char *str;     /* 返回的新字符串指针 */
	size_t length; /* 要分配的长度（含 '\0'） */
	size_t i;      /* 复制时使用的索引 */
	i = 0;         /* 初始化索引 */
	if (!s1)       /* 如果输入为 NULL，直接返回 NULL（与原实现行为一致） */
		return (NULL);
	length = gnl_strlen(s1) + 1;         /* 需要的字节数：字符串长度 + 1 字节 NUL 终止符 */
	str = malloc(sizeof(char) * length); /* 分配内存 */
	if (!str)                            /* malloc 失败返回 NULL */
		return (NULL);
	while (s1[i]) /* 逐字符复制直到遇到 '\0' */
	{
		str[i] = s1[i];
		i++;
	}
	str[i] = '\0'; /* 末尾加 NUL 终止符 */
	return (str);  /* 返回复制的字符串 */
}

/* 将 s1 与 s2 拼接成一个新字符串（分配新内存返回），如果 s1 或 s2 为 NULL 有特殊处理 */
char	*gnl_strjoin(char const *s1, char const *s2)
{
	char *str;   /* 返回的新字符串 */
	size_t i;    /* 索引计数器 */
	size_t len1; /* s1 的长度 */
	size_t len2; /* s2 的长度 */
	/* 如果 s1 和 s2 都为 NULL，则没有可以拼接的内容，返回 NULL */
	if (!s1 && !s2)
		return (NULL);
	/* 如果 s1 为 NULL，则返回 s2 的 strdup 版本 */
	if (!s1)
		return (gnl_strdup(s2));
	/* 如果 s2 为 NULL，则返回 s1 的 strdup 版本 */
	if (!s2)
		return (gnl_strdup(s1));
	/* 计算两个字符串长度以便分配准确的内存 */
	len1 = gnl_strlen(s1);
	len2 = gnl_strlen(s2);
	str = malloc(len1 + len2 + 1); /* 分配 len1+len2 字节加上 1 个 NUL 字节 */
	if (!str)                      /* malloc 失败直接返回 NULL */
		return (NULL);
	i = 0; // size_t类型应该初始化为0 设为-1会变成极大值
	while (i < len1) //使用计算好的长度边界更安全
	{
		str[i] = s1[i];
		i++;
	}
	i = 0;
	while (i < len2) //使用计算好的长度边界更安全
	{
		str[len1 + i] = s2[i];
		i++;
	}
	str[len1 + len2] = '\0';
	return (str);
}

/* 从 s 的 start 位置截取长度为 len 的子串并返回（分配新内存）
 * 若 start 超出 s 长度，则返回空字符串（"" 的 strdup）
 */
char	*gnl_substr(char const *s, unsigned int start, size_t len)
{
	size_t sub_len; /* 实际要截取的长度（可能小于传入的 len） */
	size_t s_len;   /* 原字符串的长度 */
	size_t i;       /* 复制索引 */
	char *sub;      /* 子串返回指针 */
	if (!s)         /* 如果 s 为 NULL，无法截取，返回 NULL */
		return (NULL);
	s_len = gnl_strlen(s);       /* 计算 s 的长度 */
	if (start >= s_len)          /* 如果 start 已经越界 */
		return (gnl_strdup("")); /* 返回空字符串的复制（分配新内存） */
	sub_len = s_len - start;     /* 从 start 到结尾的长度 */
	if (sub_len > len)           /* 只取不超过 len 的部分 */
		sub_len = len;
	sub = malloc(sizeof(char) * (sub_len + 1)); /* 分配子串空间（含 NUL） */
	if (!sub)                                   /* 失败返回 NULL */
		return (NULL);
	i = 0;              /* 复制起始索引置 0 */
	while (i < sub_len) /* 复制 sub_len 字节 */
	{
		sub[i] = s[start + i];
		i++;
	}
	sub[i] = '\0'; /* 添加终止符 */
	return (sub);  /* 返回子串 */
}

/* ------------------------ 辅助与核心逻辑函数 ------------------------ */

/* 把 s1 与 s2 拼接，释放 s1 并返回新分配的拼接字符串
 * 用法：backup = join_free(backup, buf);
 */
static char	*join_free(char *s1, char *s2)
{
	char *joined;                  /* 存放拼接结果的指针 */
	joined = gnl_strjoin(s1, s2); /* 拼接 tmp 与 s2（内部会分配新内存） */
	free(s1);
	/* 释放旧的 s1 内存（注意 tmp 可能为 NULL，但 free(NULL) 是安全的） */
	return (joined); /* 返回新拼接字符串 */
}


/* 从 fd 读取内容并追加到 backup 中，直到遇到换行或 EOF
 * 返回更新后的 backup（分配/重分配后的指针），失败返回 NULL
 */
char	*read_save(int fd, char *backup)
{
	ssize_t count; /* read 返回的字节数（可能为 -1 / 0 / 正数） */
	char *buf;     /* 临时缓冲区，用于每次 read */
	/* 如果传入的 backup 为 NULL，就初始化为空字符串（方便后续拼接） */
	if (!backup)
		backup = gnl_strdup("");
	if (!backup) /* 如果 strdup 失败或仍为 NULL，返回 NULL 表示错误 */
		return (NULL);
	/* 为 read 的缓冲区分配内存，大小为 BUFFER_SIZE + 1（方便 NUL 终止） */
	buf = malloc(BUFFER_SIZE + 1);
	if (!buf) /* malloc 失败，清理并返回 NULL */
		return (free(backup), NULL);
	count = 1; /* 初始化 count 为 1，进入循环 */
	/* 循环：当 backup 中没有换行并且上次 read 返回大于 0 时继续读取 */
	while (backup && !gnl_strchr(backup, '\n') && count > 0)
	{
		count = read(fd, buf, BUFFER_SIZE);
		/* 从 fd 读取最多 BUFFER_SIZE 字节 */
		if (count < 0)                              /* read 出错（返回 -1） */
			return (free(buf), free(backup), NULL); /* 释放资源并返回 NULL */
		if (count == 0)                             /* 到达 EOF（读取到文件结束） */
			break ;                                  /* 退出循环 */
		buf[count] = '\0';                          /* 将读取的字节以 NUL 终止，方便字符串操作 */
		backup = join_free(backup, buf);
		/* 将 buf 拼接到 backup（并释放旧 backup） */
		if (!backup)
			/* 如果拼接后返回 NULL（malloc 失败） */
			return (free(buf), NULL); /* 释放 buf 并返回 NULL */
	}
	return (free(buf), backup); /* 释放临时 buf，返回最终的 backup（可能包含换行） */
}

/* 从 backup 中提取第一行（含 '\\n'，若存在），返回新分配的字符串（或 NULL） */
char	*get_the_line(char *backup)
{
	char *line; /* 将返回的行字符串 */
	size_t i;   /* 用于定位换行或结尾的索引 */
	i = 0;
	if (!backup || !*backup) /* 如果 backup 为空或内容为空字符串，返回 NULL（没有可返回的行） */
		return (NULL);
	while (backup[i] && backup[i] != '\n') /* 找到第一处换行符或结尾 */
		i++;
	if (backup[i] == '\n')           /* 如果在位置 i 找到换行符 */
		i++;                         /* 包含换行符在内，行长是 i（包含 '\\n'） */
	line = gnl_substr(backup, 0, i); /* 截取从 0 开始长度为 i 的子串（分配新字符串） */
	return (line);                   /* 返回该行（调用者负责 free） */
}

/* 更新 backup：移除已返回的那一行，保留剩余部分并返回新的分配字符串 */
char	*refresh_backup(char *backup)
{
	size_t i;    /* 索引，用于查找换行位置 */
	char *fresh; /* 存放剩余部分的新分配字符串 */
	if (!backup) /* 若 backup 为 NULL，直接返回 NULL */
		return (NULL);
	i = 0;
	while (backup[i] && backup[i] != '\n') /* 找到行的结束（或结尾） */
		i++;
	if (!backup[i]) /* 如果没有发现换行（即没有剩余部分） */
	{
		free(backup);  /* 释放原 backup 内存 */
		return (NULL); /* 返回 NULL，表示没有剩余内容 */
	}
	/* 如果存在换行，则复制换行后的剩余部分 */
	fresh = gnl_strdup(backup + i + 1); /* 从换行后一个字符开始复制到末尾 */
	free(backup);                       /* 释放原 backup 内存 */
	return (fresh);                     /* 返回剩余部分（调用者赋值回 backup） */
}

/* 主函数：返回下一行字符串（含换行符），无更多行时返回 NULL */
char	*get_next_line(int fd)
{
	static char *backup; /* 静态变量用于保存剩余未返回的内容（跨次调用保持） */
	char *line;          /* 将要返回的行字符串 */
	/* 检查参数：fd 非法或 BUFFER_SIZE 非法时返回 NULL */
	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	/* 读取并保存数据到 backup，使 backup 至少包含一个完整行或 EOF */
	backup = read_save(fd, backup);
	if (!backup) /* 如果读取或拼接失败，返回 NULL */
		return (NULL);
	line = get_the_line(backup);     /* 从 backup 中提取第一行并新分配返回字符串 */
	backup = refresh_backup(backup); /* 更新 backup 为剩余部分（可能为 NULL） */
	if (!line)                       /* 如果没有可返回的行（例如 backup 为空） */
	{
		free(backup);  /* 确保释放 backup（若为 NULL free 安全） */
		backup = NULL; /* 将静态指针置 NULL 保持一致性 */
		return (NULL); /* 返回 NULL 表示没有更多行 */
	}
	return (line); /* 返回新分配的 line，调用者负责 free */
}
