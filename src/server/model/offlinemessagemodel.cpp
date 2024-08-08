#include "/home/caht/chat_project/include/server/model/offlinemessagemodel.hpp"
#include "/home/caht/chat_project/include/server/database/database.h"

#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
#include <string>
#include <vector>


bool OfflineMsgModel::insert(int userid, std::string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage Values(%d, '%s')", userid, msg.c_str());

    MySQL mysql;
    if (mysql.connect()) {
        LOG_INFO << "mysql connect";
        mysql.update(sql);
        return true;
    }
    LOG_INFO << "add offlinemessage error => sql:" << sql;
    return false;
}

bool OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userid);


    MySQL mysql;
    if (mysql.connect()) {
        LOG_INFO << "mysql connect";
        mysql.update(sql);
        return true;
    }
    LOG_INFO << "delete offlinemessage error => sql:" << sql;
    return false;
}

std::vector<std::string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from offlinemessage where userid = %d", userid);

    std::vector<std::string> vec;
    MySQL mysql;
    if (mysql.connect()) {
        // MYSQL_RES* 时mysql库提供的   数据返回指针
        MYSQL_RES* res= mysql.query(sql); // 查询操作
        if (res != nullptr) 
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) // mysql_fetch_row会一行一行的返回
            {
                vec.push_back(row[1]); //mysql_fetch_row 读取一行    
            }
            mysql_free_result(res);
        }
    }
    return vec;

}
