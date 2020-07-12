#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif
 #define DEBUG 1
#include <sys/types.h>
#ifdef DEBUG
#define DBG(fmt, args...)                                                        \
    do                                                                           \
    {                                                                            \
        fprintf(stdout, "[DBG]: [%s:%5d] --- " fmt, __FILE__, __LINE__, ##args); \
    } while (0)
#define ERR(fmt, args...)                                                        \
    do                                                                           \
    {                                                                            \
        fprintf(stderr, "[ERR]: [%s:%5d] *** " fmt, __FILE__, __LINE__, ##args); \
    } while (0)
#else
#define ERR(fmt, args...)
#define DBG(fmt, args...)
#endif

    struct pic_infor
    {
        char name[37];
        char capt[20];
        char leave[1];
    };

    int find_pid_by_name(char *ProcName, int *foundpid);
    int execcmd(const char *cmd, char *res);
    int uuid_generate(char *uuid);

    int is_file_exist(const char *filepath);
    int is_dir_exist(const char *dir);
    int get_info(char *name, char *info);
    const char *get_filename(const char *p);
#ifdef __cplusplus
}
#endif

#endif // __COMMON_H__