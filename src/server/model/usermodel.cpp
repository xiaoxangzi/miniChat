#include "/home/caht/chat_project/include/server/model/usermodel.hpp"
#include "/home/caht/chat_project/include/server/database/database.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mysql/mysql.h>
#include <muduo/base/Logging.h>

// user表：往里面添加用户  业务层和数据库层中间的orm层
bool UserModel::insert (User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into user (name, password, state) Values('%s', '%s', '%s')", 
    user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());


    MySQL mysql;
    if (mysql.connect()) {
        LOG_INFO << "mysql connect";
        if (mysql.update(sql)) {
            LOG_INFO << "add user success => sql:" << sql ;
            user.setId(mysql_insert_id(mysql.getConnection())); // user.setId 改id   mysql.getConnection()返回的MYSQL *
            return true;
        }
    }
    LOG_INFO << "add user error => sql:" << sql;
    return false;
}
// user表：往里面删除用户

// user表：往里面改写用户
bool UserModel::updata(User user)
{
    char sql[1024] = {0};
    // update user set state = "%s", 
    sprintf(sql, "update user set name = '%s', password = '%s', state = '%s' where id = '%d'", 
    user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str(), user.getId());


    MySQL mysql;
    if (mysql.connect()) {
            LOG_INFO << "mysql connect";
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

bool UserModel::updataState(User user)
{
    char sql[1024] = {0};
    // update user set state = "%s", 
    sprintf(sql, "update user set state = '%s' where id = '%d'", 
    user.getState().c_str(), user.getId());


    MySQL mysql;
    if (mysql.connect()) {
            LOG_INFO << "mysql connect";
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}



// user表：往里面查找用户
User UserModel::query(int id)
{   
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);


    MySQL mysql;
    if (mysql.connect()) {
        LOG_INFO << "mysql connect";
        MYSQL_RES* res= mysql.query(sql); // 查询操作
        if (res != nullptr) 
        {
            MYSQL_ROW row= mysql_fetch_row(res); //mysql_fetch_row 读取一行
            if (row != nullptr) 
            {
                User user;
                user.setId(std::atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);

                mysql_free_result(res); // 释放资源
                return user;
            }
        }
    }

    return User();
}

bool UserModel::resetState()
{
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
        return true;
    }
    return false;
}
