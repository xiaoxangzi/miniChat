#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"

#include <string>
#include <vector>


class GroupModel
{
public:
    // 增：创建群组
    bool createGroup(Group &group);
    // 删：
    
    
    // 改:添加群成员
    void addGrouper(int userid, int groupid, std::string role);
    // 查：查询群内信息 
    std::vector<Group> queryGroups(int id);
    // 查：根据指定groupID查询群组用户id列表，除自己id，主要用户群聊业务给群组其他成员群发信息。
    std::vector<int> queryGroupUser(int userid, int groupid);
};


#endif