#ifndef UERMODEL_H
#define UERMODEL_H

#include "user.hpp"

class UserModel
{
public:
    // 增加用户
    bool insert(User &user);
    // 通过id查找用户
    User query(int id);
    
    // 改:通过传入的user改写mysql中的数据
    bool updata(User user);
    // 改：更新user状态
    bool updataState(User user);
    // 重置用户的状态信息
    bool resetState();
    
};


#endif