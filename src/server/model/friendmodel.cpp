#include "/home/caht/chat_project/include/server/model/frindmodel.hpp"
#include "/home/caht/chat_project/include/server/database/database.h"

#include <cstdlib>
#include <muduo/base/Logging.h>



// 添：添加好友
void FriendModel::insert(int userid,int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend (userid, friendid) Values(%d, %d)", userid, friendid);

    
    MySQL mysql;
    if (mysql.connect()) {
           
        mysql.update(sql);
    }
}


// 查：返回用户好友列表
std::vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "SELECT a.id, a.name, a.state \
    FROM user a INNER JOIN friend b ON b.friendid = a.id WHERE b.userid = %d;", userid);

    std::vector<User> vec;
    MySQL mysql;
    if (mysql.connect()) {
        // MYSQL_RES* 时mysql库提供的   数据返回指针
        MYSQL_RES* res= mysql.query(sql); // 查询操作
        if (res != nullptr) 
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) // mysql_fetch_row会一行一行的返回
            {
                User user;
                user.setId(atoi(row[0])); 
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
