#include "groupmodel.hpp"
#include "database.h"

#include <cstdlib>
#include <muduo/base/Logging.h>
#include <vector>

// 增：创建群组
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup (groupname, groupdesc) Values('%s', '%s')", 
    group.getName().c_str(), group.getDesc().c_str());


    MySQL mysql;
    if (mysql.connect()) {
        LOG_INFO << "mysql connect";
        if (mysql.update(sql)) {
            group.setId(mysql_insert_id(mysql.getConnection())); // user.setId 改id   mysql.getConnection()返回的MYSQL *
            LOG_INFO << "群创建成功";
            return true;
        }
    }
    LOG_INFO << "群创建失败";
    return false;
}

// 改:加入群组
void GroupModel::addGrouper(int userid, int groupid, std::string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser (groupid, userid, grouprole) Values('%d', '%d', '%s')", 
    groupid, userid, role.c_str());


    MySQL mysql;
    if (mysql.connect()) {
        LOG_INFO << "mysql connect";
        mysql.update(sql);
    }
}

// 查：查询用户所在的群，以及群内的成员
std::vector<Group> GroupModel::queryGroups(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.groupid,a.groupname,a.groupdesc from allgroup a inner join \
        groupuser b on a.groupid = b.groupid where b.userid = %d", id);

    MySQL mysql;
    std::vector<Group> groupVec;
    if (mysql.connect()) {
        LOG_INFO << "mysql connect";
        MYSQL_RES* res= mysql.query(sql); // 查询操作
        if (res != nullptr) 
        {
            MYSQL_ROW row; //mysql_fetch_row 读取一行
            while ((row= mysql_fetch_row(res)) != nullptr) 
            {
                Group group;
                group.setId(std::atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);   
            }
            mysql_free_result(res); // 释放资源
        }
    }
    for (Group &group : groupVec) {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a inner join \
        groupuser b on b.userid = a.id where b.groupid = %d", group.getId());

        MYSQL_RES* res= mysql.query(sql); // 查询操作
        if (res != nullptr) 
        {
            MYSQL_ROW row; //mysql_fetch_row 读取一行
            while ((row= mysql_fetch_row(res)) != nullptr) 
            {
                GroupUser user;
                user.setId(std::atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUser().push_back(user);
            }
            mysql_free_result(res); // 释放资源
        }
    }
    return groupVec;
}


// 查：根据指定groupID查询群组用户id列表，除自己id，主要用户群聊业务给群组其他成员群发信息。
std::vector<int> GroupModel::queryGroupUser(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", 
            groupid, userid);

    std::vector<int> idVec;
    MySQL mysql;
    if (mysql.connect()) {
        LOG_INFO << "mysql connect";
        MYSQL_RES* res= mysql.query(sql); // 查询操作
        if (res != nullptr) 
        {
            MYSQL_ROW row; //mysql_fetch_row 读取一行
            while ((row = mysql_fetch_row(res)) != nullptr) 
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res); // 释放资源
        }
    }

    return idVec;
}

