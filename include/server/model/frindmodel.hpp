#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "user.hpp"
#include <vector>

class FriendModel
{
public:
    // 添：添加好友
    void insert(int userid,int friendid);
    // 查：返回用户好友列表
    std::vector<User> query(int userid);
};

#endif