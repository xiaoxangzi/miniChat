#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>

//通过离线信息表的操作方法
class OfflineMsgModel
{
public:
    // 增：储存离线消息
    bool insert(int userid, std::string msg);
    // 删：删除离线消息
    bool remove(int userid);
    // 查：查询指定用户的离线消息
    std::vector<std::string> query(int userid);
};


#endif