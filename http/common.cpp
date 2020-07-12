#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <strings.h>
#include "common.h"

int find_pid_by_name(char *ProcName, int *foundpid)
{
    DIR *dir;
    struct dirent *d;
    int pid, i;
    char *s;
    int pnlen;

    i = 0;
    foundpid[0] = 0;
    pnlen = strlen(ProcName);

    dir = opendir("/proc");
    if (!dir)
    {
        ERR("cannot open /proc");
        return -1;
    }
    while ((d = readdir(dir)) != NULL)
    {
        char exe[PATH_MAX + 1];
        char path[PATH_MAX + 1];
        int len;
        int namelen;

        if ((pid = atoi(d->d_name)) == 0)
            continue;

        snprintf(exe, sizeof(exe), "/proc/%s/exe", d->d_name);
        if ((len = readlink(exe, path, PATH_MAX)) < 0)
            continue;
        path[len] = '\0';

        /* Find ProcName */
        s = strrchr(path, '/');
        if (s == NULL)
            continue;
        s++;
        namelen = strlen(s);
        if (namelen < pnlen)
            continue;
        if (!strncmp(ProcName, s, pnlen))
        {
            if (s[pnlen] == ' ' || s[pnlen] == '\0')
            {
                foundpid[i] = pid;
                i++;
            }
        }
    }
    foundpid[i] = 0;
    closedir(dir);
    return 0;
}
int execcmd(const char *cmd, char *res)
{
    int len = 0;
    FILE *pp = popen(cmd, "r"); //建立管道
    if (!pp)
    {
        return -1;
    }
    char tmp[200]; //设置一个合适的长度，以存储每一行输出
    while (fgets(tmp, sizeof(tmp), pp) != NULL)
    {
        if (tmp[strlen(tmp) - 1] == '\n')
        {
            tmp[strlen(tmp) - 1] = '\0'; //去除换行符
        }
    }
    len = sprintf(res, "%s", tmp);
    pclose(pp); //关闭管道
    return len;
}
int uuid_generate(char *uuid)
{
    return execcmd("cat /proc/sys/kernel/random/uuid", uuid);
}
int is_dir_exist(const char *dir)
{
    if (dir == NULL)
    {
        return -1;
    }
    if (opendir(dir) == NULL)
    {
        return -1;
    }
    return 0;
}
int is_file_exist(const char *filepath)
{
    if (filepath == NULL)
    {
        return -1;
    }
    if (access(filepath, F_OK) == 0)
    {
        return 0;
    }
    return -1;
}
const char *get_filename(const char *p)
{
    // int x = strlen(p);
    char ch = '/';
    const char *q = strrchr(p,ch) + 1;
 
    return q;
}
int get_info(char *name, char *info)
{
    if (is_file_exist(name) != 0)
    {
        return -1;
    }
    FILE *file;
    if ((file = fopen(name, "rb")) == NULL)
    {
        fclose(file);
        return -1;
    }
    long size = 0;
    struct stat fstat;
    if(stat(name, &fstat) < 0){
		return size;
	}else{
		size = fstat.st_size;
	}
    fread(info, size, 1, file);
    fclose(file);
    return 0;
}
