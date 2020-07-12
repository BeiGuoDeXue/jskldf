/**
 * @file zhssqlite.h
 * @brief 
 * @author xiaozhong (xiaozhong@zhskg.cn)
 * @version 1.0
 * @date 2020-06-11
 * 
 * @copyright Copyright (c) 2020-2024  智慧式集团
 * 
 * @par 修改日志:
 * 2020-06-11 1.0     xiaozhong     
 */
#ifndef ZHSSQLITE_H
#define ZHSSQLITE_H

#include "sqlite3.h"
/*
 * 数据库的操作指令
 *
 * refr: 
 * sqlite_data_init(), 为指定数据库创建数据表 DATA
 */
int load_subset_data(void);
int sqlite_data_init(const char *name);
int sqlite_data_insert(sqlite3 *db_obj, const char *name, char *data, char *buf, int times);
int sqlite_data_update(const char *name, char *data, char *buf, int times);
int sqlite_data_select(sqlite3 *db_obj, const char *name, const char *cmd, char *id, char *data, char *buf, char *time);
int sqlite_data_pointlist(const char *name, char *id, char *data, char *buf, char *time);
int sqlite_data_delete(sqlite3 *db_obj, const char *name, const char *cmd);
int sqlite_point_delete(const char *name, char *data, char *buf);


/*
 * 报警数据库
 *
 * 设备通过 推车上报的以下信息类型，将写入此数据库
 * alarming
 * faulting
 * warn_alarming
 * reset_alarming
 * reset_faulting
 */
#define ALARM_FILE "/tmp/alarm.db"
#define CONFIG_FILE "/tmp/config.db"
#define UPDATA_FILE "/tmp/updata.db"

/*
 * 数据库 DATA 表的结构
 *
 * select 语句将读出一条这样的记录
 */
typedef struct _SQLITE_SELECTINFO
{
    char id[20];
    char data[512];
    char buf[512];
    char time[24];
} SQLITE_SELECTINFO;



/*
 * 某个数据库的某条记录
 *
 * 主要用于在发送 MQTT 消息成功后的回调函数中，删除相应的数据库记录
 */
struct db_rec
{
    /* 此记录对应的数据库名称 */
    const char *db_name;

    /* 此记录在数据库表中的索引 */
    int id;
};

static struct db_rec alarm_db_rec =
{ ALARM_FILE,0 };

static struct db_rec updata_db_rec =
{ UPDATA_FILE,0 };

#endif //ZHSSQLITE_H
